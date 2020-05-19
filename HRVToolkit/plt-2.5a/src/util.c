/* file: util.c		Paul Albrecht	  September 1984
			Last revised:	14 November 2002
Memory allocation, error handling, and other utilities for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  27 March 2001; added #ifdef TCSBRK and zeroed oldGrayLevel in pinit
  11 April 2001: rewrote err() to use ANSI C facilities for variadic
                 functions, and to fix a nasty buffer overrun; general cleanup
  12 April 2001: fixed pointer types in azmem
  19 May 2001: added workaround for sscanf bug in Cygwin
  14 November 2002: removed <termio.h> and TCSBRK
_______________________________________________________________________________
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA.

You may contact the maintainer by e-mail (george@mit.edu) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include "plt.h"

#define MAXERR 6     /* number of non-fatal errors allowed before giving up */

static Boolean firstQuit;
static short nErrors, opened;
static char *errors[MAXERR];

void UtilInit(Mode mode)
{
  short n;

  firstQuit = YES;
  for (n = 0; n < nErrors; n++) {
    FREE(errors[n]);
  }
  nErrors = 0;
}

/* zmem wraps calloc safely so that the caller doesn't need to check for
   errors. */
void *zmem(size_t n, size_t size)
{
  void *ptr;

  if (n == 0 || size == 0 || (ptr = calloc(n,size)) == NULL)
    err(YES, "Out of memory in zmem.  Can't allocate %u bytes", n*size);
  return (ptr);
}

/* azmem wraps realloc safely. */
void *azmem(void *ptr, size_t *current_elements, size_t new_elements,
	    size_t element_size)
{
  size_t old_size, new_size;
  char *c, *cend;

  old_size = (*current_elements) * element_size;
  new_size = old_size + new_elements * element_size;
  *current_elements += new_elements;
  if (ptr == NULL || old_size == 0)
    ptr = new_size ? calloc(new_elements, element_size) : NULL;
  else
    ptr = realloc(ptr, new_size);
  if (ptr == NULL)
    err(YES, "Out of memory in azmem.  Can't allocate %u bytes",
	new_elements * element_size);
  for (c = ptr + old_size, cend = ptr + new_size; c < cend; c++)
    *c = 0;
  return (ptr);
}

/* StringSave is functionally identical to strdup (SVID 3/BSD 4.3). */
char *StringSave(char *str)
{
  char	*ptr;

  if (str == NULL) {
    err(NO, "BUG: called StringSave(0)");
    return (NULL);
  }
  ptr = zmem(1, strlen(str) + 1);
  strcpy(ptr, str);
  return (ptr);
}

/* All of plt's error-handling is done by invoking err().  The first argument
   of err() is NO if the error is non-fatal (i.e., if plt can continue
   running anyway) or YES if plt should report the error and exit immediately.
   The remaining arguments are printf-style (a format string and a variable
   number of parameters).

   Note that two slightly different implementations are provided, depending on
   the preprocessor constant NO_VSNPRINTF.  One implementation uses the (very
   much preferred) vsnprintf; the other uses vsprintf, since some C libraries
   do not provide vsnprintf.  Beware: vsprintf is a serious security problem,
   since some error messages quote the user's input, which may be arbitrarily
   long; when these are passed to vsprintf, buffer overruns and possible stack
   corruption can follow.  vsnprintf avoids this problem because it will not
   write beyond the end of the buffer,  Upgrade your libraries if necessary;
   avoid using vsprintf if possible. */
void err(int fatal, char *fmt, ...)
{
  short n;
  char str[200];
  va_list args;

  sprintf(str, "%s: ", programName);
  n = strlen(str);
  va_start(args, fmt);
#ifndef NO_VSNPRINTF
  vsnprintf(&str[n], sizeof(str)-n-2, fmt, args);
#else
  vsprintf(&str[n], fmt, args);
#endif
  va_end(args);
  strcat(str, "\n");
  if (p && p->mode & SEP_GRAPH) fprintf(stderr, "%s\n", str);
  else errors[nErrors++] = StringSave(str);
  if (nErrors == MAXERR-1) {
    sprintf(str, "%s: Aborting...", programName);
    if (p && p->mode & SEP_GRAPH)
      fprintf(stderr, "%s\n", str);
    else
      errors[nErrors++] = StringSave(str);
    fatal = YES;
  }
  if (fatal) pquit(0);
}

/* For debugging.  See the definition of ASSERT in plot.h. */
void assert(char *text, char *file, int line)
{
  err(YES, "Assertion false at %s(%d): %s\n",  file, line, text);
}

/* Exit as gracefully as possible. */
void pquit(int signo)
{
  int n;

  signal(SIGINT, SIG_IGN);
  if (firstQuit) {
    firstQuit = NO;
    if (opened) {
      closepl();
      opened = NO;
    }
    for (n = 0; n < nErrors; n++)
      fprintf(stderr, "%s\n", errors[n]);
  }
  exit(0);
}

/* Install the interrupt handler (pquit) and open the output device. */
void pinit(void)
{
	signal(SIGINT, pquit);
	openpl();
	opened = YES;
	oldGrayLevel = 0;	/* GBM 27 March 2001 */
}

/* Convert a string to a double.  The special string "-" is evaluated as
   DEFAULT (defined in plt.h). */
double DoubleNum(char *str)
{
  double dbl;
  char line[200], check_char;

  if (strcmp("-", str) == 0)
    return (FDEFAULT);
#ifndef __CYGWIN__
  strncpy(line, str, sizeof(line)-2);
  strcat(line, "?");
  check_char = 0;
  sscanf(line, "%lf%c", &dbl, &check_char);
  if (check_char != '?') err(YES, "Expected number, found `%s'", str);
#else
  /* Cygwin 1.3.1 has a buggy sscanf, so we use atof to do the conversion
     instead, and forego the input-checking. */
  dbl = atof(str);
#endif
  return (dbl);
}

/* Convert a string to a long integer (via DoubleNum, above). */
long LongNum(char *str)
{
  double dbl;

  dbl = DoubleNum(str);
  if ((int)dbl != dbl)
    err(YES, "Expected integer, found `%s'", str);
  return ((int)dbl);
}
  

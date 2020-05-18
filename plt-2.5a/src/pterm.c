/* file: pterm.c	Paul Albrecht		September 1984
			Last revised:		11 April 2001
Device setup for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  29 March 2001; add SPECIAL_FNT | ROT_LABEL to xw, matched bounds to lw)
  11 April 2001: general cleanup (removed obsolete device definitions, still
		 available in classic plt)
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

All xfull and yfull dimensions should be in the thousands in order to avoid
roundoff errors in small motions.
*/

#include	"plt.h"

struct pdev pdevs[] = {
  "xw", SPECIAL_FNT | ROT_LABEL,
  4096, 4096, 3120, 0.20, 0.95, 0.20, 0.85, 88, 56, 0, 0,
  /* lw is special - initialization determines xfull, yfull */
  "lw",	SEP_GRAPH | SPECIAL_FNT | ROT_LABEL,
  0, 0, 0, 0.20, 0.95, 0.20, 0.85, 0, 0, 7.2, 6,
};

int PTERMLookup(char *ptermName, char *hardwired)
{
  if (hardwired) {
    for (p=pdevs; p < ENDP(pdevs); p++)
      if (strcmp(hardwired,p->pterm) == 0)
	break;
    if (p == ENDP(pdevs))
      err(NO, "Driver error, no PTERM `%s'");
    if (strcmp(ptermName,hardwired) != 0)
      err(NO, "Program hardwired for PTERM `%s'", hardwired);
  }
  else {
    for (p=pdevs; p < ENDP(pdevs); p++)
      if (strcmp(ptermName,p->pterm) == 0)
	break;
    if (p == ENDP(pdevs))
      err(NO, "Unrecognized PTERM `%s'", ptermName);
  }
  if (p == ENDP(pdevs)) p = pdevs;
  if ((p->mode&SEP_GRAPH) == 0 && !isatty(fileno(stdout)))
    p->mode |= SEP_GRAPH;
  if (fixed_font == DEFAULT)
    fixed_font = !(p->mode & SPECIAL_FNT);
  chht = p->ch;
  chwd = p->cw;
  if (p->xinches == 0)
    p->xinches = 8;
  if (p->yinches == 0)
    p->yinches = 6;
  xinch = p->xfull/p->xinches;
  yinch = p->yfull/p->yinches;
}

/* file: data.c		Paul Albrecht		September 1987
			Last revised:	        25 March 2009
Data-reading functions for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  26 March 2001: added (previously missing) x-axis range calculation for
                 NCOLZERO plot mode
  10 April 2001: incorporated rewritten low-level input functions with
		 interfaces as defined in Paul's libc library;  these
		 fix a bug that caused loss of the first data row when
		 reading from a pipe, and also add support for reading
		 most CSV-format files and for ignoring comments and
		 empty lines in data files
  14 November 2002: fixed missing cast in azmem call
  25 March 2009: added HISTOGRAM mode
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

#include <fcntl.h>
#include "plt.h"

static void DataLimits();

#ifndef	MAXNPTS
#include <limits.h>
#ifdef UINT_MAX
/* For most environments, UINT_MAX is 2^32 - 1 (around 4.2 billion).  Note that
   plt allocates memory as needed;  changing MAXNPTS affects only how many
   points can be plotted, not the amount of memory normally used by plt. */
#define MAXNPTS UINT_MAX
#else
/* This definition is just in case someone tries to compile this with an
   old 16-bit compiler. */
#define MAXNPTS	65535
#endif
#endif

#define ALL (-1)
#define MAXFIL 5
#define MAXCOL 16
#define MAXLEN 512
#define MAXTC (MAXLEN/sizeof(float))

#define DIGIT(C) (((C) >= '0' && (C) <= '9')|| (C) == '#' || (C) == '%')

typedef union {
    double *d;
    float *f;
    short *s;
    char *c;
} Pointer;

typedef	struct {
    FILE *fp;		/* file pointer */
    char *name;		/* name of the file */
    char eof;		/* YES if eof reached, NO otherwise */
    char raw;		/* 0 = ascii data, otherwize size of binary num */
    short ac;		/* ALL if all columns needed, NO otherwise */
    long tc;		/* columns in first row of file */
    short lc;		/* 1 + largest column number required from file */
    short nc;		/* number of columns to take from the file */
    short nb;		/* number of bytes per row for raw data file */
    short col[MAXCOL];	/* columns requested from specified file */
    long rf;		/* first row wanted from the file */
    long rt;		/* last row */
    long rs;		/* row step, take every nth row */
    long row;		/* current row */
    long beginData;	/* byte at which data begins */
} DFILE;

/* Prototypes for functions defined in this module. */
void DataInit(char **argv);
void ReadPoints(void);
Boolean DataRead(Boolean initialize);
Boolean ReverseDataRead(Boolean initialize);

static void DataLimits(void);
static void GetNum(char **numsp, long *longint);
static int get_strings(DFILE *f, long row);
static int data_files(char **list);
static Boolean data_read(double *outData);
static Boolean data_row(DFILE *f);

void DataInit(char **argv)
{
    if (df[0])
	argv = &df[-1];
    Data.nCols = argv[1] ? data_files(argv) : 0;
    if (Plot.pModes == 0)
	Plot.pModes = zmem(10, 1);
    switch (Plot.xDrive ? Data.nCols+1 : Data.nCols) {
    case  1:
	Plot.xDrive = YES;
	if (Plot.xFrom == DEFAULT)
	    Plot.xFrom = 0;
	if (Plot.xIncr == DEFAULT)
	    Plot.xIncr = 1;
	break;
    case  2:
	break;
    case  3:
	Plot.defaultPMode = CDRIVEN;
	break;
    }
    if (Plot.xDrive)
	Data.nCols++;
    if (Data.nCols)
	Data.row = (double *)zmem(Data.nCols, sizeof(double));
}

void ReadPoints(void)
{
    Uint addPts;
    int n;

    if (Data.nCols == 0)
	return;
    while (data_read(Plot.xDrive ? &Data.row[1] : &Data.row[0])) {
	if (Plot.xDrive) {
	    Data.row[0] = Plot.xFrom;
	    Plot.xFrom += Plot.xIncr;
	}
	if (Data.nPts+Data.nCols >= Data.maxPts) {
	    if (Data.maxPts >= MAXNPTS)
		err(YES, "Too many points");
	    addPts = 10000;
	    if (Data.maxPts+addPts > MAXNPTS)
		addPts = MAXNPTS-Data.maxPts;
	    Data.pts = (float *)azmem(Data.pts, (size_t *)&Data.maxPts, addPts,
				      sizeof(*Data.pts));
	}
	for (n = 0; n < Data.nCols; n++)
	    Data.pts[Data.nPts++] = Data.row[n];
	DataLimits();
    }
}

static void DataLimits(void)
{
    PltPtr plt;
    double cx, cy;
    int n;

    for (n = 0, plt = Plot.plts;  n < Plot.nPlts;  n++, plt++) {
	switch (plt->pm) {
	case LINES:
	case HISTOGRAM:
	    cx = Data.row[plt->c2];
	    if (cx < xmin) xmin = cx;
	    if (cx > xmax) xmax = cx;
	    cy = Data.row[plt->c3];
	    if (cy < ymin) ymin = cy;
	    if (cy > ymax)	ymax = cy;
	case NORMAL:	/* LINES must flow into this */
	case DARKNORMAL:
	case FILLED:
	case LABEL_N:
	case CDRIVEN:
	case IMPULSE:
	case SYMBOL:
	case SCATTER:
	case NCOLZERO:
	    cx = Data.row[plt->c0];
	    if (cx < xmin) xmin = cx;
	    if (cx > xmax) xmax = cx;
	    cy = Data.row[plt->c1];
	    if (cy < ymin) ymin = cy;
	    if (cy > ymax) ymax = cy;
	    break;
	case OUTLINE:
	case OUTLINEFILL:
	case FILLBETWEEN:
	    cx = Data.row[plt->c0];
	    if (cx < xmin) xmin = cx;
	    if (cx > xmax) xmax = cx;
	    cy = Data.row[plt->c1];
	    if (cy < ymin) ymin = cy;
	    if (cy > ymax) ymax = cy;
	    cy = Data.row[plt->c2];
	    if (cy < ymin) ymin = cy;
	    if (cy > ymax) ymax = cy;
	    break;
	case SYMBOL_STD:
	case SCATTER_STD:
	    cy = Data.row[plt->c1] + Data.row[plt->c2];
	    if (cy > ymax) ymax = cy;
	    cy -= Data.row[plt->c2]*2;
	    if (cy < ymin) ymin = cy;
	    cx = Data.row[plt->c0];
	    if (cx < xmin) xmin = cx;
	    if (cx > xmax) xmax = cx;
	    break;
	}
    }
}

Boolean DataRead(Boolean initialize)
{
    static long	curpts;
    int n;

    if (initialize) {
	if (!Plot.quickPlot)
	    curpts = 0;
	return (NO);	/* NO is for lint */
    }
    if (Plot.quickPlot) {
	if (Plot.xDrive) {
	    Data.row[0] = Plot.xFrom;
	    Plot.xFrom += Plot.xIncr;
	    return (data_read(&Data.row[1]));
	}
	return (data_read(&Data.row[0]));
    }
    if (curpts < Data.nPts) {
	for (n=0;  n < Data.nCols;  n++)
	    Data.row[n] = Data.pts[curpts++];
	return (YES);
    }
    return (NO);
}

Boolean ReverseDataRead(Boolean initialize)
{
    static long curPts;
    unsigned int n;
    float *fltPtr;

    if (initialize) {
	if (Plot.quickPlot)
	    return (NO);	/* NO is for lint */
	curPts = 0;
	return (NO);	/* NO is for lint */
    }
    if (Plot.quickPlot) {
	if (Plot.xDrive) {
	    Data.row[0] = Plot.xFrom;
	    Plot.xFrom += Plot.xIncr;
	    return (data_read(&Data.row[1]));
	}
	return (data_read(&Data.row[0]));
    }
    if (curPts < Data.nPts) {
	curPts += Data.nCols;
	fltPtr = &Data.pts[Data.nPts-curPts];
	for (n=0;  n < Data.nCols;  n++)
	    Data.row[n] = *fltPtr++;
	return (YES);
    }
    return (NO);
}

extern double atof();
extern long atol();

static DFILE datafile[MAXFIL];
static short nf = 0;	/* number of files being read */
static char *str[MAXTC];
static char linebuf[MAXLEN];

/* GetNum attempts to read an unsigned long int from a string.  If the first
   character of the string is not a digit, GetNum simply increments
   the string pointer.  Otherwise, it reads consecutive digits, converts
   them into a long int (note that it does not check for overflow), stores
   them in the location provided by its second argument, and sets the string
   pointer to point to the second character after the end of the digit string.

   This function is used only by data_files(), to read data format
   specification strings of the form ":nnnn,nnnn,nnnn".
*/
static void GetNum(char **numsp, long *longint)
{
    long n = 0L;
    char *nums = *numsp;

    while ('0' <= *nums && *nums <= '9')
	n = 10*n + (int)(*nums++ - '0');

    if (*numsp != nums)
	*longint = n;
    *numsp = nums + 1;
}

/* get_strings attempts to read the next line of the (text) data file specified
   by its argument.  It reads one character at a time until it encounters
   either a newline or EOF (thus it correctly handles the case of a data file
   that does not end with a newline).  As it reads, it sets the str[] pointers
   to point to the first non-whitespace character in each field.  It returns
   the number of fields found in the line.

   get_strings is used once by data_files(), to determine the number of data
   columns (fields) in the first row (line);  and by data_read, to read and
   parse all subsequent rows in the data file.

   Although from its argument list you might guess that get_strings will read
   an arbitrary row number, in fact the "row" argument only tells get_strings
   if it is being asked to reread the immediately previous row, in which case
   get_strings simply returns the previously counted number of fields.
*/
static int get_strings(DFILE *f, long row)
{
    FILE *fptr = f->fp;
    int c, ignore = 0, inword = 0;
    static int n = 0, prev_row = -1;
    char *lptr = linebuf;

    if (row == prev_row)
	return (n);
    else {
	n = 0;
	prev_row = row;
    }

    while ((c = getc(fptr)) != EOF && n < MAXTC && lptr < linebuf + MAXLEN) {
	switch (c) {
	case '\n':	/* end of line; return only if line was not empty */
	    ignore = inword = 0;
	    if (n > 0) {
		*lptr = '\0';
		return (n);
	    }
	    break;
	case ' ':     	/* whitespace separates columns */
	case '\t':
	case ',':	/* commas also separate columns, ... */
	case '\'':	/* ... as do single ... */
	case '"':	/* ... and double quotes */
	    if (inword) *lptr++ = ' ';
	    inword = 0;
	    break;
	case '#':	/* ignore remainder of this line */
	    ignore = 1;
	    inword = 0;
	    break;
	default:	/* non-space, non-comment character */
	    if (!ignore) {
		if (!inword) {
		    str[n++] = lptr;	/* first in this word -- count it */
		    inword = 1;
		}
		*lptr++ = c;
	    }
	    break;
	}
    }
    if (c == EOF) {
	f->eof = YES;
	*lptr = ' ';
	return (n);
    }
    if (n >= MAXTC)
	err(YES, "Too many columns in %s, row %ld", f->name, f->row);
    if (lptr >= linebuf + MAXLEN)
	err(YES, "Row %ld too long in %s", f->row, f->name);
    err(YES, "Error in get_strings");
}

static int data_files(char **list)
{
    static char *allist[] = { "%", 0 };
    DFILE *f;
    int n, grand_tot = 0;
    char c, *args;

    if (*(++list) == 0)
	list = allist;

    for (nf = n = 0; n < MAXFIL; n++) {
	f = &datafile[n];
	f->fp = stdin;
	f->name = "standard-input";
	f->eof = f->raw = f->ac = f->nc = f->lc = f->beginData = f->row = 0;
	f->rf = f->rt = f->rs = -1;
    }

    while (*list) {
	if (nf == MAXFIL)
	    err(YES, "Can't have more than %d files", MAXFIL);
	f = &datafile[nf];
	if (**list == ':') {
	    args = *(list++) + 1;
	    switch (*args) {
	    case 's': f->raw = sizeof(short); break;
	    case 'f': f->raw = sizeof(float); break;
	    case 'd':	f->raw = sizeof(double); break;
	    }
	    if (f->raw) {
		args++;
		GetNum(&args, &f->tc);
	    }
	    GetNum(&args,&f->rf);
	    GetNum(&args,&f->rt);
	    GetNum(&args,&f->rs);
	}
	else {
	    if (!DIGIT(**list)) {
		if (*list == 0)
		    err(YES, "no name specified for required input file");
		if (strcmp(*list, "stdin") == 0)
		    f->fp = stdin;
		if ((f->fp = fopen(*list, "r")) == 0)
		    err(YES, "can't open %s\n", *list);
		f->name = *list++;
	    }
	    while (*list && DIGIT(**list)) {
		c = **list++;
		if (c != '#' && c !=  '%') {
		    n = 1 + (f->col[f->nc] = atoi(list[-1]));
		    if (f->lc < n)
			f->lc = n;
		}
		else 
		    f->col[f->nc] = f->ac = ALL;
		if (++f->nc > MAXCOL)
		    err(YES, "Can't take more than %d columns from one file",
			  MAXCOL);
	    }
	    nf++;
	    if (f->nc == 0)
		f->col[f->nc++] = f->ac = ALL;
	    if (f->raw == 0) {
		c = getc(f->fp);
		if (c == sizeof(short) || c == sizeof(float) ||
		    c == sizeof(double)) {
		    f->beginData = 2;
		    f->raw = c;
		    f->tc  = getc(f->fp);
		}
		else {		/* text input */
		    long pos;

		    ungetc(c, f->fp);
		    if ((f->tc = get_strings(f,1)) > sizeof(str)/2)
			err(YES, "Too many columns in %s", f->name);
		}
		if (f->tc < f->lc)
		    err(YES, "Too few columns in %s", f->name);
	    }
	    
	    if ((f->nb=f->raw*f->tc) > sizeof(linebuf))
		err(YES, "Too many columns in %s", f->name);
	    if (f->ac == ALL) f->lc = f->tc;
	    if (f->lc == 0) err(YES, "No columns from %s", f->name);
	    if (f->rf == -1) f->rf = 0;
	    if (f->rt == -1) f->rt = 07777777777;
	    if (f->rs == -1) f->rs = 1;
	    
	    if (f->fp != stdin && f->rf > f->row && f->raw != 0) {
		if (fseek(f->fp, f->beginData+f->raw*(long)f->nb*(f->rf-1), 0)
		    == -1)
		    fseek(f->fp, f->beginData, 0);
		else
		    f->row = f->rf;
	    }
	    while (f->rf > f->row) {
		data_row(f);
		if (f->eof)
		    err(YES, "Only %ld rows in %s", f->row-1, f->name);
	    }
	    for (n=0; n < f->nc; n++)
		grand_tot += (f->col[n] == ALL) ? f->tc : 1;
	}
    }
    return (grand_tot);
}

static Boolean data_read(double *outData)
{
    DFILE *f;
    int c, i, j, k;
    Pointer inData;

    inData.c = linebuf;
    for (i = 0; i < nf; i++) {
	if (!data_row(f = &datafile[i]))
	    return (NO);
	for (j = 0; j < f->nc; j++) {
	    if ((c = f->col[j]) == ALL) {
		for (k = 0; k < f->tc; k++) {
		    switch (f->raw) {
		    case 0:
			*outData++ = atof(str[k]);
			break;
		    case sizeof(short):
			*outData++ = inData.s[k];
			break;
		    case sizeof(float):
			*outData++ = inData.f[k];
			break;
		    case sizeof(double):
			*outData++ = inData.d[k];
			break;
		    }
		}
	    }
	    else {
		switch (f->raw) {
		case 0:
		    *outData++ = atof(str[c]);
		    break;
		case sizeof(short):
		    *outData++ = inData.s[c];
		    break;
		case sizeof(float):
		    *outData++ = inData.f[c];
		    break;
		case sizeof(double):
		    *outData++ = inData.d[c];
		    break;
		}
	    }
	}

	for (j = 1; j < f->rs; j++)
	    data_row(f);
    }
    return (YES);
}

static Boolean data_row(DFILE *f)
{
    int n, c = 0;

    if (f->row++ > f->rt)
	return (NO);

    if (!f->raw) {
	c = get_strings(f, f->row);
	if (f->eof)
	    return (NO);
	if (c < f->lc) {
	    err(NO, "Too few columns in file %s row %ld\n", f->name, f->row);
	    f->eof = YES;
	    return (NO);
	}
	else
	    return (YES);
    }

    for (n=0; n < f->nb && c != EOF; linebuf[n++]=c=getc(f->fp));

    if (n == f->nb)
	return (YES);

    if (n != 1)
	err(NO, "Row %ld incomplete in %s\n", f->row, f->name);

    f->eof = YES;
    return (NO);
}

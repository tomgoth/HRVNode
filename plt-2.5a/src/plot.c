/* file: plot.c		Paul Albrecht		September 1984
			Last revised:	      	25 March 2009
Generate data plots

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  15 March 2001: fixed FILLED mode
  15 April 2001: general cleanup
  14 November 2002: fixed missing casts in azmem calls
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

#include "plt.h"

#define GETPM(DFLT) (*cp ? *cp++ : DFLT)
#define UNGETPM(CHR) { if( *(--cp) != CHR) ASSERT(NO, "Bug in UNGETPM"); }
#define PMBLANK(C) ((C) == ' ' || (C) == ',' || (C) == '/')

/* Prototypes of functions defined in this module */
void PlotDef(char *pspec);
void PlotDraw(PltPtr plt);
void PlotInit(Mode mode);
void PlotNameDef(long n, char *name);
void SmallBox(Ptype x, Ptype y);
Ptype X(double x);
Ptype Y(double y);

static void GetScatInfo(PltPtr plt);
static void GetCols(PltPtr plt, int n);
static void PlotText(Ptype x, Ptype y, double dbl);
static char *FontInfoString(double *dptr);
static void spen(PltPtr plt, double dx, double dy, double yerr);
static Boolean pen(Boolean draw, Ptype x, Ptype y);
static Boolean OutOfRange(Ptype x, Ptype y);

static short lastuse, nextuse, use[20], useall, nextcol;
static char *cp;

void PlotInit(Mode mode)
{
    PltPtr plt;
    short n;

    for (n = 0; n < Plot.nPlts;  n++) {
	plt = &Plot.plts[n];
	if (plt->fgName) FREE(plt->fgName);
	if (plt->name) FREE(plt->name);
    }
    if (Plot.plts != 0)	FREE(Plot.plts);
    Plot.nPlts = 0;
    Plot.suppress = zmem(20, 1);
    Plot.defaultPMode = NORMAL;
    Plot.exclude = YES;
    Plot.xFrom = FDEFAULT;
    Plot.xIncr = FDEFAULT;
}

/* Match up plot specification (-p option) with columns */
void PlotDef(char *pspec)
{
    PltPtr plt;
    short n;
    char c, cf, str[100];

    nextcol = nextuse = lastuse = 0;
    useall = YES;
    cp = pspec;
    while (*cp || (useall && nextcol < Data.nCols)) {
	if (*cp == 0) {
	    if	(!useall || nextcol == Data.nCols)
		break;
	}
	if (Plot.nPlts == Plot.maxPlts)
	    Plot.plts = (PltPtr)azmem(Plot.plts, (size_t *)&Plot.maxPlts, 2,
				      sizeof(*Plot.plts));
	plt = &Plot.plts[Plot.nPlts++];
	plt->c2 = NO_COL;
	plt->c3 = NO_COL;
	while (YES) {
	    while (PMBLANK(*cp))
		cp++;
	    if (*cp < '0' || *cp > '9')
		break;
	    n = 0;
	    while (*cp >= '0' && *cp <= '9')
		n = 10*n + ((*cp++ -'0')&0177);
	    if (n > Data.nCols)
		err(YES, "Bad column: %d", n);
	    use[lastuse++] = n;
	    useall = NO;
	}
	while (PMBLANK(*cp))
	    cp++;
	if (*cp == '(' || *cp == '[')
	    plt->pm = Plot.defaultPMode;
	else
	    plt->pm = GETPM(Plot.defaultPMode);
	Plot.defaultPMode = NORMAL;
	switch (plt->pm) {
	case LABEL_N:
	case OUTLINEFILL:
	case FILLBETWEEN:
	case OUTLINE:
	case CDRIVEN:	GetCols(plt, 3); break;
	case IMPULSE:	ymin = ymax = 0; /* Flow through to NORMAL */
	case DARKNORMAL:
	case FILLED:
	case NORMAL: 	GetCols(plt, 2); break;
	case NCOLZERO:	GetCols(plt, 1); plt->c1 = plt->c0; plt->c0 = 0; break;
	case HISTOGRAM:
	case LINES:	GetCols(plt, 4); break;
	case SYMBOL:
	case SCATTER:
	case SYMBOL_STD:
	case SCATTER_STD: GetScatInfo(plt); break;
	default: err(YES, "Bad plot mode spec `%c'", cp[-1]); break;
	}
	if (nextuse != lastuse)
	    err(YES, "Too many columns for `%c'", plt->pm);
	lastuse = nextuse = 0;
	while (PMBLANK(*cp))
	    cp++;
	if (*cp == '[' || *cp == '(') {
	    n = 0;
	    cf = *cp++;
	    do {
		str[n++] = c = *cp++;
	    } while(c && c != ']' && c != ')');
	    if (c != ']' && c != ')')
		err(YES, "Unterminated %c in `%s'", cf, Plot.pModes);
	    str[n-1] = 0;
	    plt->fgName = StringSave(str);
	}
    }
    if (*cp)
	err(YES, "Not enough columns for `%s'", cp);
}

static void GetScatInfo(PltPtr plt)
{
    static struct {
	char *name;
	short n;
    } *s, sym[] = {	/* The first 10 entries must not be altered !!! ... */
	"0", 0, "1", 1, "2", 2, "3", 3, "4", 4,	"5", 5, "6", 6, "7", 7, "8", 8,
	"9", 9,
	"circle", 0, "utriangle", 1, "diamond", 2, "square", 3, "triangle", 4,
	"fcircle", 5, "futriangle", 6, "fdiamond", 7, "fsquare", 8,
	"ftriangle", 9
    };
    short nchrs;
    char c, symname[20];

    if (plt->pm == SCATTER_STD || plt->pm == SYMBOL_STD) {
	GetCols(plt, 3);
	c = GETPM(SYMMETRIC_STD);
	if (c == SYMMETRIC_STD || c==UP_STD || c==DOWN_STD) {
	    plt->subpm = c | 040;
	    c = GETPM('*');
	}
	else
	    plt->subpm = SYMMETRIC_STD;
    }
    else {
	GetCols(plt, 2);
	plt->subpm = 0;
	c = GETPM('.');
    }
    if (plt->pm == SYMBOL || plt->pm == SYMBOL_STD) {
	while (PMBLANK(c))
	    c = GETPM(0);
	for (nchrs = 0; nchrs < sizeof(symname)-1; nchrs++) {
	    symname[nchrs] = c;
	    if ((c < 'a' || c > 'z') && (c < '0' || c > '9'))
		break;
	    c = GETPM(0);
	}
	symname[nchrs] = 0;
	if (c != 0)
	    UNGETPM(c);
	for (s = sym; YES;  s++) {
	    if (strcmp(s->name,symname) == 0 ||
	       strncmp(s->name,symname,nchrs) == 0) 
		break;
	    if (s+1 == ENDP(sym))
		err(YES, "No symbol type `%s'", symname);
	}		
	plt->pc = *sym[s->n].name;	/* ... and this is why !!!!!! */
	plt->symbol = YES;
    }
    else
	plt->pc = c;
}

static void GetCols(PltPtr plt, int n)
{
    short *col;

    col = &plt->c0;
    while (n-- > 0) {
	if (lastuse > 0) {
	    if (nextuse < lastuse)
		*col++ = use[nextuse++];
	    else
		err(YES, "Too few columns for `%c'", plt->pm);
	}
	else {
	    if (nextcol < Data.nCols)
		*col++ = nextcol++;
	    else
		err(YES, "Ran out of columns for `%c'", plt->pm);
	}
    }
}

void PlotNameDef(long n, char *name)
{
    while (n >= Plot.maxPlts)
	Plot.plts = (PltPtr)azmem(Plot.plts, (size_t *)&Plot.maxPlts, 5,
				  sizeof(*Plot.plts));
    Plot.plts[n].name = StringSave(name);
}

/* Output the plot specified by structure *plt */
void PlotDraw(PltPtr plt)
{
    PtrUnion arg0, arg1;
    double *c0, *c1, *c2, *c3, gray;
    Ptype n, x, y, xFirst, yFirst;
    Boolean dflag, pflag;

    c0  = &Data.row[plt->c0];
    c1  = &Data.row[plt->c1];
    c2 = (plt->c2 == NO_COL) ? 0 : &Data.row[plt->c2];
    c3 = (plt->c3 == NO_COL) ? 0 : &Data.row[plt->c3];
    FontGroupSelect("p", plt->fgName);
    dflag = pflag = NO;
    DataRead(YES);
    xFirst = DEFAULT;

    while (DataRead(NO)) {
	x = X(*c0);
	y = Y(*c1);
	if (xFirst == DEFAULT) {
	    xFirst = x;
	    yFirst = y;
	}
	switch (plt->pm) {
	case LABEL_N:
	    if (!OutOfRange(x,y))
		PlotText(x, y, *c2);
	    break;
	case CDRIVEN:	/* kludgy mode, but flexible */
	    n = *c2;
	    if (n == CSTROKE || n == CFILL || n == CBBFILL) {
		if (dflag)
		    PolyDef(x, y, n);
		dflag = NO;
	    }
	    else if (n >= CTEXT)
		n = CTEXT;
	    else if (n >= CSYMBOL && n < CSYMBOL+10) /* up to 10 symbols */
		n = CSYMBOL;
	    else if (n == CFILLI || n == CFILLI+1)
		n = CFILLI;
	    else if (n == CBBFILLI || n == CBBFILLI+1)
		n = CBBFILLI;
	    else if (dflag && n != CCONT) {
		PolyDef(0, 0, COSTROKE);
		dflag = NO;
	    }
	    switch (n) {
	    case CCONT:	PolyDef(x, y, dflag ? CCONT : CMOVE);
		dflag = YES;
		break;
	    case CMOVE:	PolyDef(x, y, CMOVE);
		dflag = YES;
		break;
	    case CDOT:	plt->pc = '.';
		spen(plt, x, y, 0.0);
		break;
	    case CBOX:	SmallBox(x, y);
		break;
	    case CBBFILLI:
	    case CFILLI: gray = *c2 - n;
		arg0.d = &gray;
		special(SETGRAYD, arg0, arg1);
		PolyDef(x, y, n);
		arg0.d = &oldGrayLevel;
		special(SETGRAY, arg0, arg1);
		dflag = NO;
		break;
	    case CSYMBOL: plt->pc = '0'+ (short)(*c2-n);
		plt->symbol = YES;
		scatterplot(plt, x, y, y, y);
		dflag = plt->symbol = NO;
		break;
	    case CTEXT:	PlotText(x, y, *c2-n);
		dflag = NO;
		break;
	    case CHANGEFNT:
		old_font = FontInfoString(c0);
		if (*c1 > 0)
		    old_ps = *c1;
		arg0.c = old_font;
		arg1.d = &old_ps;
		special(SETFONT, arg0, arg1);
		break;
	    case CHANGEPS:
		old_ps = *c0;
		arg0.c = old_font;
		arg1.d = &old_ps;
		special(SETFONT, arg0, arg1);
		break;
	    case CHANGELW:
		old_lw = *c0;
		arg0.d = &old_lw;
		special(SETLINEWIDTH, arg0, arg1);
		break;
	    case CHANGEGRAY:
		oldGrayLevel = *c0;
		arg0.d = &oldGrayLevel;
		special(SETGRAY, arg0, arg1);
		break;
	    case CHANGECOLOR:
		arg0.c = FontInfoString(c0);
		special(SETCOLOR, arg0, arg1);
		break;
	    case CHANGELM:
		old_lm = FontInfoString(c0);
		arg0.c = old_lm;
		special(SETLINEMODE, arg0, arg1);
		break;
	    }
	    break;
	case DARKNORMAL:
	case OUTLINEFILL:
	    if (dflag)
		PolyDef(x, y, CBBCONT);
	    else 
		PolyDef(x, y, CMOVE);
	    dflag = YES;
	    break;
	case FILLED:
	case FILLBETWEEN:
	case OUTLINE:
	    if (dflag)
		PolyDef(x, y, CCONT);
	    else 
		PolyDef(x, y, CMOVE);
	    dflag = YES;
	    break;
	case NCOLZERO:
	case NORMAL:
	    dflag = pen(dflag, x, y);
	    pflag |= dflag;
	    break;
	case IMPULSE:
	    pen(NO, x, Y(0.0));
	    pen(YES, x, y);
	    SmallBox(x, y);
	    break;
	case SYMBOL:
	case SCATTER:
	    spen(plt, *c0, *c1, 0.0);
	    break;
	case SYMBOL_STD:
	case SCATTER_STD:
	    spen(plt, *c0, *c1, *c2);
	    break;
	case LINES:
	    pen(NO,  x, y);
	    pen(YES, X(*c2), Y(*c3));
	    break;
	case HISTOGRAM:
	    PolyDef(x, y, CMOVE);
	    PolyDef(x, Y(*c3), CCONT);
	    PolyDef(X(*c2), Y(*c3), CCONT);
	    PolyDef(X(*c2), y, CCONT);
	    PolyDef(x, y, CFILL);
	    break;
	}
    }
    switch (plt->pm) {
    case CDRIVEN:
	if(dflag)
	    PolyDef(x, y, COSTROKE);
	break;
    case DARKNORMAL:
	y = Y(ya.min);
	PolyDef(x, y, CBBCONT);
	PolyDef(xFirst, y, CBBFILL);
	break;
    case FILLBETWEEN:
	ReverseDataRead(YES);
	while (ReverseDataRead(NO))
	    PolyDef(X(*c0), Y(*c2), CBBCONT);
	PolyDef(xFirst, yFirst, CFILL);
	break;
    case FILLED:
	PolyDef(xFirst, yFirst, CFILL);
	break;
    case OUTLINEFILL:
	ReverseDataRead(YES);
	while (ReverseDataRead(NO))
	    PolyDef(X(*c0), Y(*c2), CBBCONT);
	PolyDef(xFirst, yFirst, CBBFILL);
	break;
    case OUTLINE:
	ReverseDataRead(YES);
	while (ReverseDataRead(NO))
	    PolyDef(X(*c0), Y(*c2), CCONT);
	PolyDef(xFirst, yFirst, CSTROKE);
	break;
    }
    if (plt->name && pflag)
	label(plt->name);
    else
	pen(NO, xwmin, ywmin);
}

/* Output a string provided by the -tf or -ts option */
static void PlotText(Ptype x, Ptype y, double dbl)
{
    short n;
    char lbl[30];

    if (pstr.n > 0) {
	n = dbl;
	if (n >= 0 && n < pstr.n)
	    plabel(pstr.list[n], x, y, pstr.just, 0.0);
	else
	    err(NO, "Plot string %g undefined", dbl);
    }
    else {
	sprintf(lbl, "%g", dbl);
	plabel(lbl, x, y, "CC", 0.0);
    }
}

/* Match the index *dptr with a string provided by the -fs option */
static char *FontInfoString(double *dptr)
{
    short n;

    n = *dptr;
    if (fgstr.list == 0 || n < 0 || n >= fgstr.n)
	err(YES, "No string for font index %lg", *dptr);
    return (fgstr.list[n]);
}

void SmallBox(Ptype x, Ptype y)
{
    static Ptype	xd, yd;

    if (xd == 0) {
	xd = (p->xfull+256)/512;
	yd = (p->yfull+195)/390;
    }
    move(x-xd, y-yd);
    cont(x+xd, y-yd);
    cont(x+xd, y+yd);
    cont(x-xd, y+yd);
    cont(x-xd, y-yd);
}

static void spen(PltPtr plt, double dx, double dy, double yerr)
{
    Ptype x, y, yneg, ypos;

    x = X(dx);
    y = Y(dy);
    if (OutOfRange(x,y)) {
	Plot.excluded++;
	return;
    }
    switch (plt->pm ) {
    case SYMBOL:
    case SCATTER:
	if (plt->pc == ' ')
	    SmallBox(x, y);
	else
	    scatterplot(plt, x, y, y, y);
	break;
    case SYMBOL_STD:
    case SCATTER_STD:
	if (plt->pc == ' ') {
	    SmallBox(x, y);
	    break;
	}
	yneg = Y(dy-yerr);
	ypos = Y(dy+yerr);
	switch (plt->subpm) {
	case SYMMETRIC_STD: break;
	case UP_STD:	yneg = y; break;
	case DOWN_STD:	ypos = y; break;
	}
	scatterplot(plt, x, y, yneg, ypos);
	break;
    }
}

static Boolean pen(Boolean draw, Ptype x, Ptype y)
{
    static Boolean oldOff = YES, off;

    off = OutOfRange(x, y);
    if (off) {
	Plot.excluded++;
	return (NO);
    }
    if (!draw || oldOff)
	move(x, y);
    else
	cont(x, y);
    oldOff = off;
    return (YES);
}

static Boolean OutOfRange(Ptype x, Ptype y)
{
    if (Plot.exclude)
	return	((x < xwmin) || (x > xwmax) || (y < ywmin) || (y > ywmax));
    else
	return((x < 0) || (x > p->xfull) || (y < 0) || (y > p->yfull));
}

Ptype X(double xpt)
{
    Ptype x = xpt * xa.scl + xa.off;

    if (x < 0)
	x = 0;
    else if (x > p->xfull)
	x = p->xfull;
    return (x);
}

Ptype Y(double ypt)
{
    Ptype y = ypt * ya.scl + ya.off;

    if (y < 0)
	y = 0;
    else if (y > p->yfull)
	y = p->yfull;
    return (y);
}

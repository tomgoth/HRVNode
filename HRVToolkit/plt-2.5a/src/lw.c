/* file: lw.c		Paul Albrecht		July 1987
 			Last revised:	      7 November 2002
PostScript driver for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  23 February 2001: added color support including the X11R6 color database
  30 March 2001: added LN, CN, RN, label concatenation
  11 April 2001: general cleanup
  10 May 2001: replaced old elabel() with implementation from xw.c
  7 November 2002: corrected axis title placement when using log axes
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

The plotting space for PostScript output is always the same number (300*2) of
units per inch, so that the line width and font shapes are not changed in size
if the plot is compressed in one dimension.  This code needs the prolog file
plt.pro, which defines the PostScript procedures used here.  For example,
paths are defined with M (move) and N (next) and stroked with L (line) or F
(filled region);  M, N, L, and F are not PostScript primitives, but are
defined by plt.pro.
*/

#include "plt.h"
#include "rgb.h"	/* color definitions from X11R6 */

/* Prototypes of functions defined in this module */
void openpl(void);
void closepl(void);
void erase(void);
void space(Ptype x0, Ptype y0, Ptype x1, Ptype y1);
void label(char *lbl);
void move(Ptype x, Ptype y);
void line(Ptype x0, Ptype y0, Ptype x1, Ptype y1);
void cont(Ptype x, Ptype y);
void plabel(char *lbl, Ptype x, Ptype y, char *just, double angle);
void alabel(char *what, char *lbl, Ptype x, Ptype y);
void elabel(char *what, char *base, char *exp, Ptype x, Ptype y);
void strsize(char *str, Ptype *xsize, Ptype *ysize, double angle);
void scatterplot(PltPtr plt, Ptype x, Ptype y, Ptype yneg, Ptype ypos);
void PolyDef(Ptype x, Ptype y, Const what);
void special(int what, PtrUnion arg0, PtrUnion arg1);

static void sendstring(char *s);
static void flushpath(void);
static void PTERMInit();
static void setfont();
static void linewidth();
static void linemod();
static void setgray();
static void setcolor();
static void PTERMInit(char *pterm);
static void setfont(char *name, double ps);
static void linewidth(double lw);
static void linemod(char *lm);
static void setgray(double grey);
static void setcolor(char *color);

#define MAXPATH 1475	/* Laser Writer can't handle 1500 */

static Ptype oldx, oldy;
static long npath;

void openpl(void)
{
    npath = 0;
}

void closepl(void)
{
    flushpath();
    fflush(stdout);
}

void erase(void)
{
    /* This space intentionally left void :-) */
}

void space(Ptype x0, Ptype y0, Ptype x1, Ptype y1)
{
    printf("%.5lg %.5lg %.5lg %.5lg %.5lg ",
	   fscl, xorig, yorig, xorig+xdim, yorig+ydim);
    printf("%ld %ld %ld %ld WDEF\n", (long)x0, (long)y0, (long)x1, (long)y1);
    setfont(DEFAULT_FONT, (double)DEFAULT_PS);
}

void label(char *lbl)
{
    sendstring(lbl);
    printf(" show\n");
    npath = 0;
}

void move(Ptype x, Ptype y)
{
    flushpath();
    printf("%ld %ld M\n", (long)x, (long)y);
    oldx = x;
    oldy = y;
    npath = 1;
}

void line(Ptype x0, Ptype y0, Ptype x1, Ptype y1)
{
    move(x1, y1);
    printf("%ld %ld L\n", (long)(x0-oldx), (long)(y0-oldy));
    npath = 0;
}

void cont(Ptype x, Ptype y)
{
    if (npath == 0)
	err(YES, "Called cont() before move()");

    if (npath == MAXPATH-1) {
	printf("%ld %ld L\n%ld %ld M\n",
	       (long)(x-oldx), (long)(y-oldy), (long)x, (long)y);
	npath = 1;
    }
    else {
	printf("%ld %ld N\n", (long)(x-oldx), (long)(y-oldy));
	npath++;
    }
    oldx = x;
    oldy = y;
}

static void sendstring(char *s)
{
    char c;

    flushpath();
    putchar('(');
    while (c = *s++) {
	if ((c == ')') || (c == '(') || (c == '\\'))
	    putchar('\\');
	if ((c > 0176) || (c <040)) {
	    putchar('\\');
	    putchar(((c>>6)&07)+'0');
	    putchar(((c>>3)&07)+'0');
	    putchar((c&07)+'0');
	}
	else	putchar(c);
    }
    putchar(')');
    npath = 0;
}

static void flushpath(void)
{
    if (npath > 1) {
	printf("0 0 L\n");
	npath = 0;
    }
}

static struct {
    char *just;
    short pos;
} *j, jtypes[] = { "LB", 0, "LC", 1, "LT", 2, "CT", 3, "RT", 4, "RC", 5,
		   "RB", 6, "CB", 7, "CC", 8, "LN", 9, "CN",10, "RN", 11 };

void plabel(char *lbl, Ptype x, Ptype y, char *just, double angle)
{
    static char jtmp[3];

    if (lbl[0] == 0)
	return;
    for (j=jtypes; j < ENDP(jtypes) ;  j++) {
	if (strcmp(j->just,just) == 0)
	    break;
    }
    if (j == ENDP(jtypes)) {
	err(NO, "Unknown mode `%s' in plabel(%s,...)", just, lbl);
    }
    else {
	sendstring(lbl);
	printf(" %ld %ld %ld %.5lg T\n",
	       (long)x, (long)y, (long)(j->pos), angle);
    }
}

static Ptype xax_off, yax_off, xax_roff, yax_roff;

void alabel(char *what, char *lbl, Ptype x, Ptype y)
{
    sendstring(lbl);
    if (strcmp(what, "YT") == 0)
	x -= yax_off;
    else if (strcmp(what, "YTR") == 0)
	x += yax_roff;
    else if (strcmp(what, "XT") == 0)
	y -= xax_off;
    else if (strcmp(what, "XTR") == 0)
	y += xax_roff;
    printf(" %ld %ld %s\n", (long)x, (long)y, what);
}

static double prev_ps = -1;
static char fontcode[4] = "t";

void elabel(char *what, char *base, char *exponent, Ptype x, Ptype y)
{
    static Ptype xsize, ysize;
    Ptype off;
    char *font_name = StringSave(fontcode);
    double base_ps = prev_ps, exp_ps = prev_ps*0.8;

    strsize(base, &xsize, &ysize, 0.0);
    off = xsize;
    setfont(font_name, exp_ps);
    strsize(exponent, &xsize, &ysize, 0.0);
    off += xsize;

    if (strcmp(what,"XE") == 0) {
	x += chwd/3;
	plabel(exponent,  x+off/2, y, "RT", 0.0);
	setfont(font_name, base_ps);
	plabel(base, x-off/2, y-chht/3, "LT", 0.0);
	xax_off = 3*chht/2;
    }
    else if (strcmp(what,"YE") == 0) {
	x -= chwd/3;
	setfont(font_name, base_ps);
	plabel(base, x-off, y-chht/3, "LB", 0.0);
	setfont(font_name, exp_ps);
	plabel(exponent, x, y+(chht+3)/6, "RB", 0.0);
	if (off > yax_off)
	    yax_off = off;
	setfont(font_name, base_ps);
    }
    else if (strcmp(what,"XER") == 0) {
	x += chwd/3;
	plabel(exponent,  x+off/2, y+3*chht/4, "RB", 0.0);
	setfont(font_name, base_ps);
	plabel(base, x-off/2, y+chht/4, "LB", 0.0);
	xax_roff = 3*chht/2;
    }
    else if (strcmp(what,"YER") == 0) {
	x += chwd/2;
	plabel(exponent, x+off, y+(chht+3)/6, "RB", 0.0);
	setfont(font_name, base_ps);
	plabel(base, x, y-chht/3, "LB", 0.0);
	if (off > yax_roff)
	    yax_roff = off;
    }
    else
	err(YES, "Bad call: elabel(%s,...)", what);
}

/* For the PostScript driver, there is no proper string size function --
   this one gives an approximate size that is usable for determining
   line spacing but not much more.  The code downloaded in plt.pro uses
   the PostScript rendering engine to get accurate string sizes for
   centering and concatenating strings, but these are not reported back
   to plt. */
void strsize(char *str, Ptype *xsize, Ptype *ysize, double angle)
{
    short len;

    if (fabs(angle) < 1 ) {
	*xsize = chwd * strlen(str);
	*ysize = chht;
    }
    else if (fabs(angle-90) < 1) {
	*xsize = chht;
	*ysize = chwd * strlen(str);
    }
    else
	err(NO, "Bad angle %.5lg in for strsize(%s,...)", angle, str);
}

void scatterplot(PltPtr plt, Ptype x, Ptype y, Ptype yneg, Ptype ypos)
{
    static char lbl[2];

    flushpath();
    yneg -= y;
    ypos -= y;
    if (plt->symbol)
	printf("%d %ld %ld %ld %ld SY\n", (int)(plt->pc-'0'),
	       (long)yneg, (long)ypos, (long)x, (long)y);
    else {
	lbl[0] = plt->pc;
	sendstring(lbl);
	printf(" %ld %ld %ld %ld SP\n",
	       (long)yneg, (long)ypos, (long)x, (long)y);
    }
}

void PolyDef(Ptype x, Ptype y, Const what)
{
    int fill;

    fill = -1;
    switch (what) {
    case CMOVE:
	move(x, y);
	break;
    case CBBCONT:
    case CCONT:
	cont(x, y);
	break;
    case CFILL:
    case CFILLI:
	cont(x, y);
	fill = 0;
	break;
    case CSTROKE:
	cont(x, y);	/* flow through */
    case COSTROKE:
	flushpath();
	break;
    case CBBFILL:
    case CBBFILLI:
	cont(x, y);
	fill = 1;
	break;
    default:
	err(YES, "Bad PolyDef() code %d", what);
    }
    if (fill >= 0) {
	printf("%d F\n", fill);
	npath = 0;
    }
    if (npath == MAXPATH)
	fprintf(stderr, "Polygon path exceeds %d sections", MAXPATH);
}

void special(int what, PtrUnion arg0, PtrUnion arg1)
{
    switch (what) {
    case SETPTERM:
	PTERMInit(arg0.c);
	break;
    case SETLINEWIDTH:
	linewidth(*arg0.d);
	break;
    case SETLINEMODE:
	linemod(arg0.c);
	break;
    case SETCOLOR:		/* added 23 Feb 2001 GBM */
	setcolor(arg0.c);
	break;
    case SETFONT:
	setfont(arg0.c, *arg1.d);
	break;
    case SETGRAY:
	flushpath();	/* flow through */
    case SETGRAYD:
	setgray(*arg0.d);
	break;
    }
}

static void PTERMInit(char *pterm)
{
    double f1, f2, f3, f4, f5;
    short n;
    char ptmp[50];

    if (pterm == 0) {
	ptmp[0] = 0;
	n = 1;
    }
    else
	n = sscanf(pterm, "%s%lf%lf%lf%lf%lf", ptmp, &f1, &f2, &f3, &f4, &f5);
    PTERMLookup(ptmp, "lw");	/* defines p */
    if (fscl == DEFAULT)
	fscl = (n > 1) ? f1 : 0.85;
    if (xdim == DEFAULT)
	xdim = (n > 2) ? f2 : p->xinches;
    if (ydim == DEFAULT)
	ydim = (n > 3) ? f3 : p->yinches;
    if (xorig == DEFAULT)
	xorig = (n > 4) ? f4 : 0.5*(8.25-xdim) + 0.25;
    if (yorig == DEFAULT)
	yorig = (n > 5) ? f5 : 0.67*(11-ydim);
    default_ps = DEFAULT_PS;
    xinch = yinch = 300*2;
    p->xinches = xdim;
    p->yinches = ydim;
    p->xfull = p->xsquare = xdim * xinch;
    p->yfull = ydim * yinch;
    p->ch = chht = 1.1*chhtscl*fscl*default_ps/72.0*yinch + 0.5;
    p->cw = chwd = 0.5*chwdscl*fscl*default_ps/72.0*xinch + 0.5;
}

static struct {
    char *name;
    char *code;
} *f, fonts[] = {
    "Helvetica", "h",
    "Helvetica-Bold", "hb",
    "Helvetica-Oblique", "ho",
    "Helvetica-BoldOblique", "hbo",
    "Helvetica-BoldOblique", "hob", 	
    "Courier", "c",
    "Courier-Bold",	"cb",
    "Courier-Oblique", "co",
    "Courier-BoldOblique", "cbo",
    "Courier-BoldOblique", "cob",
    "Times-Roman", "t",
    "Times-Roman", "tr",
    "Times-Italic", "ti",
    "Times-Bold", "tb",
    "Times-BoldItalic", "tbi",
    "Times-BoldItalic", "tib",	"Symbol", "s",
    NULL, NULL };

static char *prev_lm="";

static void setfont(char *name, double ps)
{
    static char *prev_name;
    short n = 0;
    char *fontname = name;

    while (*fontname && n < 3) {
	fontcode[n++] = *(fontname++) | 040;
	while (*fontname && *(fontname++) != '-');
    }
    fontcode[n] = 0;
    for (f = fonts;  f->code != NULL && strcmp(fontcode, f->code) != 0; f++)
	;
    if (f->code == NULL) {
	err(NO, "Unknown font: %s -- Using: %s", name, DEFAULT_FONT);
	f->name = DEFAULT_FONT;
    }
    if (ps < 0 ) {
	err(NO, "Bad font size: %.5lg -- Using %g", ps, DEFAULT_PS);
	ps = DEFAULT_PS;
    }
    if (prev_ps != ps || prev_name != f->name) {
	printf("/%s %.5lg SF\n", prev_name=f->name, prev_ps=ps);
	if (*prev_lm != 0 && strcmp(prev_lm,"solid") != 0)
	    printf("%s setdash\n", prev_lm);
    }	
}

static void linewidth(double lw)
{
    static double prev_lw = -1;

    if (lw < 0) {
	err(NO, "Bad line width: %.5lg", lw);
	lw = 2;
    }
    if (prev_lw != lw)
	printf("%.5lg LW\n", prev_lw = lw);
}

static void linemod(char *lm)
{
    flushpath();
    if (strcmp(prev_lm,lm))
	printf("%s setdash\n", prev_lm=lm);
}

static int ored=-1, ogreen=-1, oblue=-1;

static void setgray(double grey)
{
    if (grey < 0 || grey > 1) {
	err(NO, "Bad grey value: %.5lg", grey);
	grey = 0;
    }
    else if (ored >= 0) {
	char greystring[8];

	sprintf(greystring, "grey%02d", (int)(grey*100.0));
	setcolor(greystring);
    }
    else 
	printf("%.5lg setgray\n", grey);
}

/* Added 23 February 2001 GBM */

static void setcolor(char *color)
{
    int i, red, green, blue;

    if (color == NULL)
	red = green = blue = 0;
    else if (*color == '#')
	sscanf(color, "#%2x%2x%2x", &red, &green, &blue);
    else {
	for (i = 0; i < NCOLORS; i++) {
	    if (strcasecmp(color, colortab[i].name) == 0) {
		red = colortab[i].red;
	        green = colortab[i].green;
		blue = colortab[i].blue;
		break;
	    }
	}
	if (i >= NCOLORS)
	    red = green = blue = 0;
    }
    if (red != ored || green != ogreen || blue != oblue) {
      ored = red; ogreen = green; oblue = blue;
      printf("%g %g %g setrgbcolor\n", red/255.0, green/255.0, blue/255.0);
    }
}

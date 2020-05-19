/* file: plt.h		Paul Albrecht		August 1984
			Last revised:	      10 December 2010
Constants, macros, data types, global variables, and function prototypes

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  12 April 2001: merged in axis.h, figs.h, optn.h, plot.h, and text.h; added
                 prototypes, general cleanup
  3 May 2001: added MAXLABELFILES (see option.c)
  7 May 2001: removed prototypes for functions in option.c that are now
		 local in scope (static)
  9 May 2001: added PLT_VERSION, set initially to 1.99
  21 October 2002: (2.1) moved many formerly global variables to *.c files
  25 April 2005: (2.3) changed Uint type from unsigned int to size_t (tested
		on x86 (32-bit) and x86_64 architectures)
  25 March 2009: (2.5) added HISTOGRAM plot mode
  10 December 2010: (2.5a) now includes <stdlib.h> and <string.h>
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
#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#endif
#include <math.h>

#ifndef PLT_VERSION
#define PLT_VERSION	"2.5a"
#endif

/* Constants */
#define FHUGE		1e38
#define	YES		1
#define NO		0
#define	DEFAULT		(Const)(-32767)
#define	FDEFAULT	(-32767.0)

#define	SETPTERM	(Const)1
#define	GRAPHELEMENT	(Const)2
#define	SETFONT		(Const)3
#define SETLINEWIDTH	(Const)4
#define SETGRAY		(Const)5
#define SETGRAYD	(Const)6
#define SETLINEMODE	(Const)7
#define SETCOLOR	(Const)8

#define EAXIS		'a'
#define EFIGURE		'f'
#define ELABEL		'l'
#define EPLOT		'p'
#define ETITLE		't'

#define SEP_GRAPH	(Const)01
#define SPECIAL_FNT	(Const)02
#define ROT_LABEL	(Const)04

#define DEFAULT_FONT	"Times-Roman"
#define DEFAULT_PS	16.0
#define DEFAULT_LW	3.0

#define PS_BLACK	0.0
#define PS_WHITE	1.0

#define NOTHING		(Const)0
#define ERASE		(Const)01
#define TITLES		(Const)02
#define LABELS		(Const)04
#define PLOTS		(Const)010
#define FIGURES		(Const)020
#define TICKMARKS	(Const)0100
#define TICKNUMS	(Const)0200
#define GRIDMARKS	(Const)0400
#define AXIS		(Const)01000

#define CDRIVEN		'c'
#define FILLED		'C'
#define FILLBETWEEN	'f'
#define IMPULSE		'i'
#define LABEL_N		't'
#define LINES		'l'
#define NCOLZERO	'm'
#define NORMAL		'n'
#define DARKNORMAL	'N'
#define OUTLINE		'o'
#define OUTLINEFILL	'O'
#define SCATTER		's'
#define SCATTER_STD	'e'
#define SYMBOL		'S'
#define SYMBOL_STD	'E'
#define HISTOGRAM	'h'

#define UP_STD			'+'
#define DOWN_STD		'-'
#define SYMMETRIC_STD	':'

#define CCONT		(Const)0
#define CMOVE		(Const)1
#define CDOT		(Const)2
#define CBOX		(Const)3

#define CBBCONT		(Const)7
#define COSTROKE	(Const)8	/* stroke path without adding (x,y) */
#define CSTROKE		(Const)9

#define CFILL		(Const)10
#define CBBFILL		(Const)11
#define CFILLI		(Const)12
#define CBBFILLI	(Const)14

#define CHANGEBEGIN	(Const)20
#define CHANGEFNT	(Const)20
#define CHANGEPS	(Const)21
#define CHANGELW	(Const)22
#define CHANGELM	(Const)23
#define CHANGEGRAY	(Const)24
#define CHANGECOLOR	(Const)25
#define CHANGEEND	25

#define CSYMBOL		(Const)30
#define CTEXT		(Const)100

#define NO_COL		(-1)

#define GCOLOR 		'C'
#define GELEMENT	'E'
#define GFONT		'F'
#define GGRAY		'G'
#define GLINEMODE	'L'
#define GPS		'P'
#define GSYMBOL		'S'
#define GLINEWIDTH	'W'

#define ARROW		'A'
#define BOX		'B'
#define CONNECT		'C'
#define DARKBOX		'D'

#define DATAC		1
#define WINC		2
#define PDEVC		3

#define MAXFLDS		7
#define MAXLINE		200

#define ALLINE		(Const)1
#define ALLINE2		(Const)2
#define ALLINE3		(Const)3
#define ALLINE4		(Const)4
#define ALLMASK		(Const)017

#define NOTARGV		(Const)020
#define SPECIAL		(Const)040
#define TYPEMASK	(~(NOTARGV|SPECIAL|ALLMASK))

#define COMMENT		(Const)0100
#define FIGURE		(Const)0200
#define HELPTHEM	(Const)0400
#define FIELDS		(Const)01000
#define SLABEL		(Const)02000

#define SPACE_DELIM	(Const)01
#define SKIP_CR		(Const)02
#define SEMI_TERM	(Const)04
#define DEL_SPACE	(Const)010
#define SAVE_TERM	(Const)020
#define VERBATIM	(Const)040

#define IN_OBJ		(DEL_SPACE | SPACE_DELIM | SEMI_TERM )

#define END_INPUT	(Const)01
#define END_LINE	(Const)02
#define HAVE_OBJ	(Const)04

#define MAXLABELFILES	6

/* Macros */
#ifdef DEBUG
#define ASSERT(CONDITION,TEXT)	\
	  if (!(CONDITION)) assert(TEXT, __FILE__, __LINE__);
#else
#define ASSERT(CONDITION,TEXT)		;
#endif

#define ENDP(ARRAY) (&ARRAY[sizeof(ARRAY)/sizeof(ARRAY[0])])
#define	FREE(PTR) { char *charPtr=(char *)(PTR); \
		    ASSERT(charPtr != 0, "Freeing null ptr");	\
		    free(charPtr); }
#ifdef MAIN
#define	COMMON
#define	SVAL(X)	= X
#else
#define	COMMON extern
#define	SVAL(X)
#endif

/* Data types */
typedef int Const;
typedef	size_t Uint;
typedef unsigned int Boolean;
typedef short Ptype;	/* Type expected by plotting routines */
typedef	enum { fullInit } Mode;

typedef struct pdev {
    char *pterm;	/* identifying name of plot device		*/
    short mode;		/* info on device capabilities			*/
    Ptype xfull;	/* full dimension in device units		*/
    Ptype xsquare;	/* x dimension for equal to yfull		*/
    Ptype yfull;	/* full dimension in device units		*/
    float xwmins;	/*  x0 for plot area in (0,0)->(1,1) units	*/
    float xwmaxs;	/*  y0 for plot area in (0,0)->(1,1) units	*/
    float ywmins;	/*  x1 for plot area in (0,0)->(1,1) units	*/
    float ywmaxs;	/*  y1 for plot area in (0,0)->(1,1) units	*/
    Ptype ch;		/* Char height in y device units		*/
    Ptype cw;		/* Char width in x device units			*/
    double xinches;	/* horizontal size of plot display		*/
    double yinches;	/* vertical size of plot display		*/
} *PTERM;

typedef	union {
    double *d;
    long *l;
    Boolean *b;
    char **s;
    char *c;
} PtrUnion;

typedef struct {
    char name;		/* 'x' or 'y'					*/
    double min;		/* lower limit					*/
    double max;		/* upper limit					*/
    double cr;		/* where this axis crosses the other axis	*/
    double aoff;	/* offset from other axis (alternative to cr)	*/
    double mlt;		/* if != DEFAULT make axis and ticks=0 mod mlt	*/
    double tick;	/* spacing between tic marks			*/
    double tmark;	/* a tick which must be marked and labeled	*/
    double tscl;	/* scale factor for tick mark length		*/
    long skp;		/* label one in every skp tick marks		*/
    char *pfm;		/* format (e.g. %4.2f) for outputing tick values*/
    char *lbl;		/* label for the axis				*/
    char *base;		/* base for log plots				*/
    short mode;		/* what to show					*/
    Boolean logflg;	/* YES for log axis				*/
    Boolean rev;	/* put reverse the axis ticks and labels	*/
    double scl;		/* scaling from user to device coordinates	*/
    double off;		/* offset from user to device coordinates	*/
    double acchi;	/* how much the xmax can be increased		*/
    double acclo;	/* how much the xmin can be decreased		*/
    char *numfg;	/* name of font group to use axis numbers	*/
    char *lblfg;	/* name of font group to use for axis label	*/
    char *extra;
    Ptype lo;		/* same as cr but in plot device coordinates	*/
    Ptype hi;		/* opposite end from lo				*/
    Ptype (*this)();	/* subr for this axis device coordinate		*/ 
    Ptype (*other)();	/* subr for the other axis device coordinate	*/
} AxisInfo, *AxisPtr;

typedef	struct {
    short c0;
    short c1;
    short c2;
    short c3;
    char *fgName;
    char pm;
    char subpm;
    char symbol;
    char pc;
    char *name;
} PltInfo, *PltPtr;

/*	Label justification is coded as two character string.  In general, 
	the first character (C, R or L) specifies the justification in the
	axis of the text, while the second specifies the justification
	(C, N, T, or B) in the perpendicular direction.			*/
typedef	struct {
    char *text;		/* label string; '\n' separates lines		*/
    char *fgName;	/* font group to use in printing label		*/
    char *just;		/* justification mode for the label		*/
    double xpos;	/* x position in window coordinates		*/
    double ypos;	/* y position in window coordinates		*/
    double angle;	/* angle at which to print the label		*/
    Const coord;	/* coordinate system for position		*/
} LblInfo, *LblPtr;

/* Information about labels that are to be used for plotting with
   the LABEL_N plot type is in struct PStrInfo.				*/

typedef	struct	{
    char *str;		/* string to read strings from			*/
    char **list;	/* argv[] type list of string pointers		*/
    char *just;		/* justification to use in printing strings	*/
    Boolean file;	/* file to read strings from			*/
    Uint n;		/* number of strings in list			*/
    Uint nmax;		/* max possible strings in list			*/
} PStrInfo, *PStrPtr;

typedef	struct	{
    char *name;
    char *help;
    short minflds;
    short maxflds;
    short mode;
    char *flds;
    char **fgnamep;
    char *ptrs[MAXFLDS];
} OptInfo, *OptPtr;

typedef	struct	{
    char *pterm;
    char *options;
} PSOInfo, *PSOPtr;

typedef	struct {
    char type;
    char coord;
    double x0;
    double y0;
    double x1;
    double y1;
    char *fgName;
} FigInfo, *FigPtr;

typedef struct {
    short pltNum;
    short lineNum;
    Ptype yPos;
    char *text;
} LegInfo, *LegPtr;

/* Anonymous structures */
COMMON struct {
    float *pts;
    double *row;
    Uint maxPts, nPts;
    Uint maxCols, nCols;
} Data;

COMMON struct {
    FigPtr figs;
    Uint maxFigs, nFigs;
    LblPtr lbls;
    Uint maxLbls, nLbls;
    LegPtr legs;
    Uint maxLegs, nLegs;
    double xlPos;
    double ylPos;
    double xlDel;
    double xlBoxScl;
    char *legfg;
    char *eStat;
} Figure;

COMMON struct {
    PltPtr plts;
    Uint maxPlts, nPlts;
    double xFrom;
    double xIncr;
    Boolean xDrive;
    Boolean quickPlot;
    Boolean exclude;
    long excluded;
    char *suppress;
    char *pModes;
    char defaultPMode;
} Plot;

/* Global variables */
COMMON AxisInfo
    xa,
    ya;
COMMON Boolean
    fixed_font SVAL(DEFAULT),
    optn,
    ticklogic;
COMMON char *df[7],
    *fgs,
    *pterm,
    *old_font,
    *old_lm SVAL(""),
    *programName,
    *axisfile,
    *gridfg,
    *gridtype;
COMMON double
    default_ps SVAL(DEFAULT_PS),
    chhtscl SVAL(1),
    chwdscl SVAL(1),
    fscl SVAL(FDEFAULT),
    xdim SVAL(FDEFAULT),
    ydim SVAL(FDEFAULT),
    xorig SVAL(FDEFAULT),
    yorig SVAL(FDEFAULT),
    oldGrayLevel SVAL(-1),
    old_ps SVAL(-1),
    old_lw SVAL(-1),
    theight,
    xfract,
    yfract,
    xmin,
    xmax,
    ymin,
    ymax,
    xwmins,
    xwmaxs,
    ywmins,
    ywmaxs;
COMMON LblInfo
    title;
COMMON PSOPtr
    psos;
COMMON PStrInfo
    pstr,
    fgstr;
COMMON PTERM
    p;
COMMON Ptype
    chht,
    chwd,
    xinch,
    yinch,
    xwmin,
    xwmax,
    ywmin,
    ywmax;
COMMON short
    omode SVAL(ERASE|TITLES|LABELS|PLOTS|FIGURES);

/* Function prototypes */

/* Functions defined in the driver modules (lw.c and xw.c).  If you
   add a driver for a new device, it must implement these functions. */
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

/* Functions defined in axis.c */
void SetupAxes(void);
void AxisInit(Mode mode);
void XAxisDraw(void);
void YAxisDraw(void);
void TickDef(AxisPtr a, double tick, char *lbl, double scl, Boolean override);

/* Functions defined in data.c */
void DataInit(char **argv);
void ReadPoints(void);
Boolean DataRead(Boolean initialize);
Boolean ReverseDataRead(Boolean initialize);

/* Functions defined in figure.c */
void FigureInit(Mode mode);
void FigureDef(char type, Const coord, double x0, double y0,
	       double x1, double y1, char *fgName);
void FigureDraw(FigPtr f);
void transform(Ptype *xp, Ptype *yp, double xdbl, double ydbl, double dflt,
	       Const coord);
void LegendDef(long lineNum, long pltNum, char *name);
void LegendDraw(void);

/* Functions defined in option.c */
void PTERMSpecificOpts(void);
void argvOpts(char **argv, int argc);

/* Functions defined in plot.c */
void PlotDef(char *pspec);
void PlotDraw(PltPtr plt);
void PlotInit(Mode mode);
void PlotNameDef(long n, char *name);
void SmallBox(Ptype x, Ptype y);
Ptype X(double x);
Ptype Y(double y);

/* Functions defined in pterm.c */
int PTERMLookup(char *ptermName, char *hardwired);

/* Functions defined in text.c */
void TextInit(int psdel);
void MakeGraphTitle(char **argv);
void TextDraw(LblPtr l);
void ReadStrings(PStrPtr ps);
char *JustMap(char *userTbc);
void FontGroupSelect(char *dflt_fgName, char *fgName);
void FontGroupDef(char *fgName, char *specs);
int getstr(char **cpp, char **strp);

/* Functions defined in tick.c */
void LinearTickLogic(AxisPtr a);

/* Functions defined in util.c */
void assert(char *text, char *file, int line);
void *azmem(void *ptr, size_t *current_elements, size_t new_elements,
	    size_t element_size);
double DoubleNum(char *str);
void err(int fatal, char *fmt, ...);
long LongNum(char *str);
void pinit(void);
void pquit(int signo);
char *StringSave(char *str);
void UtilInit(Mode mode);
void *zmem(size_t n, size_t size);

/* Functions defined in window.c */
void WindowDef(char *config, char *subsect);
void SuppressionDef(char *str);
void gsuppress(void);



/* file: option.c	Paul Albrecht		September 1987
			Last revised:	      14 November 2002 
Option-processing for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  15 April 2001: general cleanup
  3 May 2001: MAXLABELFILES now defined in plt.h
  7 May 2001: merged former optn.c, optn2.c, optn3.c
  9 May 2001: added version number, pointer to sources to optnHelp output
  21 October 2002: moved formerly global variables here from plt.h
  14 November 2002: fixed missing casts in azmem calls
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
#include <ctype.h>

#define ISOPT(STR) ((STR)[0] == '-' && isalpha((STR)[1]))
#define H(STRING) STRING
#define MATCH(CODE) (!match && (match = !strcmp(CODE,o->name)))
#define IF(STR,CMD) { if (!match && strcmp(STR,o->name) == 0) CMD; }
#define P(ITEM) (char *)(&ITEM)
#define	PP(ITEM) (char **)(&ITEM)
#define NOFG ((char **)0)
#define INFILE 01
#define STRING 02

/* Prototypes of functions defined in this module */
void PTERMSpecificOpts(void);
void argvOpts(char **argv, int argc);
static void SetPTERMSpecificOpt(char *specificPterm, char *options);
static void StringOptions(char *string);
static void FileOptions(char *name);
static void SetParams(OptPtr o, char **s, int nflds);
static void optnHelp(OptPtr otable, char **s, short nflds);
static void OneHelp(OptPtr o);
static void Options(void);
static void slabel(OptPtr o);
static void OneLiner(OptPtr o, char **s, int nflds);
static OptPtr lookup(char *opt);
static OptPtr lookup2(char **argv, short *np);
static void in_file(char *name, Boolean lock);
static void in_string(char *string, Boolean lock);
static int in_close(short nlev);
static int in_char(void);
static void in_unchar(short c);
static int in_parse(char **s, short maxflds, char *obj, short maxchrs);
static int in_obj(char *obj, short maxchrs, Const mode);

static Uint npso;

void PTERMSpecificOpts(void)
{
    int n;

    for (n=0; n < npso; n++)
	if (strcmp(psos[n].pterm,pterm) == 0)
	    StringOptions(psos[n].options);
}

void argvOpts(char **argv, int argc)
{
    OptPtr o;
    short k, n = 0, nflds;
    char *fld, *s[MAXFLDS], *getenv();

    while (n < argc) {
	while (n < argc && !ISOPT(argv[n]))
	    n++;
	if (n == argc)
	    break;
	o = lookup2(argv, &n);
	if(o->mode & NOTARGV)
	    err(YES, "Can't use `%s' on command line", o->name);
	nflds = 0;
	fgs = 0;
	while (n < argc && nflds < o->maxflds && !ISOPT(argv[n])) {
	    fld = argv[n];
	    if (o->fgnamep != 0 && nflds == 0 && (*fld == '(' || *fld == '['))
		*o->fgnamep = fld;
	    else
		s[nflds++] = fld;
	    argv[n++]  = NULL;
	}
	OneLiner(o, s, nflds);
    }
    for (k = n = 0; n < argc; n++)
	if (argv[n])
	    argv[k++] = argv[n];
    argv[k] = 0;
}

/* Functions defined below are available only within this file. */
static void SetPTERMSpecificOpt(char *specificPterm, char *options)
{
    PSOPtr pso;
    static Uint maxpsos;

    if (npso == maxpsos)
	psos = (PSOPtr)azmem(psos, (size_t *)&maxpsos, 10, sizeof(*psos));
    pso = &psos[npso++];
    pso->pterm = specificPterm;
    pso->options = options;
}

static void StringOptions(char *string)
{
    in_string(string, NO);
    Options();
}

static void FileOptions(char *name)
{
    in_file(name, NO);
    Options();
}


static void SetParams(OptPtr o, char **s, int nflds)
{
    PtrUnion ptr;
    short n = 0, nf = 0, np = 0, type, default_mode = NO;

    if (o->flds == 0)
	return;
    while (YES) {
	type = o->flds[nf++];
	ptr.c = o->ptrs[np];
	if (type == 0 && ptr.c == 0)
	    break;
	if (type == 0)
	    err(YES, "Missing format spec for `%s'", o->name);
	if (ptr.c == 0)
	    err(YES, "Null otable address for `%s'", o->name);
	switch (type) {
	case '-':
	    default_mode = NO;
	    break;
	case '+':
	    default_mode = YES;
	    break;
	case 'F':
	    if (n < nflds && strcmp(s[n],"-") != 0)
		*ptr.d = DoubleNum(s[n]);
	    else if (default_mode)
		*ptr.d = DEFAULT;
	    n++; np++;
	    break;
	case 'l':
	    if (n < nflds && strcmp(s[n],"-") != 0)
		*ptr.l = LongNum(s[n]);
	    else if (default_mode)
		*ptr.l = DEFAULT;
	    n++; np++;
	    break;
	case 's':
	case 'S':
	    if (n < nflds && strcmp(s[n],"-") != 0)
		*ptr.s = StringSave(s[n]);
	    else if (default_mode)
		*ptr.s = (type == 's') ? 0 : "";
	    n++; np++;
	    break;
	case 'y':
	    *ptr.b  = YES;
	    np++;
	    break;
	case 'n':
	    *ptr.b  = NO;
	    np++;
	    break;
	default:
	    err(YES, "Table err for `%s'", o->name);
	    break;
	}
    }
}

static void optnHelp(OptPtr otable, char **s, short nflds)
{
    OptPtr o;
    short len, matches, n;
    char *name;

    if (nflds  > 0)
	for (n = 0; n < nflds;  n++) {
	    name = s[n];
	    len = strlen(name);
	    matches = 0;
	    for (o = otable; *o->name; o++)
		if (o->help && strncmp(name,o->name,len) == 0) {
		    OneHelp(o);
		    matches++;
		}
	    if (matches == 0)
		err(NO, "No option begins with `%s'", name);
	}
    else {
	printf("plt version %s (compiled %s)\n", PLT_VERSION, __DATE__);
	printf("Use `-h xx' for help on all options beginning with `xx'\n\n");
	while (*otable->name) {
	    OneHelp(otable);
	    otable++;
	}
	printf("\nNote: a `-' argument leaves a field unchanged.\n\n");
	printf("The most recent version of plt is available freely from:\n");
	printf("     http://www.physionet.org/physiotools/plt/\n");
	printf("where a user's guide and examples can also be found.\n");
    }
    pquit(0);
}

static void OneHelp(OptPtr o)
{
    short n = 0, set = 0, reset = 0;
    char *fld;

    if (o->help == 0)
	return;
    printf("-%s\t", o->name);
    if (*o->help)
	printf("%s\n", o->help);
    else {
	for (fld = o->flds; fld && *fld; fld++) {
	    switch (*fld) {
	    case 'y': set++; break;
	    case 'n': reset++; break;
	    case 'F': printf("(float) "); n++; break;
	    case 'i': printf("(int) ");	n++; break;
	    case 's': printf("(string) "); n++; break;
	    }
	}
	if (set || reset) {
	    if (n > 0) printf("-- also ");
	    if (set) printf("sets flag");
	    if (reset) printf("%sresets flag", set ? ", " : "");
	}
	printf("\n");
    }
}

static double d0, d1, d2, d3;
static long l0, l1, l3;
static char *s0, *s1, *s2, *s3, *s4;

static	OptInfo	otable[] = {
    { "p",	H("specify (plotmodes) -- /ceEilmnsSt,0-9/"),
      1, 1,	SPECIAL|ALLINE, "+s", NOFG, P(s0) },
    { "s",	H("suppress (parts) of graph -- eflnmtpxyBCXY"),
      1, 1,	SPECIAL|ALLINE, "+s", NOFG, P(s0) },
    { "X",	H("x axis (min) (max) -- use xa instead"),
      1, 2,	FIELDS,	"FF", PP(xa.numfg), P(xa.min), P(xa.max) },
    { "Y",	H("y axis (min) (max) -- use ya instead"),
      1, 2,	FIELDS, "FF", PP(ya.numfg), P(ya.min), P(ya.max) },
    { "t",	H("graph (title)"),
      0, 1,	ALLINE,	"+S", PP(title.fgName), P(title.text) },
    { "T",	H("graph device is (PTERM)"),
      1, 1,	FIELDS,	"s", NOFG, P(pterm) },
    { "g",	H("specify tick mark grid (instructions)"),
      0, 1,	ALLINE,	"s", PP(gridfg), P(gridtype) },
    { "h",	H("help on (option) (option) ..."),
      0, 4,	SPECIAL|HELPTHEM, "+sssss", NOFG,
                 P(s0), P(s1), P(s2), P(s3), P(s4) },
    { "a",	H("arrow to data (x0) (y0) from (x1) (y1)"),
      0, 4,	SPECIAL|FIGURE, "+FFFF", PP(fgs), P(d0), P(d1), P(d2), P(d3) },
    { "A",	H("arrow to window (x0) (y0) from (x1) (y1)"),
      0, 4,	SPECIAL|FIGURE, "+FFFF", PP(fgs), P(d0), P(d1), P(d2), P(d3) },
    { "b",	H("box data at (x0) (y0) to (x1) (y1)"),
      0, 4,	SPECIAL|FIGURE, "+FFFF", PP(fgs), P(d0), P(d1), P(d2), P(d3) },
    { "B",	H("box at window (x0) (y0) to (x1) (y1)"),
      0, 4,	SPECIAL|FIGURE, "+FFFF", PP(fgs), P(d0), P(d1), P(d2), P(d3) },
    { "c",	H("connect from data (x0) (y0) to (x1) (y1)"),
      0, 4,	SPECIAL|FIGURE, "+FFFF", PP(fgs), P(d0), P(d1), P(d2), P(d3) },
    { "C",	H("connect from window (x0) (y0) to (x1) (y1)"),
      0, 4,	SPECIAL|FIGURE, "+FFFF", PP(fgs), P(d0), P(d1), P(d2), P(d3) },
    { "d",	H("dark box data at (x0) (y0) to (x1) (y1)"),
      0, 4,	SPECIAL|FIGURE, "+FFFF", PP(fgs), P(d0), P(d1), P(d2), P(d3) },
    { "D",	H("dark box at window (x0) (y0) to (x1) (y1)"),
      0, 4,	SPECIAL|FIGURE, "+FFFF", PP(fgs), P(d0), P(d1), P(d2), P(d3) },
    { "l",	H("label at data (x) (y) with (just) (text)"),
      3, 4,	SPECIAL|SLABEL|ALLINE4, "+FFss",
      		 PP(fgs), P(d0), P(d1), P(s2), P(s3) },
    { "L",	H("label at window (x) (y) with (just) (text)"),
      3, 5,	SPECIAL|SLABEL|ALLINE4, "+FFss",
		 PP(fgs), P(d0), P(d1), P(s2), P(s3) },
    { "w",	H("plot in window (configuration) (subwindow)"),
      1, 2,	SPECIAL, "+ss",	NOFG, P(s0), P(s1) },
    { "W",	H("plot in device window (x0) (y0) (x1) (y1)"),
      1, 4,	FIELDS, "FFFF",	NOFG,
      		 P(xwmins), P(ywmins), P(xwmaxs), P(ywmaxs) },
    { "f",	H("format (file)"),
      1, 1,	SPECIAL, "+s", NOFG, P(s0) },
    { "fa",	H("write axis information to format (file)"),
      1, 1,	FIELDS, "s", NOFG, P(axisfile) },
    { "F",	H("format (string)"),
      1, 1,	SPECIAL, "+s", NOFG, P(s0) },
    { "n",	0,
      2, 2,	SPECIAL|ALLINE2, "+ls", PP(fgs), P(l0), P(s1) },
    { "o",	H("suppress all output except plots"),
      0, 0,	SPECIAL, "" },
    { "cz",	H("generate column 0 using (xfrom) and (xincr)"),
      0, 2,	FIELDS,	"yFF", NOFG,
      		 P(Plot.xDrive), P(Plot.xFrom), P(Plot.xIncr) },
    { "ex", H("don't exclude points outside axis limits"),
      0, 0,	FIELDS, "n", NOFG, P(Plot.exclude) },
    { "hl",	H("horizontal label at window (x) (y) (just) (lines) (file)"),
      0, 5,	NOTARGV|SLABEL, "+FFsls",
      		 PP(fgs), P(d0), P(d1), P(s2), P(l3), P(s4) },
    { "vl",	H("vertical label at window (x) (y) (just) (lines) (file)"),
      0, 5,	NOTARGV|SLABEL, "+FFsls",
      		 PP(fgs), P(d0), P(d1), P(s2), P(l3), P(s4) },
    { "le",	H("legend (entry #) is (plot #) with (text)"),
      1, 3,	SPECIAL|ALLINE3, "+llS", NOFG, P(l0), P(l1), P(s2) },
    { "lp",   H("put legend at window (x) (y) with (boxScl) (segLen) (erase)"),
      1, 5,	FIELDS,	"FFFFS",
      		 PP(Figure.legfg), P(Figure.xlPos), P(Figure.ylPos),
      		 P(Figure.xlBoxScl), P(Figure.xlDel), P(Figure.eStat) },
    { "lx",	H("log x axis with (base) and (subticks?)"),
      0, 2,	FIELDS,	"yss",
      		 PP(xa.numfg), P(xa.logflg), P(xa.base), P(xa.extra) },
    { "ly",	H("log y axis with (base) and (subticks?)"),
      0, 2,	FIELDS,	"yss",
		 PP(ya.numfg), P(ya.logflg), P(ya.base), P(ya.extra) },
    { "tf",	H("use plot text from (file) with (just)"),
      1, 2,	FIELDS, "yss", NOFG, P(pstr.file), P(pstr.str), P(pstr.just) },
    { "ts",	H("use plot text from (string) with (just)"),
      1, 2,	ALLINE,	"ss", NOFG, P(pstr.str), P(pstr.just) },
    { "fs", H("take font group strings from (string)"),
      1, 1,	ALLINE,	"s", NOFG, P(fgstr.str)},
    { "x",	H("x axis (title)"),
      0, 1,	ALLINE,	"+S", PP(xa.lblfg), P(xa.lbl), },
    { "xa",	H("x axis (min) (max) (tick) (fmt) (tskip) (cross)"),
      1, 6,	FIELDS,	"FFFslF",
		 PP(xa.numfg), P(xa.min), P(xa.max), P(xa.tick),
      		 P(xa.pfm), P(xa.skp), P(xa.cr) },
    { "xe", H("allowed error in (xmin) and (xmax)"),
      1, 2,	FIELDS, "FF", NOFG, P(xa.acclo), P(xa.acchi) },
    { "xm", H("make x axis ticks a multiple of (num)" ),
      1, 1,	FIELDS, "F", PP(xa.numfg), P(xa.mlt) },
    { "xo", H("move x axis down by (xwfract)"),
      1, 1,	FIELDS, "F", NOFG, P(xa.aoff) },
    { "xr", H("reverse x axis ticks and labels"),
      0, 0,	FIELDS, "y", NOFG, P(xa.rev) },
    { "xt", H("add x axis tick at (pos) with (label)"),
      1, 3,	SPECIAL,"+FsF", PP(fgs), P(d0), P(s1), P(d2) },
    { "xts", H("specify x tick (anchor) (scl)"),
      1, 2,	FIELDS,"FF", NOFG, P(xa.tmark), P(xa.tscl) },
    { "y",	H("y axis (title)"),
      0, 1,	ALLINE,	"+S", PP(ya.lblfg), P(ya.lbl), },
    { "ya",	H("y axis (min) (max) (tick) (fmt) (tskip) (cross)"),
      1, 6,	FIELDS, "FFFslF",
		 PP(ya.numfg), P(ya.min), P(ya.max), P(ya.tick),
      		 P(ya.pfm), P(ya.skp), P(ya.cr) },
    { "ye", H("allowed error in (ymin) and (ymax)"),
      1, 2,	FIELDS, "FF", NOFG, P(ya.acclo), P(ya.acchi) },
    { "ym", H("make y axis ticks a multiple of (num)" ),
      1, 1,	FIELDS, "F", PP(ya.numfg), P(ya.mlt) },
    { "yo", H("move y axis over by (ywfract)"),
      1, 1,	FIELDS, "F", NOFG, P(ya.aoff) },
    { "yr", H("reverse y axis ticks and labels"),
      0, 0,	FIELDS, "y", NOFG, P(ya.rev) },
    { "yt",	H("add y axis tick at (pos) with (label)"),
      1, 3,	SPECIAL,"+FsF", PP(fgs), P(d0), P(s1), P(d2) },
    { "yts", H("specify y tick (anchor) (scl)"),
      1, 2,	FIELDS, "FF", NOFG, P(ya.tmark), P(ya.tscl) },
    { "dev", H("if PTERM is (pterm) process (options)"),
      2, 2,	SPECIAL|ALLINE2, "+ss",	 NOFG, P(s0), P(s1) },
    { "tick", 0,
      0, 1,	FIELDS,	"y", NOFG, P(ticklogic) },
    { "sf",	H("set (fgroup) to (specs) using FPCGWLE"),
      2, 2,	SPECIAL|ALLINE2, "+ss", NOFG, P(s0), P(s1) },
    { "ch", H("scale factor for char (height) (width)"),
      1, 2,	FIELDS, "FF", NOFG, P(chhtscl), P(chwdscl) },
    { "size", H("specify page (fscale) (x1-x0) (y1-y0) (x0) (y0)"),
      1, 5,	FIELDS, "FFFFF", NOFG,
		 P(fscl), P(xdim), P(ydim), P(xorig), P(yorig) },
    { "data", 0,
      1, 6,	FIELDS, "+ssssss", NOFG,
		 P(df[0]), P(df[1]), P(df[2]), P(df[3]), P(df[4]), P(df[5]) }, 
    { "optn", 0,
      0, 1,	FIELDS,	"y", NOFG, P(optn) },
    { "",	0,	/* Options beyond here can't give "help" */
      0, 0,	COMMENT },
};

/* Read options from format string or file */
static void Options(void)
{
    OptPtr o;
    short c, n, nflds, status;
    char obj[MAXLINE], *s[MAXFLDS];

    while (!(in_obj(obj,sizeof(obj),IN_OBJ|SAVE_TERM)==(END_LINE+END_INPUT))) {
	if (optn)
	    err(NO, "OPTION: `%s'\n", obj);
	if (obj[0] == '#') {
	    do {
		c = in_char();
	    } while (c != '\n' && c != EOF);
	    continue;
	}
	else
	    o = lookup(obj);
	do {
	    c = in_char();
	} while (c == ' ' || c == '\t');
	fgs = NULL;
	if (o->fgnamep && c == '(' || c == '[') {
	    n = 0;		/* get inline font group spec */
	    do {
		c = in_char();
		obj[n++] = c;
		if (c == '\n') {
		    in_unchar((short)c);
		    break;
		}
	    } while (c != EOF && c != ']' && c != ')');
	    if (c != ']' && c != ')')
		err(YES, "Unterminated fgroup spec for `%s'", o->name);
	    obj[n-1] = 0;
	    *o->fgnamep = StringSave(obj);
	    do {
		c = in_char();
	    } while (c == ' ' || c == '\t');
	}
	in_unchar((short)c);
	n = (o->mode & ALLMASK);
	if (n > 0) {
	    nflds = status = 0;
	    while (nflds < n-1 && !(status&END_LINE)) {
		status = in_obj(obj, sizeof(obj), IN_OBJ|SAVE_TERM);
		if (status & HAVE_OBJ)
		    s[nflds++] = StringSave(obj);
	    }
	    if (in_obj(obj,sizeof(obj),SEMI_TERM) & HAVE_OBJ)
		s[nflds++] = StringSave(obj);
	}
	else
	    nflds = in_parse(s, MAXFLDS, obj, sizeof(obj));
	if ((o->mode & TYPEMASK) == SLABEL) {
	    SetParams(o, s, nflds);
	    slabel(o);
	}
	else
	    OneLiner(o, s, nflds);
    }
}

/* process the hl, vl, l, and L options */
static void slabel(OptPtr o)
{
    LblPtr l;
    Uint len, maxlen;
    short n, nc, nlines, status;
    char text[MAXLINE];

    if (Figure.nLbls == Figure.maxLbls)
	Figure.lbls = (LblPtr)azmem(Figure.lbls, (size_t *)&Figure.maxLbls, 5,
				    sizeof(*Figure.lbls));
    l = &Figure.lbls[Figure.nLbls++];
    if (strcmp(o->name,"l") == 0 || strcmp(o->name,"L") == 0) {
	l->xpos = d0;
	l->ypos = d1;
	l->just = (s2 == NULL) ? "CC" : JustMap(s2);
	l->text = (s3 == NULL) ? "" : s3;
	l->fgName = fgs;
	l->coord = (strcmp(o->name,"l") == 0) ? DATAC : WINC;
	return;
    }
    if (s4 != NULL) {
	in_file(s4, YES);
	nlines = (l3 == DEFAULT) ? 1000 : l3;
    }
    else
	nlines = (l3 == DEFAULT) ? 1 : l3;
    l->text = 0;
    l->xpos = d0;
    l->ypos = d1;
    l->angle = (o->name[0] == 'h') ? 0 : 90;
    l->fgName = fgs;
    l->coord = WINC;
    if (s2 == 0)
	l->just = nlines > 1 ? "LB": "CB";
    else
	l->just = JustMap(s2);
    maxlen = nc = 0;
    for (n = 0; n < nlines; n++) {
	status = in_obj(text, sizeof(text), SEMI_TERM);
	if (status == (END_LINE+END_INPUT) || strcmp(text,"<END>") == 0)
	    break;
	len = strlen(text);
	l->text = azmem(l->text, (size_t *)&maxlen, len+1, sizeof(char));
	if (n > 0)
	    l->text[nc++] = '\n';
	strcpy(&l->text[nc], text);
	nc += len;
    }
    if (l->text == NULL)
	l->text = "";
    else
	l->text[nc++] = 0;
    if (s4 != NULL)
	in_close((short)1);
}

/* process simple, (i.e. one line) options */
static void OneLiner(OptPtr o, char **s, int nflds)
{
    short n;
    char match = NO;

    if (nflds < o->minflds)
	err(YES, "Too few fields for `%s'", o->name);
    SetParams(o, s, nflds);
    if (o->mode & SPECIAL) {
	switch (o->mode & TYPEMASK) {
	case 0:
	    IF ("dev", SetPTERMSpecificOpt(s0, s1));
	    IF ("f", FileOptions(s0));
	    IF ("F", StringOptions(s0));
	    IF ("le", LegendDef(l0, l1, s2));
	    IF ("n", PlotNameDef(l0,s1));
	    IF ("o", SuppressionDef("xyte"));
	    IF ("p", Plot.pModes=s0);
	    IF ("s", SuppressionDef(s0));
	    IF ("sf", FontGroupDef(s0,s1));
	    IF ("xt", TickDef(&xa, d0, s1, d2, YES));
	    IF ("yt", TickDef(&ya, d0, s1, d2, YES));
	    IF ("w", WindowDef(s0, s1));
	    break;
	case SLABEL:
	    slabel(o);
	case COMMENT:
	    break;
	case FIGURE:
	    n = (*o->name & 040) ? DATAC : WINC;
	    FigureDef((*o->name& ~040), n, d0, d1, d2, d3, fgs);
	    break;
	case HELPTHEM:
	    optnHelp(otable, s, nflds);
	    break;
	default:
	    err(YES, "Missing special case `%s'", o->name);
	}
    }
}

static OptPtr lookup(char *opt)
{
    OptPtr o;

    for (o = ENDP(otable)-1; o >= otable; o--) {
	if (strcmp(o->name,opt) == 0)
	    return(o);
    }
    err(YES, "Illegal option `%s'", opt);
    return(NO);	/* for lint */
}

static OptPtr lookup2(char **argv, short *np)
{
    OptPtr o;
    char *opt = ++argv[*np];

    for (o = ENDP(otable)-1;  o >= otable;  o--) {
	if (o->name[1] == 0 && *opt == o->name[0]) {
	    if (*(++argv[*np]) == 0)
		argv[(*np)++] = 0;
	    return(o);
	}
	else {
	    if (strcmp(o->name,opt) == 0) {
		argv[(*np)++] = 0;
		return(o);
	    }
	}
    }
    err(YES, "Illegal option: `%s'", opt);
    return (NO);	/* for lint */
}

static struct {
    int	type;
    FILE *fp;
    char *sp;
    char *name;
    short line;
    Boolean lock;
} input[MAXLABELFILES], *in;
static short nilev = 0;

static void in_file(char *name, Boolean lock)
{
    if (nilev == MAXLABELFILES)
	err(YES, "Too many nested input streams");
    in = &input[nilev];
    in->fp = fopen(name, "r");
    if (in->fp == NULL)
	err(YES, "Can't read file `%s'", name);
    in->type = INFILE;
    in->name = name;
    in->line = 0;
    in->lock = lock;
    nilev++;
}

static void in_string(char *string, Boolean lock)
{
    if (nilev == MAXLABELFILES)
	err(YES, "Too many nested input streams");
    in = &input[nilev];
    in->type = STRING;
    in->sp   = string;
    in->name = "format string";
    in->line = 0;
    in->lock = lock;
    nilev++;
}

static int in_close(short nlev)
{
    if (nlev < -1 || nlev > nilev)
	nlev = nilev;
    while (nlev-- > 0) {
	nilev--;
	if (input[nilev].type == INFILE)
	    fclose(input[nilev].fp);
    }
    in = &input[nilev-1];
    return ((int)nilev);
}

static int in_char(void)
{
    short c;

    if (nilev == 0)
	return (EOF);
    switch (in->type) {
    case INFILE: c = getc(in->fp); break;
    case STRING: c = *in->sp;
	if (c == 0)
	    c = EOF;
	else
	    in->sp++;
	break;
    }
    if (c == EOF && !in->lock) {
	in_close((short)1);
	return (in_char());
    }
    else
	return (c);
}

static void in_unchar(short c)
{
    if (c != EOF) {
	switch (in->type) {
	case INFILE: ungetc(c, in->fp);
	    break;
	case STRING: in->sp--;
	    break;
	}
    }
}

static int in_parse(char **s, short maxflds, char *obj, short maxchrs)
{
    short status = 0, nflds = 0, nchrs, n;

    while (nflds < maxflds && !(status & END_LINE)) {
	status = in_obj(obj, maxchrs, IN_OBJ);
	if (status & HAVE_OBJ) {
	    if (nflds == maxflds)
		err(YES, "Too many fields");
	    s[nflds++] = StringSave(obj);
	    nchrs = strlen(obj);
	    obj += (nchrs + 1);
	    maxchrs -= (nchrs + 1);
	}
    }
    if (optn) {
	err(NO, "PARSE:");
	for (n = 0; n < nflds; n++)
	    err(NO, "  (%s)", s[n]);
	err(NO, "\n");	
    }
    return ((int)nflds);
}

static int in_obj(char *obj, short maxchrs, Const mode)
{
    short nchrs = 0, c, shield = NO, quote = 0, first = YES, nbrack = 0,
	status = 0, more = YES, outchar;

    if (mode & DEL_SPACE) {
	do {
	    c = in_char();
	    switch (c) {
	    case '\t':
	    case ' ':	break;
	    case '\n':	if (!(mode & SKIP_CR))
		more = NO;
	    break;
	    default:	more = NO;
		break;
	    }
	} while (more && c != EOF);
    }
    else
	c = in_char();
    do {
	outchar = NO;
	switch ((c!= EOF && shield) ? 0 : c) {
	case 0:
	    if (quote != 0) {
		if (quote == c) {
		    shield = NO;
		    quote = 0;
		}
		else
		    outchar = YES;
	    }
	    else {
		shield = NO;
		outchar = YES;
	    }
	    break;
	case '\\':
	    if (!(mode & VERBATIM))
		shield = YES;
	    break;
	case '"':
	case '\'':
	    if (!(mode & VERBATIM) && quote == 0) {
		shield = YES;
		quote = c;
	    }
	    break;
	case '{':
	    nbrack++;
	    break;
	case '}':
	    if (nbrack == 0)
		err(NO, "Stray '}' -- line %d of %s\n", in->line, in->name);
	    if (--nbrack == 0)
		status |= END_INPUT;
	    break;
	case ';':
	    if (mode & SEMI_TERM) {
		if (mode & SAVE_TERM)
		    in_unchar(c);
		status |= END_LINE;
	    }
	    else
		outchar = YES;
	    break;
	case '\n':
	    if (!(mode & SKIP_CR)) {
		if (mode & SAVE_TERM)
		    in_unchar(c);
		status |= END_LINE;
	    }
	    break;
	case '\t':
	case ' ':
	    if (mode & SPACE_DELIM)
		status |= HAVE_OBJ;
	    else
		outchar = YES;
	    break;
	case EOF:
	    status |= (END_LINE|END_INPUT);
	    break;
	default:
	    outchar = YES;
	    break;
	}
	if (outchar) {
	    if (maxchrs > 1) {
		obj[nchrs++] = c;
		maxchrs--;
	    }
	    else if (first) {
		err(NO, "Line `%.25s...' too long", obj);
		first = NO;
	    }
	}
	if (status == 0)
	    c = in_char();
    } while (status == 0);
    if (nbrack > 0)
	err(NO, "Missing '}' for `%.10s...'\n", obj);
    if (nchrs > 0)
	status |= HAVE_OBJ;
    obj[nchrs] = 0;
    if (optn)
	err(NO, "OBJECT: (%s)\n", obj);
    if (quote)
	err(NO, "Unmatched quote in format specification");
    return ((int)status);
}

/*	plt/optn2.c		Paul Albrecht		Sept 1987

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4
*/

#include	"plt.h"
#include	"optn.h"
#include	"plot.h"
#include	"axis.h"
#include	"figs.h"
#include	"text.h"

#define		H(STRING)	STRING

#define		MATCH(CODE)	( !match && (match = !strcmp(CODE,o->name)) )

#define		IF(STR,CMD)	{ if( !match && strcmp(STR,o->name) == 0 ) CMD; }

#define		P(ITEM)		(char *)(&ITEM)
#define		PP(ITEM)	(char **)(&ITEM)
#define		NOFG		((char **)0)

static	double	d0, d1, d2, d3;

static	long	l0, l1, l3;

static	char	*s0, *s1, *s2, *s3, *s4;


static	OptInfo	otable[] = {
{ "p",	H("specify (plotmodes) -- /ceEilmnsSt,0-9/"),
1, 1,	SPECIAL|ALLINE, "+s", NOFG, P(s0) },
{ "s",	H("supress (parts) of graph -- eflnmtpxyBCXY"),
1, 1,	SPECIAL|ALLINE, "+s", NOFG, P(s0) },
{ "X",	H("x axis (min) (max) -- use xa instead"),
1, 2,	FIELDS,	"FF", PP(xa.numfg), P(xa.min), P(xa.max) },
{ "Y",	H("y axis (min) (max) -- use ya instead"),
1, 2,	FIELDS, "FF", PP(ya.numfg), P(ya.min), P(ya.max) },
{ "t",	H("graph (title)"),
0, 1,	ALLINE,	"+S", PP(title.fgName), P(title.text) },
{ "T",	H("graph device is (PTERM)"),
1, 1,	FIELDS,	"s", NOFG, P(pterm) },
{ "g",	H("specify tic mark grid (instructions)"),
0, 1,	ALLINE,	"s", PP(gridfg), P(gridtype) },
{ "h",	H("help on (option) (option) ..."),
0, 4,	SPECIAL|HELPTHEM, "+sssss", NOFG, P(s0), P(s1), P(s2), P(s3), P(s4) },
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
3, 4,	SPECIAL|SLABEL|ALLINE4, "+FFss", PP(fgs), P(d0), P(d1), P(s2), P(s3) },
{ "L",	H("label at window (x) (y) with (just) (text)"),
3, 5,	SPECIAL|SLABEL|ALLINE4, "+FFss", PP(fgs), P(d0), P(d1), P(s2), P(s3) },
{ "w",	H("plot in window (configuration) (subwindow)"),
1, 2,	SPECIAL, "+ss",	NOFG, P(s0), P(s1) },
{ "W",	H("plot in device window (x0) (y0) (x1) (y1)"),
1, 4,	FIELDS, "FFFF",	NOFG, P(xwmins), P(ywmins), P(xwmaxs), P(ywmaxs) },
{ "f",	H("format (file)"),
1, 1,	SPECIAL, "+s", NOFG, P(s0) },
{ "fa",	H("write axis information to format (file)"),
1, 1,	FIELDS, "s", NOFG, P(axisfile) },
{ "F",	H("format (string)"),
1, 1,	SPECIAL, "+s", NOFG, P(s0) },
{ "n",	0,
2, 2,	SPECIAL|ALLINE2, "+ls", PP(fgs), P(l0), P(s1) },
{ "o",	H("supress all output except plots"),
0, 0,	SPECIAL, "" },
{ "cz",	H("generate column 0 using (xfrom) and (xincr)"),
0, 2,	FIELDS,	"yFF", NOFG, P(Plot.xDrive), P(Plot.xFrom), P(Plot.xIncr) },
{ "ex", H("don't exclude points outside axis limits"),
0, 0,	FIELDS, "n", NOFG, P(Plot.exclude) },
{ "hl",	H("horizontal label at window (x) (y) (just) (lines) (file)"),
0, 5,	NOTARGV|SLABEL, "+FFsls", PP(fgs), P(d0), P(d1), P(s2), P(l3), P(s4) },
{ "vl",	H("vertical label at window (x) (y) (just) (lines) (file)"),
0, 5,	NOTARGV|SLABEL, "+FFsls", PP(fgs), P(d0), P(d1), P(s2), P(l3), P(s4) },
{ "le",	H("legend (entry #) is (plot #) with (text)"),
1, 3,	SPECIAL|ALLINE3, "+llS", NOFG, P(l0), P(l1), P(s2) },
{ "lp",	H("put legend at window (x) (y) with (boxScl) (segLen) (erase)"),
1, 5,	FIELDS,	"FFFFS",	PP(Figure.legfg), P(Figure.xlPos), P(Figure.ylPos),
			 P(Figure.xlBoxScl), P(Figure.xlDel), P(Figure.eStat) },
{ "lx",	H("log x axis with (base) and (subtics?)"),
0, 2,	FIELDS,	"yss", PP(xa.numfg), P(xa.logflg), P(xa.base), P(xa.extra) },
{ "ly",	H("log y axis with (base) and (subtics?)"),
0, 2,	FIELDS,	"yss", PP(ya.numfg), P(ya.logflg), P(ya.base), P(ya.extra) },
{ "tf",	H("use plot text from (file) with (just)"),
1, 2,	FIELDS, "yss", NOFG, P(pstr.file), P(pstr.str), P(pstr.just) },
{ "ts",	H("use plot text from (string) with (just)"),
1, 2,	ALLINE,	"ss", NOFG, P(pstr.str), P(pstr.just) },
{ "fs", H("take font group strings from (string)"),
1, 1,	ALLINE,	"s", NOFG, P(fgstr.str)},
{ "x",	H("x axis (title)"),
0, 1,	ALLINE,	"+S", PP(xa.lblfg), P(xa.lbl), },
{ "xa",	H("x axis (min) (max) (tic) (fmt) (tskip) (cross)"),
1, 6,	FIELDS,	"FFFslF", PP(xa.numfg), P(xa.min), P(xa.max), P(xa.tic),
			  P(xa.pfm), P(xa.skp), P(xa.cr) },
{ "xe", H("allowed error in (xmin) and (xmax)"),
1, 2,	FIELDS, "FF", NOFG, P(xa.acclo), P(xa.acchi) },
{ "xm", H("make x axis tics a multiple of (num)" ),
1, 1,	FIELDS, "F", PP(xa.numfg), P(xa.mlt) },
{ "xo", H("move x axis down by (xwfract)"),
1, 1,	FIELDS, "F", NOFG, P(xa.aoff) },
{ "xr", H("reverse x axis tics and labels"),
0, 0,	FIELDS, "y", NOFG, P(xa.rev) },
{ "xt", H("add x axis tic at (pos) with (label)"),
1, 3,	SPECIAL,"+FsF", PP(fgs), P(d0), P(s1), P(d2) },
{ "xts", H("specify x tic (anchor) (scl)"),
1, 2,	FIELDS,"FF", NOFG, P(xa.tmark), P(xa.tscl) },
{ "y",	H("y axis (title)"),
0, 1,	ALLINE,	"+S", PP(ya.lblfg), P(ya.lbl), },
{ "ya",	H("y axis (min) (max) (tic) (fmt) (tskip) (cross)"),
1, 6,	FIELDS, "FFFslF", PP(ya.numfg), P(ya.min), P(ya.max), P(ya.tic),
			  P(ya.pfm), P(ya.skp), P(ya.cr) },
{ "ye", H("allowed error in (ymin) and (ymax)"),
1, 2,	FIELDS, "FF", NOFG, P(ya.acclo), P(ya.acchi) },
{ "ym", H("make y axis tics a multiple of (num)" ),
1, 1,	FIELDS, "F", PP(ya.numfg), P(ya.mlt) },
{ "yo", H("move y axis over by (ywfract)"),
1, 1,	FIELDS, "F", NOFG, P(ya.aoff) },
{ "yr", H("reverse y axis tics and labels"),
0, 0,	FIELDS, "y", NOFG, P(ya.rev) },
{ "yt",	H("add y axis tic at (pos) with (label)"),
1, 3,	SPECIAL,"+FsF", PP(fgs), P(d0), P(s1), P(d2) },
{ "yts", H("specify y tic (anchor) (scl)"),
1, 2,	FIELDS, "FF", NOFG, P(ya.tmark), P(ya.tscl) },
{ "dev", H("if PTERM is (pterm) process (options)"),
2, 2,	SPECIAL|ALLINE2, "+ss",	 NOFG, P(s0), P(s1) },
{ "tic", 0,
0, 1,	FIELDS,	"y", NOFG, P(ticlogic) },
{ "sf",	H("set (fgroup) to (specs) using FPCGWLE"),
2, 2,	SPECIAL|ALLINE2, "+ss", NOFG, P(s0), P(s1) },
{ "ch", H("scale factor for char (height) (width)"),
1, 2,	FIELDS, "FF", NOFG, P(chhtscl), P(chwdscl) },
{ "size", H("specify page (fscale) (x1-x0) (y1-y0) (x0) (y0)"),
1, 5,	FIELDS, "FFFFF", NOFG, P(fscl), P(xdim), P(ydim), P(xorig), P(yorig) },
{ "data", 0,
1, 6,	FIELDS, "+ssssss", NOFG, P(df[0]), P(df[1]), P(df[2]), P(df[3]), P(df[4]), P(df[5]) }, 
{ "optn", 0,
0, 1,	FIELDS,	"y", NOFG, P(optn) },
{ "",	0,	/* Options beyondn here can't give "help" */
0, 0,	COMMENT },
};



Options()	/* Read options from format string or file */
{
OptPtr	o;
short	c, n, nflds, status;
char	obj[MAXLINE], *s[MAXFLDS];

	while( !(in_obj(obj,sizeof(obj),IN_OBJ|SAVE_TERM) == (END_LINE+END_INPUT)) ) {
		if( optn )
			eprintf( "OPTION: `%s'\n", obj );

		if( obj[0] == '#' ) {
			do {	c = in_char();
			} while( c != '\n' && c != EOF );
			continue;
		}
		else
			o = lookup( obj );

		do { c = in_char();
		} while( c == ' ' || c == '\t' );

		fgs = NULLS;
		if( o->fgnamep && c == '(' || c == '[' ) {
			n = 0;		/* get inline font group spec */
			do {
				c = in_char();
				obj[n++] = c;
				if( c == '\n' ) {
					in_unchar( (short)c );
					break;
				}
			} while( c != EOF && c != ']' && c != ')' );

			if( c != ']' && c != ')' )
				err( YES, "Unterminated fgroup spec for `%s'", o->name );
			obj[n-1] = 0;
			*o->fgnamep = StringSave( obj );

			do { c = in_char();
			} while( c == ' ' || c == '\t' );
		}

		in_unchar( (short)c );

		n = (o->mode & ALLMASK);
		if( n > 0 ) {
			nflds = status = 0;
			while( nflds < n-1 && !(status&END_LINE) ) {
				status = in_obj( obj, sizeof(obj), IN_OBJ|SAVE_TERM );
				if( status & HAVE_OBJ )
					s[nflds++] = StringSave( obj );
			}
			if( in_obj(obj,sizeof(obj),SEMI_TERM) & HAVE_OBJ )
				s[nflds++] = StringSave( obj );
		}
		else
			nflds = in_parse( s, MAXFLDS, obj, sizeof(obj) );

		if( (o->mode & TYPEMASK) == SLABEL ) {
			SetParams( o, s, nflds );
			slabel( o );
		}
		else
			OneLiner( o, s, nflds );
	}
}


slabel( o )	/* process the hl, vl, l, and L options */
OptPtr	o;
{
LblPtr	l;
Uint	len, maxlen;
short	n, nc, nlines, status;
char	text[MAXLINE];

	if( nlbls == maxlbls )
		lbls = (LblPtr)azmem(lbls,&maxlbls,5,sizeof(*lbls));
	l = &lbls[nlbls++];

	if( strcmp(o->name,"l") == 0 || strcmp(o->name,"L") == 0 ) {
		l->xpos = d0;
		l->ypos = d1;
		l->just = (s2 == NULLS) ? "CC" : JustMap(s2);
		l->text = (s3 == NULLS) ? "" : s3;
		l->fgName = fgs;
		l->coord = (strcmp(o->name,"l") == 0) ? DATAC : WINC;
		return;
	}

	if( s4 != NULLS ) {
		in_file( s4, YES );
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
	if( s2 == 0 )
		l->just = nlines > 1 ? "LB": "CB";
	else
		l->just = JustMap( s2 );

	maxlen = nc = 0;
	for( n=0;  n < nlines;  n++ ) {
		status = in_obj( text, sizeof(text), SEMI_TERM );
		if( status == (END_LINE+END_INPUT) || strcmp(text,"<END>") == 0 )
			break;

		len = strlen(text);
		l->text = azmem( l->text, &maxlen, len+1, sizeof(char) );
		if( n > 0 )
			l->text[nc++] = '\n';
		strcpy( &l->text[nc], text );
		nc += len;
	}
	if( l->text == NULLS )
		l->text = "";
	else
		l->text[nc++] = 0;

	if( s4 != NULLS )
		in_close( (short)1 );
}


OneLiner( o, s, nflds)	/* process simple, (i.e. one line) options */
register OptPtr	o;
char	*s[];
{
short	n;
char	match;

	if( nflds < o->minflds )
		err( YES, "Too few fields for `%s'", o->name );

	match = NO;

	SetParams( o, s, nflds );

	if( o->mode & SPECIAL ) {
		switch( o->mode & TYPEMASK ) {
			case 0:
				IF( "dev", SetPTERMSpecificOpt(s0, s1) );
				IF( "f", FileOptions(s0) );
				IF( "F", StringOptions(s0) );
				IF( "le", LegendDef(l0, l1, s2) );
				IF( "n", PlotNameDef(l0,s1) );
				IF( "o", SupressionDef("xyte") );
				IF( "p", Plot.pModes=s0 );
				IF( "s", SupressionDef(s0) );
				IF( "sf", FontGroupDef(s0,s1) );
				IF( "xt", TicDef(&xa, d0, s1, d2, YES) );
				IF( "yt", TicDef(&ya, d0, s1, d2, YES) );
				IF( "w", WindowDef(s0, s1) );
				break;
			case SLABEL:
				slabel( o );
			case COMMENT:
				break;
			case FIGURE:
				n = (*o->name & 040) ? DATAC : WINC;
				FigureDef((*o->name& ~040), n, d0, d1, d2, d3, fgs );
				break;
			case HELPTHEM:
				optnHelp( otable, s, nflds );
				break;
			default:
				err( YES, "Missing special case `%s'", o->name );
		}
	}
}


OptPtr	lookup( opt )
char	*opt;
{
register OptPtr	o;

	for( o=ENDP(otable)-1;  o >= otable;  o-- ) {
		if( strcmp(o->name,opt) == 0 )
			return( o );
	}
	err( YES, "Illegal option `%s'", opt );
	return( NO );	/* for lint */
}


OptPtr	lookup2( argv, np )
char	*argv[];
short	*np;
{
register OptPtr	o;
register char	*opt;

	opt = ++argv[*np];

	for( o=ENDP(otable)-1;  o >= otable;  o-- ) {
		if( o->name[1] == 0 && *opt == o->name[0] ) {
			if( *(++argv[*np]) == 0 )
				argv[(*np)++] = 0;
			return( o );
		}
		else {
			if( strcmp(o->name,opt) == 0 ) {
				argv[(*np)++] = 0;
				return( o );
			}
		}
	}
	err( YES, "Illegal option: `%s'", opt );
	return( NO );	/* for lint */
}

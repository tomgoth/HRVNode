/* 	lw.c		Paul Albrecht		July 1987
 
	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	13 June 1995 (GBM)
	EMACS_MODES:	tabstop=4

	Plot drivers for a the Apple LaserWriter.  The plotting space
	for the laser writer is always the same number (300*2) of units
	per inch.  This is done so that the line width and font shapes
	are not changed in size compressed in one dimension.  This code
	needs the prolog file pslw.pro.

	The paths are defined with M (move) and N (next) and stroked
	with L (line) or F (filled region).
*/

#include	"plt.h"
#include	"plot.h"

static void sendstring();
static void flushpath();
static void PTERMInit();
static void setfont();
static void linewidth();
static void linemod();
static void setgray();

#define		MAXPATH		1475	/* Laser Writer can't handle 1500 */

static	Ptype	oldx, oldy;
static	long	npath;


openpl()
{
	npath = 0;
}

closepl()
{
	flushpath();
	fflush( stdout );
}

erase() {
}

space( x0, y0, x1, y1 )
Ptype	x0, y0, x1, y1;
{
	printf( "%.5lg %.5lg %.5lg %.5lg %.5lg ", fscl, xorig, yorig, xorig+xdim, yorig+ydim );
	printf("%ld %ld %ld %ld WDEF\n", (long)x0, (long)y0, (long)x1, (long)y1);
	setfont( DEFAULT_FONT, (double)DEFAULT_PS );
}

label( lbl )
char	*lbl;
{
	sendstring( lbl );
	printf( " show\n" );
	npath = 0;
}

line( x0, y0, x1, y1 )
Ptype	x0, y0, x1, y1;
{
	move( x1, y1 );
	printf( "%ld %ld L\n", (long)(x0-oldx), (long)(y0-oldy) );
	npath = 0;
}

move( x, y )
Ptype	x, y;
{
	flushpath();
	printf( "%ld %ld M\n", (long)x, (long)y );
	oldx = x;
	oldy = y;
	npath = 1;
}

cont( x, y )
Ptype	x, y;
{
	if( npath == 0 )
		err( YES, "Called cont() before move()" );

	if( npath == MAXPATH-1 ) {
		printf( "%ld %ld L\n%ld %ld M\n",
			(long)(x-oldx), (long)(y-oldy), (long)x, (long)y );
		npath = 1;
	}
	else {	printf( "%ld %ld N\n", (long)(x-oldx), (long)(y-oldy) );
		npath++;
	}

	oldx = x;
	oldy = y;
}

static void sendstring( s )
char	*s;
{
register char	c;

	flushpath();
	putchar( '(' );
	while( c = *s++ ) {
		 if( (c == ')') || (c == '(') || (c == '\\') )
			putchar('\\');
	 	if( (c > 0176) || (c <040) ) {
			putchar( '\\' );
			putchar( ((c>>6)&07)+'0' );
			putchar( ((c>>3)&07)+'0' );
			putchar( (c&07)+'0' );
		}
		else	putchar( c );
	}
	putchar( ')' );
	npath = 0;
}

static void flushpath()
{
	if( npath > 1 ) {
		printf( "0 0 L\n" );
		npath = 0;
	}
}

/***** The following subroutines replace the routines in iface.c *****/

static	struct	{
	char	*just;
	short	pos;
	}	*j, jtypes[] = {
	"LB", 0, "LC", 1, "LT", 2, "CT", 3, "RT", 4, "RC", 5,
	"RB", 6, "CB", 7, "CC", 8 };

plabel( lbl, x, y, just, angle )
Ptype	x, y;
char	*lbl, *just;
double	angle;
{
static	char	jtmp[3];

	if( lbl[0] == 0 )
		return;

	for( j=jtypes; j < ENDP(jtypes) ;  j++ ) {
		if( strcmp(j->just,just) == 0 )
			break;
	}
	if( j == ENDP(jtypes) ) {
		err( NO, "Unknown mode `%s' in plabel(%s,...)", just, lbl );
	}
	else {	sendstring( lbl );
		printf( " %ld %ld %ld %.5lg T\n", (long)x, (long)y, (long)(j->pos), angle );
	}
}

alabel( what, lbl, x, y )
Ptype	x, y;
char	*what, *lbl;
{
	sendstring( lbl );
	printf( " %ld %ld %s\n", (long)x, (long)y, what );
}

elabel( what, base, exp, x, y )
char	*what, *base, *exp;
Ptype	x, y;
{
	sendstring( exp );
	putchar( ' ' );
	sendstring( base );
	printf( " %ld %ld %s\n", (long)x, (long)y, what );
}

strsize( str, xsize, ysize, angle )	/* no string size function */
char	*str;
Ptype	*xsize, *ysize;
double	angle;
{
short	len;

	if( fabs(angle) < 1  ) {
		*xsize = chwd * strlen(str);
		*ysize = chht;
	}
	else	if( fabs(angle-90) < 1 ) {
		*xsize = chht;
		*ysize = chwd * strlen(str);
	}
	else	err( NO, "Bad angle %.5lg in for strsize(%s,...)", angle, str );
}


scatterplot( plt, x, y, yneg, ypos )
PltPtr	plt;
Ptype	x, y, yneg, ypos;
{
static	char	lbl[2];

	flushpath();
	yneg -= y;
	ypos -= y;
	if( plt->symbol )
		printf( "%d %ld %ld %ld %ld SY\n", (int)(plt->pc-'0'),
		(long)yneg, (long)ypos, (long)x, (long)y );
	else {	lbl[0] = plt->pc;
		sendstring( lbl );
		printf( " %ld %ld %ld %ld SP\n", (long)yneg, (long)ypos, (long)x, (long)y );
	}
}


PolyDef( x, y, what )
Ptype	x, y;
Const	what;
{
int		fill;

	fill = -1;
	switch( what ) {
		case CMOVE:
			move( x, y );
			break;
		case CBBCONT:
		case CCONT:
			cont( x, y );
			break;
		case CFILL:
		case CFILLI:
			cont( x, y );
			fill = 0;
			break;
		case CSTROKE:
			cont( x, y );	/* flow through */
		case COSTROKE:
			flushpath();
			break;
		case CBBFILL:
		case CBBFILLI:
			cont( x, y );
			fill = 1;
			break;
		default:
			err( YES, "Bad PolyDef() code %d", what );
	}
	if( fill >= 0 ) {
		printf( "%d F\n", fill );
		npath = 0;
	}
	if( npath == MAXPATH )
		fprintf( stderr, "Polygon path exceeds %d sections", MAXPATH );
}

/****************** Special subroutines needed by plt *******************/

special( what, arg0, arg1 )
PtrUnion	arg0, arg1;
{
	switch( what ) {
		case SETPTERM:
			PTERMInit( arg0.c );
			break;
		case SETLINEWIDTH:
			linewidth( *arg0.d );
			break;
		case SETLINEMODE:
			linemod( arg0.c );
			break;
		case SETFONT:
			setfont( arg0.c, *arg1.d );
			break;
		case SETGRAY:
			flushpath();	/* flow through */
		case SETGRAYD:
			setgray( *arg0.d );
			break;
	}
}


static void PTERMInit( pterm )
char	*pterm;
{
double	f1, f2, f3, f4, f5;
short	n;
char	ptmp[50];

	if( pterm == 0 ) {
		ptmp[0] = 0;
		n = 1;
	}
	else
		n = sscanf( pterm, "%s%lf%lf%lf%lf%lf", ptmp, &f1, &f2, &f3, &f4, &f5 );

	PTERMLookup( ptmp, "lw" );	/* defines p */

	if( fscl == DEFAULT )
		fscl = (n > 1) ? f1 : 0.85;
	if( xdim == DEFAULT )
		xdim = (n > 2) ? f2 : p->xinches;
	if( ydim == DEFAULT )
		ydim = (n > 3) ? f3 : p->yinches;
	if( xorig == DEFAULT )
		xorig = (n > 4) ? f4 : 0.5*(8.25-xdim) + 0.25;
	if( yorig == DEFAULT )
		yorig = (n > 5) ? f5 : 0.67*(11-ydim);

	default_ps = DEFAULT_PS;
	xinch = yinch = 300*2;

	p->xinches = xdim;
	p->yinches = ydim;
	p->xfull = p->xsquare = xdim * xinch;
	p->yfull = ydim * yinch;

	p->ch = 1.1*chhtscl*fscl*default_ps/72.0*yinch + 0.5;
	p->cw = 0.5*chwdscl*fscl*default_ps/72.0*xinch + 0.5;

	chht = p->ch;
	chwd = p->cw;
}

/************************************************************************/

static	struct	{
	char	*name;
	char	*code;
	}	*f, fonts[] = {
	"Helvetica", "h",		"Helvetica-Bold", "hb",
	"Helvetica-Oblique", "ho",	"Helvetica-BoldOblique", "hbo",
	"Helvetica-BoldOblique", "hob", 	
	"Courier", "c",			"Courier-Bold",	"cb",
	"Courier-Oblique", "co",	"Courier-BoldOblique", "cbo",
	"Courier-BoldOblique", "cob",
	"Times-Roman", "t",		"Times-Roman", "tr",
	"Times-Italic", "ti",
	"Times-Bold", "tb",		"Times-BoldItalic", "tbi",
	"Times-BoldItalic", "tib",	"Symbol", "s",
	NULLS, NULLS };

static	char	*prev_lm="";


static void setfont( name, ps )
char	*name;
double	ps;
{
static	double	prev_ps = -1;
static	char	*prev_name;
short	n;
char	code[4], *font = name;

	n = 0;
	while( *font ) {
		code[n++] = *(font++) | 040;
		while( *font && *(font++) != '-' );
	}
	code[n] = 0;
		
	for( f=fonts;  f->code != NULLS && strcmp(code,f->code) != 0; f++ );

	if( f->code == NULLS ) {
		f = &fonts[0];
		err( NO, "Unknown font: %s -- Using: %s", name, f->name );
	}
	if( ps < 0  ) {
		err( NO, "Bad font size: %.5lg -- Using 16", ps );
		ps = 16;
	}
	if( prev_ps != ps || prev_name != f->name ) {
		printf( "/%s %.5lg SF\n", prev_name=f->name, prev_ps=ps );
		if( *prev_lm != 0 && strcmp(prev_lm,"solid") != 0 )
			printf( "%s setdash\n", prev_lm );
	}
}


static void linewidth( lw )
double	lw;
{
static	double	prev_lw = -1;

	if( lw < 0 ) {
		err( NO, "Bad line width: %.5lg", lw );
		lw = 2;
	}
	if( prev_lw != lw )
		printf( "%.5lg LW\n", prev_lw = lw );
}


static void linemod( lm )
char	*lm;
{
	flushpath();
	if( strcmp(prev_lm,lm) != 0 )
		printf( "%s setdash\n", prev_lm=lm );
}


static void setgray( gray )
double	gray;
{
	if( gray < 0 || gray > 1) {
		err( NO, "Bad gray value: %.5lg", gray );
		gray = 0;
	}
	printf( "%.5lg setgray\n", gray );
}

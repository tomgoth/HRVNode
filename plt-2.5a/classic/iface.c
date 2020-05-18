/*	iface.c		Paul Albrecht		Aug 1987
 
	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	13 June 1995 (GBM)
	EMACS_MODES:	tabstop=4

	Interface between higher level plt calls and the lower
	level Unix plot utilities.

	Copyright (C) Paul Albrecht 1988.  All rights reserved.
*/

#include	"plt.h"
#include	"plot.h"
#include	"text.h"

static void label0();
static void label90();

static	Ptype	xax_off, yax_off, xax_roff, yax_roff;


plabel( lbl, x, y, just, angle )
char	*lbl, *just;
Ptype	x, y;
double	angle;
{
	if( lbl[0] == 0 )
		return;

	if( fabs(angle) < 1 )
		label0( lbl, x, y, just );
	else	if( fabs(angle-90) < 1 )
			label90( lbl, x, y, just );
	else	err( NO, "Bad angle %g in for plabel(%s,...)", just, lbl );
}


static void label0( lbl, x, y, just )	/* horizontal label */
char	*lbl, *just;
{
short	len;
Boolean	errflag;

	len = strlen( lbl );
	errflag = NO;

	switch( just[0] ) {
		case 'L':				break;
		case 'R':	x -= len*chwd;		break;
		case 'C':	x -= (3*len-1)*chwd/6;	break;
		default:	errflag = YES;		break;
	}
	switch( just[1] ) {
		case 'B':			break;
		case 'T':	y -= chht;	break;
		case 'C':	y -= chht/3;	break;
		default:	errflag = YES;	break;
	}
	if( errflag ) {
		err( NO, "Unknown justification mode `%s'", just );
	}
	else {	move( x, y );
		label( lbl );
	}
}

static void label90( lbl, x, y, just )	/* vertical label */
char	*lbl, *just;
{
static	char	chr[2] = { 0, 0 };
short	len;
Boolean	errflag;

	len = strlen( lbl );
	errflag = NO;

	switch( just[0] ) {
		case 'L':				break;
		case 'R':	y -= (3*len-1)*chht/3;	break;
		case 'C':	y -= (3*len-1)*chht/6;	break;
		default:	errflag = YES;		break;
	}
	switch( just[1] ) {
		case 'T':			break;	
		case 'B':	x -= chwd;	break;
		case 'C':	x -= chwd/3;	break;
		default:	errflag = YES;	break;
	}
	if( errflag ) {
		err( NO, "Unknown justification mode `%s'", just );
	}
	else {	while( len > 0 ) {
			move( x, y );
			chr[0] = lbl[--len];
			label( chr );
			y += chht;
		}
	}
}

/********* Output the tic labels and axis label for the axes *************/

alabel( what, lbl, x, y )
char	*what, *lbl;
Ptype	x, y;
{
short	n;

	if( strcmp(what,"XN") == 0 ) {
		n = (lbl[0] == '-') ? 2*chwd/3 : 0;
		label0( lbl, x-n, y, "CT" );
		xax_off = chht;
	}
	else	if( strcmp(what,"YN") == 0 ) {
			label0( lbl, x-chwd/3, y, "RC" );
			n = chwd*strlen(lbl)+chwd/3;
			if( n > yax_off )
				yax_off = n;
		}
	else	if( strcmp(what,"XNR") == 0 ) {
			n = (lbl[0] == '-') ? 2*chwd/3 : 0;
			label0( lbl, x+n, y+chht/4, "CB" );
			xax_roff = chht;
		}
	else	if( strcmp(what,"YNR") == 0 ) {
			label0( lbl, x+chwd/3, y, "LC" );
			n = chwd*strlen(lbl)+chwd/3;
			if( n > yax_roff )
				yax_roff = n;
		}
	else	if( strcmp(what,"XT") == 0 ) {
			label0( lbl, x, y-xax_off, "CT" );
		}
	else	if( strcmp(what,"YT") == 0 ) {
			label90( lbl, x-yax_off, y, "CB" );
		}
	else	if( strcmp(what,"XTR") == 0 ) {
			label0( lbl, x, y+xax_roff, "CB" );
		}
	else	if( strcmp(what,"YTR") == 0 ) {
			label90( lbl, x+yax_roff, y, "CT" );
		}
	else	err( YES, "Bad call: alabel(%s,...)", what );
}

/****************** Output log tic labels for the axes ********************/

elabel( what, base, exponent, x, y )
char	*what, *base, *exponent;
Ptype	x, y;
{
Ptype	off;

	off = chwd * (strlen(base) + strlen(exponent));

	if( strcmp(what,"XE") == 0 ) {
		x += chwd/3;
		label0( exponent,  x+off/2, y, "RT" );
		label0( base, x-off/2, y-chht/2, "LT" );
		xax_off = 3*chht/2;
	}
	else 	if( strcmp(what,"YE") == 0 ) {
			x -= chwd/3;
			label0( exponent, x, y+(chht+3)/6, "RB" );
			label0( base, x-off, y-chht/3, "LB" );
			if( off > yax_off )
				yax_off = off;
		}
	else	if( strcmp(what,"XER") == 0 ) {
			x += chwd/3;
			label0( exponent,  x+off/2, y+3*chht/4, "RB" );
			label0( base, x-off/2, y+chht/4, "LB" );
			xax_roff = 3*chht/2;
		}
	else 	if( strcmp(what,"YER") == 0 ) {
			x += chwd/2;
			label0( exponent, x+off, y+(chht+3)/6, "RB" );
			label0( base, x, y-chht/3, "LB" );
			if( off > yax_roff )
				yax_roff = off;
		}
	else	err( YES, "Bad call: elabel( %s,...)", what );
}

/****** Scatterplot characters or symbols with or without error bars *******/

static	char	circle[] = {100, 0, 89, 46, 57, 82, 12, 99, -34, 94, -74, 66,
 -96, 24, -96, -23, -74, -65, -34, -93, 12, -98, 57, -81, 89, -45, 100, 0};
static	char	diamond[] = {0, -15, 13, 0, 0, 15, -10, 0, 0, -12 };
static	char	triangle[] = {0, 1, -1, -1, 1, -1, 0, 1};
static	char	utriangle[] = {0, -1, -1, 1, 1, 1, 0, -1};
static	char	square[] = {-1, -1, -1, 1, 1, 1, 1, -1, -1, -1};

static	struct	syminf {
	char	*pts;	/* x,y points specifying the symbol contour */
	short	npts;
	double	scl;	/* factor for converting *pts to inches */
	}	*sym, syms[] = {
		circle, sizeof(circle), 0.11,
		utriangle, sizeof(utriangle), 12,
		diamond, sizeof(diamond), 1,
		square, sizeof(square), 10,
		triangle, sizeof(triangle), 12 };


scatterplot( plt, x, y, yneg, ypos )
PltPtr	plt;
Ptype	x, y, yneg, ypos;
{
static	char	lbl[2] = {0, 0};
double	xscl, yscl;
Ptype	n, xd, yd, xdmin, xdmax, ydmin, ydmax;

	if( plt->symbol ) {
		sym = &syms[(plt->pc-'0') % (sizeof(syms)/sizeof(syms[0]))];
		xdmin = ydmin =  30000;
		xdmax = ydmax = -30000;
		xscl = xinch*sym->scl/250;
		yscl = yinch*sym->scl/250;

		for( n=0;  n < sym->npts;  n+=2 ) {
			xd = sym->pts[n];
			yd = sym->pts[n+1];
			xd = xscl*xd + (xd < 0 ? -0.5 : 0.5);
			yd = yscl*yd + (yd < 0 ? -0.5 : 0.5);
			if( n == 0 )
				move( x+xd, y+yd );
			else
				cont( x+xd, y+yd );
			if( xd < xdmin ) xdmin = xd;
			if( xd > xdmax ) xdmax = xd;
			if( yd < ydmin ) ydmin = yd;
			if( yd > ydmax ) ydmax = yd;
		}
	}
	else {	if( plt->pc != '.' ) {
			lbl[0] = plt->pc;
			label0( lbl, x, y, "CC" );
		}
		else	point( x, y );

		xdmax = chwd/2;
		xdmin = -xdmax;
		ydmax = chht/3;
		ydmin = -ydmax;
	}

	if( ypos-y >= ydmax ) {
		move( x+xdmin, ypos );
		cont( x+xdmax, ypos );
		move( x, ypos );
		cont( x, y+ydmax );
	}
	if( yneg-y <= ydmin ) {
		move( x+xdmin, yneg );
		cont( x+xdmax, yneg );
		move( x, yneg );
		cont( x, y+ydmin );
	}
}


PolyDef( x, y, what )
Ptype	x, y;
Const	what;
{
static	int	x0, y0;

	switch( what ) {
		case CMOVE:
			move( x, y );
			x0 = x;
			y0 = y;
			break;
		case CCONT:
			if( oldGrayLevel != PS_WHITE )
				cont( x, y );
			break;
		case CBBCONT:
			cont( x, y );
			break;
		case CFILL:
		case CSTROKE:
		case CFILLI:
			if( oldGrayLevel != PS_WHITE ) {
				cont( x, y );
				cont( x0, y0 );
			}
			break;
		case COSTROKE:
			break;
		case CBBFILL:
		case CBBFILLI:
			cont( x, y );
			cont( x0, y0 );
			break;
		default:
			err( YES, "Bad PolyDef() code %d", what );
	}
}

/******* Give the x and y dimension of a string in device units ******/

strsize( str, xsize, ysize, angle )
char	*str;
Ptype	*xsize, *ysize;
double	angle;
{
	if( fabs(angle) < 1  ) {
		*xsize = chwd * strlen(str);
		*ysize = chht;
	}
	else	if( fabs(angle-90) < 1 ) {
		*xsize = chwd;
		*ysize = chht * strlen(str);
	}
	else	err( NO, "Bad angle %lg in for strsize(%s,...)", angle, str );
}

/*	plt/window.c		Paul Albrecht		Sept 1984

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	13 June 1995 (GBM)
	EMACS_MODES:	tabstop=4
*/

#include	"plt.h"
#include	"plot.h"
#include	"axis.h"
#include	"figs.h"
#include	"text.h"

static void turnoff();

WindowDef( config, subsect )
char	*config, *subsect;
{
static	float
	mgrid[] = {0, .935, 1, .935, 0, .941, 1 , .941, -1 },
	mwin[]  = {.12, .1, .94, .85 };
static	float
	*msgrid = mgrid,
	mswin[]  = {.2, .17, .92, .85 };
static	float
	bgrid[] = {0, .935, 1, .935, 0, .941, 1 , .941, 0, .463, 1, .463, 0,
	.468, 1, .468, -1 },
	bwin[]  = {.074, .53, .96, .88, .074, .06, .96, .41 };
static	float
	*bsgrid = bgrid,
	bswin[] = {.1, .53, .945, .85, .1, .07, .945, .39 };
static	float
	qgrid[] = {0, .935, 1, .935, 0, .941, 1 , .941, 0, .463, 1, .463, 0,
	.468, 1, .468, .498, 0, .498, .94, .502, 0, .502, .94, -1},
	qwin[]  = {.074, .53, .454, .88, .58, .53, .96, .88, .074, .06,
	.454, .41, .58, .06, .96, .41 };
static	float
	qpgrid[] = {0, .935, 1, .935, 0, .941, 1 , .941, 0, .463, 1, .463, 0,
	.468, 1, .468, .498, 0, .498, .94, .503, 0, .503, .94, -1},
	qpwin[]  = {.08, .54, .45, .87, .59, .54, .96, .87, .08, .07,
	.45, .40, .59, .07, .96, .40 };
static	float
	*qsgrid = qgrid,
	qswin[] = {.1, .53, .435, .85, .6, .53, .945, .85, .1, .07,
	.435, .39, .6, .07, .945, .39 };

float	*wl, *gr;

	if( config == 0 )
		config = "m";
	if( subsect == 0 )
		subsect = "0";

	if( strcmp(config,"m") == 0 ) {
		gr = mgrid;
		wl = mwin;
		title.text = "";
	}
	else	if( strcmp(config,"ms") == 0 ) {
		gr = msgrid;
		wl = mswin;
		title.text = "";
	}
	else	if( strcmp(config,"b") == 0 ) {
		gr = bgrid;
		wl = bwin;
	}
	else	if( strcmp(config,"bs") == 0 ) {
		gr = bsgrid;
		wl = bswin;
	}
	else	if( strcmp(config,"q") == 0 ) {
		gr = qgrid;
		wl = qwin;
	}
	else	if( strcmp(config,"qs") == 0 ) {
		gr = qsgrid;
		wl = qswin;
	}
	else	if( strcmp(config,"qp") == 0 ) {
		gr = qpgrid;
		wl = qpwin;
	}

	if( *subsect < '1' || *subsect > '9' ) {
		xa.min = ya.min = xmin = ymin = xwmins = ywmins = 0;
		xa.max = ya.max = xmax = ymax = xwmaxs = ywmaxs = 1;

		theight = 0.97;
		title.just = "CC";

		while( *gr >= 0 ) {
			FigureDef( CONNECT, WINC, gr[0], gr[1], gr[2], gr[3], NULLS );
			gr += 4;
		}
		FigureDef( BOX, WINC, 0.0, 0.0, 1.0, 1.0, NULLS );
		SupressionDef( "xy" );
	}
	else {	wl  +=  (*subsect-'1')*sizeof(*wl);
		xwmins = wl[0];
		ywmins = wl[1];
		xwmaxs = wl[2];
		ywmaxs = wl[3];
		omode &= ~ERASE;
		title.just = "CB";
		theight = 1.07;
		if( config[0] != 'm' )
			TextInit( -1 );
		SupressionDef( "e" );
		xa.lbl = "";
	}
}

/************ Supress the generation of parts of the graph ***************/


SupressionDef( str )
char	*str;
{
char	tmp[120];

	if( Plot.supress != 0 ) {
		strcpy( tmp, Plot.supress );
		free( Plot.supress );
	}
	else
		tmp[0] = 0;
	strcat( tmp, str );

	Plot.supress = StringSave( tmp );
}


gsupress()
{
AxisPtr	a;
char	chr;

	a = 0;
	while( chr = *Plot.supress++ ) {
		switch( chr ) {
			case 'a': turnoff(a,AXIS);		break;
			case 'e': turnoff(a,ERASE);		break;
			case 'f': turnoff(a,FIGURES);		break;
			case 'g': turnoff(a,GRIDMARKS);		break;
			case 'm': turnoff(a,TICMARKS);		break;
			case 'n': turnoff(a,TICNUMS);		break;
			case 'l': turnoff(a,LABELS);		break;
			case 'p': turnoff(a,PLOTS);		break;
			case 't': turnoff(a,TITLES);		break;
			case 'x': xa.mode = 0;			break;
			case 'y': ya.mode = 0;			break;
			case 'C': omode = xa.mode = ya.mode = 077777;
				  a = 0;
				  break;
			case 'A': a = 0;		break;
			case 'X': a = &xa;		break;
			case 'Y': a = &ya;		break;
			default: err( NO, "Bad element `%c' for supress", chr );
		}
	}
}

static void turnoff( a, flag )
AxisPtr	a;
Const	flag;
{
short	mask;

	mask  = ~flag;
	if( a == 0 )
		omode &= mask;
	if( a == 0 || a == &xa )
		xa.mode &= mask;
	if( a == 0 || a == &ya )
		ya.mode &= mask;
}

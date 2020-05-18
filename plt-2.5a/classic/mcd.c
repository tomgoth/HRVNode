/*	mcd.c		Paul Albrecht		Aug 1985
 
	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4

	Adaptation of GBM's primitives for MASSCOMP graphics display
*/

#include	"plt.h"


static	double	yoff;			/* offset for chars */
static	double	xcurs, ycurs;		/* current cursor position */
static	char	*gelem;
static	int	ncolor;

openpl()
{
	mgiasngp(0, 0);
	mgifetchgf(0, "7x9_bold");
	mgicm( 1, mgfcns("very-light-vivid-greenish-blue") );
	mgicm( 2, mgfcns("light-vivid-orange") );
	mgicm( 3, mgfcns("very-light-vivid-red") );
	mgicm( 4, mgfcns("white") );
	mgicm( 5, mgfcns("light-vivid-yellow") );
	mgicm( 6, mgfcns("light-green") );
	mgihue( 3 );
	ncolor = 7;
}


closepl()
{
	mgideagp();
}

erase()
{
	printf( "\033[2J" );
	fflush( stdout );
	mgiclearpln(2, -1, 0);
}

space(x0, y0, x1, y1)
int x0, y0, x1, y1;
{
	mgrvcoor(2, (double)x0, (double)y0, (double)x1, (double)y1);
	yoff = 4.0*(y1-y0)/600;
}

/*****************************************************************************/

label( s )
char	*s;
{
	if( *s == 0 )
		return;
 	mgrgfs(xcurs, ycurs-yoff, 0, s);
}

line(x1, y1, x2, y2)
{
	mgrl((double)x1, (double)y1, xcurs = (double)x2, ycurs = (double)y2);
}

linemod( s )
char	*s;
{
	if( strcmp(s, "solid") == 0 )
		mgidash( 1, -1);
	else if( strcmp(s, "dotted") == 0 )
		mgidash( 1, 0xAAAAAAAA );
	else if( strcmp(s, "longdashed") == 0 )
		mgidash( 1, 0xFF00FF00 );
	else if( strcmp(s, "shortdashed") == 0 )
		mgidash( 1, 0xF0F0F0F0 );
	else if( strcmp(s, "dotdashed") == 0 )
		mgidash( 1, 0x9C9C9C9C );
}


/****************************************************************************/

point( x, y )
{
	mgrp(xcurs = (double)x, ycurs = (double)y);
}

move(x, y)
{
	xcurs = x;
	ycurs = y;
}

cont(x, y)
{
	mgrl( xcurs, ycurs, (double)x, (double)y );
	xcurs = x;
	ycurs = y;
}

special( what, arg0, arg1 )
PtrUnion	arg0, arg1;
{
	switch( what ) 	{
		case SETPTERM:
			PTERMInit( arg0.c );
			break;
		case GRAPHELEMENT:
			gelem = arg0.c;
			break;
		case SETCOLOR:
			setcolor( arg0.c );
			break;
		case SETLINEMODE:
			linemod( arg0.c );
			break;
	}
}

static	PTERMInit( pterm )
char	*pterm;
{
	 PTERMLookup( pterm, "mcd" );
}


static	defaultcolor( dcolor )
{
}

static	setcolor( color )
char	*color;
{
	if( color == 0 || *color == 0 ) {
		switch( *gelem ) {
			case 'a':	mgihue( 1 );	break;
			case 'f':	mgihue( 2 );	break;
			case 'l':	mgihue( 3 );	break;
			case 'p':	mgihue( 4 );	break;
			case 't':	mgihue( 5 );	break;
			case 'g':	mgihue( 6 );	break;
			default:	mgihue( 5 );	break;
		}
	}
	else {	if( *color >= '0' && *color <= '0' )
			mgihue( (int)LongNum(color) );
		else {	mgicm( ncolor, mgfcns(color) );
			mgihue( ncolor++ );
		}
	}
}

/*	sun.c			Paul Albrecht				 Feb 1988
 
	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4

	Plt driver for Sun systems in native mode or SunView.
*/

#include	"plt.h"
#include	<sunwindow/window_hs.h>
#include	<pixrect/pixrect_hs.h>


#define		X(N)	(int)(xScl*((N)-xMin)+0.5)
#define		Y(N)	(yDim - (int)(yScl*((N)-yMin)+0.5))


static	Pixrect	*pr;
static	Pixwin	*pw;
static	Pixfont	*pf;

static	int		paintBackground, paintColor;
static	Boolean	usePixrect;

static	Ptype	xlast, ylast;			/* Ptype is int or short */
static	Ptype	xMin, yMin, xDim, yDim;
static	double	xScl=1, yScl=1;




openpl()
{
void	OpenPixelWindow();

	if( pw != 0 )
		return;

	OpenPixelWindow();
	paintBackground = PIX_NOT(PIX_SET);
	paintColor = PIX_SET;
}


closepl()
{
	if( pr != 0 ) {
		pr_close( pr );
		pr = 0;
	}
	if( pw != 0 ) {
		pw_batch_off( pw );
		pw_close( pw );
		pw = 0;
	}
	if( pf != 0 ) {
		pf_close( pf );
		pf = 0;
	}
}


erase() 
{
	if( usePixrect )
		pr_rop( pr, 0, 0, xDim, yDim, paintBackground, 0, 0, 0 );
	else
		pw_rop( pw, 0, 0, xDim, yDim, paintBackground, 0, 0, 0 );
}


space( x0, y0, x1, y1 )
Ptype	x0, y0, x1, y1;
{
	xMin = x0;
	yMin = y0;
	xScl = (double)xDim/(x1-x0+1);
	yScl = (double)yDim/(y1-y0+1);
}


label( s )
char	*s;
{
	if( usePixrect )
		pr_ttext( pr, xlast, ylast, paintColor, pf, s );  
	else
		pw_ttext( pw, xlast, ylast, paintColor, pf, s );  
}


line( x0, y0, x1, y1 )
Ptype	x0, y0, x1, y1;
{
	xlast = X(x1);
	ylast = Y(y1);
	if( usePixrect )
		pr_vector( pr, X(x0), Y(y0), xlast, ylast, paintColor, 1 );
	else
		pw_vector( pw, X(x0), Y(y0), xlast, ylast, paintColor, 1 );
}


point( x, y )
Ptype	x, y;
{
	move( x, y );
	cont( x, y );
}


move( x, y )
Ptype	x, y;
{
	xlast = X(x);
	ylast = Y(y);
}


cont( x, y )
Ptype	x, y;
{
	x = X(x);
	y = Y(y);
	if( usePixrect )
		pr_vector( pr, xlast, ylast, x, y, paintColor, 1 );
	else
		pw_vector( pw, xlast, ylast, x, y, paintColor, 1 );
	xlast = x;
	ylast = y;
}

/************************************************************************/

special( what, arg0, arg1 ) 
PtrUnion	arg0, arg1;
{
	switch( what ) {
		case SETPTERM:
			if( pw == 0 )
				openpl();

			PTERMLookup( arg0.c, "sun" );
			p->xfull = p->xsquare = xDim*10;
			p->yfull = yDim*10;
			chwd = p->cw = pf->pf_defaultsize.x*10;
			chht = p->ch = pf->pf_defaultsize.y*10;
			p->xinches = xDim/75.0;
			p->yinches = yDim/75.0;
			xinch = p->xfull/p->xinches;
			yinch = p->yfull/p->yinches;
			break;
	}
}


void	OpenPixelWindow()
{
char	*gfxWin, *getenv();
void	OpenPixelFont();

	gfxWin = getenv( "WINDOW_GFX" );
	usePixrect = (gfxWin == 0 || *gfxWin == 0);

	if( usePixrect ) {
		printf( "[32;0H\n" );
		pr = pr_open( "/dev/fb" );
		if( pr == 0 )
			estop( "Can't open /dev/fb as screen" );
		xDim = pr->pr_size.x;
		yDim = pr->pr_size.y;
	}
	else {
		int	fd = open( gfxWin, 2 );
		if( fd < 0 )
			estop( "Can't open window %s", gfxWin );

		pw = pw_open( fd );
		if( pw == 0 )
			estop( "Can't open window %s for drawing", gfxWin );
		pw_use_fast_monochrome( pw );
		pw_batch_off( pw );

		xDim = pw->pw_clipdata->pwcd_prmulti->pr_size.x;
		yDim = pw->pw_clipdata->pwcd_prmulti->pr_size.y;
	}

	OpenPixelFont();
}



typedef	struct	{
	short	size;
	short	xMinDim;
	short	yMinDim;
	}	SizeSpec;

static	SizeSpec	pointSizes[] = {
		10,	250,	350,
		12,	300,	400,
		14,	550,	450,
		16,	650,	550,
		18,	800,	700,
		24,	9999,	9999,	/* dummy, never used */
		};				

void	OpenPixelFont()
{
SizeSpec	*pt;
char		fontFile[50];

	for( pt=pointSizes;  pt < ENDP(pointSizes)-1;  pt++ ) {
		if( pt[1].xMinDim > xDim || pt[1].yMinDim > yDim )
			break;
	}

	sprintf( fontFile, "/usr/lib/fonts/fixedwidthfonts/cour.r.%d", pt->size );
	pf = pf_open( fontFile );
	if( pf == 0 )
		estop( "Can't find %s", fontFile );
}

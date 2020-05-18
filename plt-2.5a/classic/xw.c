/*		xw.c			Paul Albrecht			May 1989
 
	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	13 June 1995 (GBM)
	EMACS_MODES:	tabstop=4

	X Windows driver for plt.
*/
	
#include	"plt.h"
#include	<signal.h>
#include	<X11/X.h>
#include	<X11/Xlib.h>
#include	<X11/Xatom.h>



#define		X(N)	(int)(xScl*((N)-xMin))
#define		Y(N)	(int)(yDim - (int)(yScl*((N)-yMin)))


static	Display		*dpy;
static	int			screen;
static	Window		window;
static	Pixmap		pixmap;
static	GC			gc;
static	XFontStruct	*font;
static	XWindowAttributes	windowAttr;


static	Ptype	xLast, yLast;			/* Ptype is int or short */
static	Ptype	xMin, yMin, xDim, yDim;
static	double	xScl=1, yScl=1;




openpl() 
{
void	OpenPixelWindow();

	if( dpy == 0 )
		OpenPixelWindow();
}


void	closepl()
{
XExposeEvent	*e;
XEvent			event;

	if( windowAttr.map_state == IsViewable ) {
		e = (XExposeEvent *)(&event);		/* Send message to window to redraw */
		e->type = Expose;
		e->send_event = True;
		e->display = dpy;
		e->window = window;
		e->x = 0;
		e->y = 0;
		e->width = windowAttr.width;
		e->height = windowAttr.height;
		XSendEvent( dpy, window, True, ExposureMask, &event );
	}
	XFlush( dpy );
}


erase()
{
	if( pixmap != window ) {
		XSetForeground( dpy, gc, WhitePixel(dpy,screen) );
		XFillRectangle( dpy, pixmap, gc, 0, 0, windowAttr.width, windowAttr.height );
		XSetForeground( dpy, gc, BlackPixel(dpy,screen) );
	}

	if( windowAttr.map_state == IsViewable )
		XClearWindow( dpy, window );
}


space( x0, y0, x1, y1 )
Ptype	x0, y0, x1, y1;
{
	xMin = x0;
	yMin = y0;
	xScl = (double)xDim/(x1-x0+1);
	yScl = (double)yDim/(y1-y0+1);
}


move( x,y )
Ptype	x, y;
{
	xLast = x;
	yLast = y;
}


cont( x, y )
Ptype	x, y;
{
	XDrawLine( dpy, pixmap, gc, X(xLast), Y(yLast), X(x), Y(y) );
	xLast = x;
	yLast = y;
}


line( x0, y0, x1, y1 )
Ptype	x0, y0, x1, y1;
{
	XDrawLine( dpy, pixmap, gc, X(x0), Y(y0), X(x1), Y(y1) );
	xLast = x1;
	yLast = y1;
}


point(x,y)
Ptype	x, y;
{
	XDrawPoint( dpy, pixmap, gc, X(x), Y(y) );
	xLast = x;
	yLast = y;
}


label( str )
char *str ;
{
	XDrawString( dpy, pixmap, gc, X(xLast), Y(yLast), str, strlen(str) );
}


#define	SETDASHES(MODE)		XSetDashes( dpy, gc, 0, MODE, sizeof(MODE)/sizeof(MODE[0]) )

linemod( lineMode )
char	*lineMode;
{
static	char	solid[] = { 100 }, dotted[] = { 2, 1 }, shortdashed[] = { 2, 2 },
					longdashed[] = { 4, 4 }, dotdashed[] = { 1, 2, 3, 2 };
static	char	*oldLineMode = "";

	if( strcmp(lineMode,oldLineMode) == 0 )
		return;

		if( strcmp(lineMode,"dotted") == 0 )
			SETDASHES( dotted );
	else	if( strcmp(lineMode,"dotdashed") == 0 )
			SETDASHES( dotdashed );
	else	if( strcmp(lineMode,"shortdashed") == 0 )
			SETDASHES( shortdashed );
	else	if( strcmp(lineMode,"longdashed") == 0 )
			SETDASHES( longdashed );
	else {	lineMode = "solid";
			SETDASHES( solid );
		}

	oldLineMode = lineMode;
}




special( what, arg0, arg1 ) 
PtrUnion	arg0, arg1;
{
	switch( what ) {
		case SETPTERM:
			openpl();		/* defines xDim, yDim and font size information */
			PTERMLookup( arg0.c, "xw" );
			p->xfull = p->xsquare = xDim*10;
			p->yfull = yDim*10;
			chwd = p->cw = font->max_bounds.width*10;
			chht = p->ch = (font->max_bounds.ascent+font->max_bounds.descent)*10;
			p->xinches = xDim/75.0;
			p->yinches = yDim/75.0;
			xinch = p->xfull/p->xinches;
			yinch = p->yfull/p->yinches;
			break;
		case SETLINEMODE:
			linemod( arg0.c );
			break;
	}
}

/************************************************************************/


Boolean		PltWindowFind();
void		CreatePltWindow();
void		OpenPixelFont();



void	OpenPixelWindow()
{
char	*windowName, *dpyName, *getenv();

	dpyName = getenv( "XPD" );
	if( dpyName != 0 && *dpyName == 0 )
		dpyName = 0;

	dpy = XOpenDisplay( dpyName );
	if( dpy == 0 ) {
		fprintf( stderr, "Can't open X server display\n" );
		exit( 1 );
	}
	screen = DefaultScreen( dpy );
	
	windowName = getenv( "XPLTWIN" );
	if( windowName == 0 || *windowName == 0 )
		windowName = "Plt Window";

	if( !PltWindowFind(windowName) ) {
		CreatePltWindow( windowName );
		if( !PltWindowFind(windowName) )
			sleep( 3 );	/* Time sharing, wait for window to show up */
		if( !PltWindowFind(windowName) ) {
			fprintf( stderr, "Missing xpltwin or something is botched!\n" );
			exit( 1 );
		}
	}

	XGetWindowAttributes( dpy, window, &windowAttr );
	xDim = windowAttr.width;
	yDim = windowAttr.height;

	gc = XCreateGC( dpy, pixmap, 0L, (XGCValues *)0 );
	XSetForeground( dpy, gc, BlackPixel(dpy,screen) );

	OpenPixelFont();
}



typedef	struct	{
	short	xMinDim;
	short	yMinDim;
	char	*fontName;
	}	SizeSpec;

static	SizeSpec	pointSizes[] = {
		250,	350,	"6x10",
		300,	400,	"6x12",
		550,	450,	"8x13",
		650,	550,	"9x15",
		9999,	9999,	"" /* dummy, never used */
		};				


void	OpenPixelFont()
{
SizeSpec	*pt;

	for( pt=pointSizes;  pt < ENDP(pointSizes)-1;  pt++ ) {
		if( pt[1].xMinDim > xDim || pt[1].yMinDim > yDim )
			break;
	}

	if( (font=XLoadQueryFont(dpy,pt->fontName)) == 0) {
		fprintf( stderr, "%s font not found\n", pt->fontName );
		exit (1);
	}
	XSetFont( dpy, gc, font->fid );
}





void	CreatePltWindow( windowName )
char	*windowName;
{
char	systemCall[128];

	sprintf( systemCall, "xpltwin '%s'", windowName );
	system( systemCall );
	sleep( 1 );
}

/*************************************************************************************/


ErrorHandler()
{
}

Boolean	PltWindowFind( windowName )
char	*windowName;
{
Drawable	*drawables;
Atom		pltWindowAtom, actualType;
long		nItems, bytesAfter;
int			actualFormat;
char		atomName[100];

	sprintf( atomName, "_xpltwin_%d_%s", screen, windowName );
	pltWindowAtom = XInternAtom( dpy, atomName, True );
	if( pltWindowAtom == None )
		return( NO );

	XGetWindowProperty( dpy, RootWindow(dpy,screen), pltWindowAtom, 0, 2, False,
		XA_DRAWABLE, &actualType, &actualFormat, &nItems, &bytesAfter,
			   (unsigned char **)&drawables );

	if( actualType != XA_DRAWABLE || actualFormat != 32 || nItems != 2 )
		return( NO );

	window = drawables[0];
	pixmap = drawables[1];
 	XFree( drawables );

	actualType = actualFormat = nItems = 0;
	XSetErrorHandler( ErrorHandler );
	XGetWindowProperty( dpy, window, pltWindowAtom, 0, 2, False,
		XA_DRAWABLE, &actualType, &actualFormat, &nItems, &bytesAfter,
			   (unsigned char **)&drawables );
	XSetErrorHandler( (void *)0 );

	if( actualType != XA_DRAWABLE || actualFormat != 32 || nItems != 2 )
		return( NO );

	if( window != drawables[0] )
		return( NO );

 	XFree( drawables );
	return( YES );
}

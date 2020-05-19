/*		xpltwin.c			Paul Albrecht			May 1989
 
	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4


	Make an window for xplt to plot in.

	This version modified 16 November 1989 by G. Moody to accept
	geometry arguments as for X Toolkit applications.  (This program
	does not use the X Toolkit and does not accept any other Toolkit
	options, nor does it support retrieval of geometry specifications
	from resource files.)  Usage:
	   xpltwin [ -g WIDTHxHEIGHT+-XOFF+-YOFF ] [ NAME ]
	The geometry can also be specified by the XPGEOMETRY environment
	variable.  Note that successful use of negative offsets requires
	that DSP_WIDTH and DSP_HEIGHT be correct for the display;  it should
	be possible to get this information from the X server.
*/

	
#include	"plt.h"
#include	<signal.h>
#include	<X11/X.h>
#include	<X11/Xlib.h>
#include	"X11/Xutil.h"
#include	<X11/Xatom.h>

#define DSP_WIDTH	1152
#define DSP_HEIGHT	900

typedef struct	{
	Display		*dpy;
	int			screen;
	Window		root;
	Window		window;
	GC			gc;
	char		*name;
	Pixmap		pixmap;
	int			pixmapWidth;
	int			pixmapHeight;
	Atom		atom;
	} GfxData, *Gfx;


Window	WindowFind();


main( argc, argv )
char	*argv[];
{
GfxData		gfxData;
char		*dpyName, *geometry, *getenv();
int		x = -10, y = 120, xDim = 500, yDim = 340;

	dpyName = getenv( "XPD" );
	if( dpyName != 0 && *dpyName == 0 )
		dpyName = 0;

	bzero( (char *)&gfxData, sizeof(gfxData) );

	gfxData.dpy = XOpenDisplay( dpyName );
	if( gfxData.dpy == 0 ) {
		fprintf( stderr, "Can't open display %s\n", XDisplayName(dpyName) );
		exit( 1 );
	}

	gfxData.screen = DefaultScreen( gfxData.dpy );
	gfxData.root = RootWindow( gfxData.dpy, gfxData.screen );
	gfxData.gc = DefaultGCOfScreen( ScreenOfDisplay(gfxData.dpy,gfxData.screen) );
	if (geometry = getenv("XPGEOMETRY"))
	    sscanf(geometry, "%dx%d%d%d", &xDim, &yDim, &x, &y);
	if (argc > 2 && argv[1][0] == '-' && argv[1][1] == 'g') {
	    sscanf(argv[2], "%dx%d%d%d", &xDim, &yDim, &x, &y);
	    argc -= 2;
	    argv += 2;
	}
	if (x < 0) x = DSP_WIDTH - xDim + x;
	if (y < 0) y = DSP_HEIGHT - yDim + y;

	gfxData.name = (argc > 1) ? argv[1] : "Plt Window";

	if( strcmp(gfxData.name,"debug") == 0 ) {
		WindowCreate( &gfxData, x, y, xDim, yDim);
		WindowMaintain( &gfxData );
	}

	if( fork() == 0 ) {
		WindowCreate( &gfxData, x, y, xDim, yDim);
		WindowMaintain( &gfxData );
	}
	else
		printf( "Created window \"%s\"\n", gfxData.name );
}


/**********************************************************************************/


WindowCreate( gfx, x, y, xDim, yDim )
Gfx		gfx;
int		x, y, xDim, yDim;
{
XEvent				event;
XWindowAttributes	attributes;
char				atomName[100];


	gfx->window = XCreateSimpleWindow( gfx->dpy, gfx->root, x, y, xDim, yDim, 1,
			BlackPixel(gfx->dpy,gfx->screen), WhitePixel(gfx->dpy,gfx->screen) );

	if( gfx->window == 0 ) {
		fprintf( stderr, "Can't create window %s", gfx->name );
		exit( 1 );
	}

	XSetStandardProperties( gfx->dpy, gfx->window, gfx->name, gfx->name,
			(Pixmap)0, (char **)0, 0, (XSizeHints *)0 );

	sprintf( atomName, "_xpltwin_%d_%s", gfx->screen, gfx->name );
	gfx->atom = XInternAtom( gfx->dpy, atomName, False );

	XGetWindowAttributes( gfx->dpy, gfx->window, &attributes );
	WindowConfigure( gfx, attributes.width, attributes.height );

	XMapWindow( gfx->dpy, gfx->window );
	XFlush( gfx->dpy );
	XSync( gfx->dpy, NO );

	XSelectInput( gfx->dpy, gfx->window, ExposureMask );

	while( XCheckWindowEvent(gfx->dpy,gfx->window,ExposureMask,&event) == 0
			 || event.type != Expose );
}


/**********************************************************************************/


WindowMaintain( gfx )
register Gfx	gfx;
{
XEvent		event;

	signal( SIGHUP, SIG_IGN );
	signal( SIGINT, SIG_IGN );

	XSelectInput( gfx->dpy, gfx->window, ExposureMask|StructureNotifyMask );

	while( YES ) {
		XNextEvent( gfx->dpy, &event );
		switch( event.type ) {
			case Expose:			
			case GraphicsExpose:
				WindowExpose( gfx, (XExposeEvent *)(&event) );
				break;
			case ConfigureNotify: {
				XConfigureEvent 	*config = (XConfigureEvent *)(&event);
				WindowConfigure( gfx, config->width, config->height );
				break;
			}
			case DestroyNotify:
				WindowPropertiesRemove( gfx );				
				exit( 0 );
				break;
		}
	}
}



WindowExpose( gfx, event )
Gfx				gfx;
XExposeEvent	*event;
{
XEvent	anotherEvent;

	if( gfx->pixmap == 0 )
		return;

	if( event != 0  )
		XCopyArea( gfx->dpy, gfx->pixmap, gfx->window, gfx->gc,
			event->x, event->y,	event->width, event->height, event->x, event->y );

	while( XCheckWindowEvent(gfx->dpy,gfx->window,ExposureMask,&anotherEvent) != 0 )
		WindowExpose( gfx, (XExposeEvent *)(&anotherEvent) );
}



WindowConfigure( gfx, width, height )
Gfx		gfx;
int		width, height;
{
Drawable	drawable[2];
Pixmap		newPixmap;

	if( gfx->pixmapWidth == width && gfx->pixmapHeight == height )
		return;

	newPixmap = XCreatePixmap( gfx->dpy, gfx->window, width, height,
		DefaultDepth(gfx->dpy,gfx->screen) );
	if( newPixmap == 0 ) {
		fprintf( stderr, "Warning: cannot allocate backup storage for plt window" );
		return;
	}

	if( gfx->pixmap != 0 ) {
		XCopyArea( gfx->dpy, gfx->pixmap, newPixmap, gfx->gc, 0, 0,
			gfx->pixmapWidth, gfx->pixmapHeight, 0, 0 );
		XFreePixmap( gfx->dpy, gfx->pixmap );
	}

	gfx->pixmap = newPixmap;
	gfx->pixmapWidth = width;
	gfx->pixmapHeight = height;

	drawable[0] = gfx->window;
	drawable[1] = (gfx->pixmap == 0 ? gfx->window : gfx->pixmap);
	XChangeProperty( gfx->dpy, gfx->root, gfx->atom, XA_DRAWABLE, 32, 
				PropModeReplace, (char *)drawable, 2 );
	XChangeProperty( gfx->dpy, gfx->window, gfx->atom, XA_DRAWABLE, 32, 
				PropModeReplace, (char *)drawable, 2 );
}


WindowPropertiesRemove( gfx )
Gfx			gfx;
{
Drawable	*drawables;
Atom		pltWindowAtom, actualType;
long		nItems, bytesAfter;
int			actualFormat;
char		atomName[100];

	drawables = 0;
	sprintf( atomName, "_xpltwin_%d_%s", gfx->screen, gfx->name );
	pltWindowAtom = XInternAtom( gfx->dpy, atomName, True );
	if( pltWindowAtom == None )
		goto RETURN;

	XGetWindowProperty( gfx->dpy, gfx->root, pltWindowAtom, 0, 2, False,
		XA_DRAWABLE, &actualType, &actualFormat, &nItems, &bytesAfter,
			   (unsigned char **)&drawables );

	if( actualType != XA_DRAWABLE || actualFormat != 32 || nItems != 2 )
		goto RETURN;

	if( gfx->window == drawables[0] )
		XDeleteProperty( gfx->dpy, gfx->window, pltWindowAtom );

RETURN:
	if( drawables != 0 )
		XFree( drawables );
	return;
}


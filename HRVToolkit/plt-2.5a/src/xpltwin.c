/* file: xpltwin.c	G. Moody	9 March 2001
			Last revised: 11 October 2002
Makes a window for xplt's output

Copyright (C) George B. Moody 2002

Recent changes:
  11 October 2002: default geometry is now square (better with Xinerama)
  19 March 2001: added support for reading geometry from X11 resource database
  10 June 2001: corrected comments on geometry specification
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

You may contact the author by e-mail (george@mit.edu) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________

This program is based on Paul Albrecht's original xpltwin and on basicwin
from the Xlib Programming Manual (O'Reilly).  It runs in the background
and might be called a server for xplt, although what it does is extremely
limited.

xpltwin accepts geometry arguments as for X Toolkit applications.  (This
program does not use the X Toolkit and does not accept any other Toolkit
options).  Usage:
     xpltwin [ -g WIDTHxHEIGHT+-XOFF+-YOFF ]
The geometry can also be specified by the XPGEOMETRY environment variable,
or by setting the "xplt.geometry" resource (e.g., in .Xdefaults).  The window
opened by xpltwin will be given the name specified by the value of the XPLTWIN
environment variable if set, or "Plt Window" otherwise.

In normal use, xplt checks to see if xpltwin is already running by looking for
a window named "Plt Window" (or another name if specified by the XPLTWIN
environment variable), and launches xpltwin (which runs in the background)
if necessary.  The output of xplt then goes into the xpltwin window.  The
window remains on-screen after xplt exits, since it is maintained by the
backgrounded xpltwin process.  At this point, the user has several choices:

* To display another plot in the same window, simply run xplt again.  xplt
  will not launch another instance of xpltwin if one is running already,
  unless you change the XPLTWIN environment variable.  If you wish to replace
  the old plot with the new one, no special action is needed;  if you wish
  to overlay the old plot with the new one, use xplt's -se option to avoid
  erasing the window before drawing the new plot.

* To display another plot in a different xpltwin window, either:
  * Run 'xpltwin' directly from the command line, then run xplt. xplt will
    always draw in the most recently created xpltwin window, if more than one
    with the correct name exists.
  * Change the value of the XPLTWIN environment variable, then run xplt.

* To close the xpltwin window, give it the keyboard focus (move the pointer
  into the window;  depending on your window manager settings, you may need
  to click in the window also), then press Escape.  (Q, q, X, and x also
  cause xpltwin to exit cleanly.)  This is preferable to using the window
  manager's controls, since the resources used by xpltwin will be returned
  to the pool properly if you do a clean exit.
*/
	
#include "plt.h"
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#ifdef NOSTDLIB_H
char *getenv();
#else
#include <stdlib.h>
#endif

Display *display;
int screen_num;
Window root, window;
GC gc;
char *name;
Pixmap pixmap;
int pixmapWidth, pixmapHeight;
Atom atom;

main(int argc, char **argv)
{
  static char *geometry;
  static int x, y, xDim, yDim;
  unsigned int display_width, display_height;
  XColor fgcolor;
  XEvent event;
  XWindowAttributes attributes;
  char atomName[100];

  if ((display = XOpenDisplay(NULL)) == NULL) {
    fprintf(stderr, "Can't open display %s\n", XDisplayName(NULL));
    exit(1);
  }

  screen_num = DefaultScreen(display);
  display_width = DisplayWidth(display, screen_num);
  display_height = DisplayHeight(display, screen_num);

  root = RootWindow(display, screen_num);
  gc = DefaultGCOfScreen(ScreenOfDisplay(display, screen_num));

  /* Set the window size and placement hints.  Most window managers will
     accept the size hints, but many will ignore the placement hints. */

  /* -g[eometry] command line option overrides all other hints. */
  if (argc > 2 && argv[1][0] == '-' && argv[1][1] == 'g')
    geometry = argv[2];

  /* XPGEOMETRY environment variable overrides everything else. */
  if (!geometry)
    geometry = getenv("XPGEOMETRY");

  /* X resource database overrides defaults. */
  if (!geometry)
    geometry = XGetDefault(display, "xplt", "Geometry");
  if (!geometry)
    geometry = XGetDefault(display, "xplt", "geometry");

  if (geometry) {
    sscanf(geometry, "%dx%d%d%d", &xDim, &yDim, &x, &y);
    if (xDim <= 0) xDim = display_width/2;
    if (yDim <= 0) yDim = display_height/2;
    if (xDim < 64) xDim = 64;
    if (yDim < 64) yDim = 64;
    if (x < 0) x = display_width - xDim + x;
    if (y < 0) y = display_height - yDim + y;
  }
  else {
    /* These are the defaults, used only if there is no other source of hints.
       If the window manager pays attention, these settings will put the
       window in the center of the screen ... but most window managers will
       ignore the hint. */
    xDim = display_width/2;
    yDim = display_height/2;
    if (xDim > yDim) xDim = yDim;
    else yDim = xDim;
    x = (display_width - xDim)/2;
    y = (display_height - yDim)/2;
  }

  /* Name the window, using the value of the XPLTWIN environment variable if
     set, or the default name ("Plt Window") otherwise. */
  if ((name = getenv("XPLTWIN")) == NULL)
    name = "Plt Window";
  
  /* Run in the background. */
  if (fork())
    exit(0);

  window = XCreateSimpleWindow(display, root, x, y, xDim, yDim, 1,
			       BlackPixel(display, screen_num),
			       WhitePixel(display, screen_num));
  if (window == 0) {
    fprintf(stderr, "Can't create window %s", name);
    exit(1);
  }
    
  XSetStandardProperties(display, window, name, name,
			 (Pixmap)0, (char **)0, 0, (XSizeHints *)0);
    
  sprintf(atomName, "_xpltwin_%d_%s", screen_num, name);
  atom = XInternAtom(display, atomName, False);
  
  XGetWindowAttributes(display, window, &attributes);
  WindowConfigure(attributes.width, attributes.height);
  XMapWindow(display, window);
  XFlush(display);
  XSync(display, NO);
  
  signal(SIGHUP, SIG_IGN);
  signal(SIGINT, SIG_IGN);

  XSelectInput(display, window, ExposureMask|KeyPressMask|StructureNotifyMask);

  while (1) {
    XConfigureEvent *config;
    Drawable *drawables = NULL;
    Atom pltWindowAtom, actualType;
    long nItems, bytesAfter;
    int actualFormat;
    char buffer[10];
    KeySym keysym;
    XComposeStatus compose;
    int count;

    XNextEvent(display, &event);
    switch (event.type) {
    case Expose:			
    case GraphicsExpose:
      WindowExpose((XExposeEvent *)(&event));
      break;
    case ConfigureNotify:
      config = (XConfigureEvent *)(&event);
      WindowConfigure(config->width, config->height);
      break;
    case KeyPress:
      count = XLookupString((XKeyEvent *)&event, buffer, sizeof(buffer),
			    &keysym, &compose);
      if (keysym != XK_Escape && keysym != XK_q && keysym != XK_x &&
	  keysym != XK_Q && keysym != XK_X)
	break;
      /* else fall through to DestroyNotify ... */
    case DestroyNotify:
      sprintf(atomName, "_xpltwin_%d_%s", screen_num, name);
      if ((pltWindowAtom = XInternAtom(display, atomName, True)) != None) {
	XGetWindowProperty(display, root, pltWindowAtom, 0, 2, False,
			   XA_DRAWABLE, &actualType, &actualFormat, &nItems,
			   &bytesAfter, (unsigned char **)&drawables);
  	if (actualType == XA_DRAWABLE && actualFormat == 32 && nItems == 2) {
	  if (window == drawables[0])
	    XDeleteProperty(display, window, pltWindowAtom);
	}
	if (drawables)
	  XFree(drawables);
      }
      XCloseDisplay(display);
      exit(0);
      break;
    }
  }
}

WindowExpose(XExposeEvent *event)
{
  XEvent anotherEvent;

  if (pixmap == 0)
    return;
  
  if (event != 0)
    XCopyArea(display, pixmap, window, gc,
	      event->x, event->y, event->width, event->height,
	      event->x, event->y);
  
  while (XCheckWindowEvent(display,window,ExposureMask,&anotherEvent))
    WindowExpose((XExposeEvent *)(&anotherEvent));
}

WindowConfigure(int width, int height)
{
  Drawable drawable[2];
  Pixmap newPixmap;

  if (pixmapWidth == width && pixmapHeight == height)
    return;
  
  newPixmap = XCreatePixmap(display, window, width, height,
			    DefaultDepth(display,screen_num));
  if (!newPixmap) {
    fprintf(stderr, "Warning: cannot allocate backup storage for plt window");
    return;
  }
  
  if (pixmap) {
    XCopyArea(display, pixmap, newPixmap, gc, 0, 0,
	      pixmapWidth, pixmapHeight, 0, 0);
    XFreePixmap(display, pixmap);
  }
  
  pixmap = newPixmap;
  pixmapWidth = width;
  pixmapHeight = height;
  
  drawable[0] = window;
  drawable[1] = (pixmap == 0 ? window : pixmap);
  XChangeProperty(display, root, atom, XA_DRAWABLE, 32, 
		  PropModeReplace, (char *)drawable, 2);
  XChangeProperty(display, window, atom, XA_DRAWABLE, 32, 
		  PropModeReplace, (char *)drawable, 2);
}

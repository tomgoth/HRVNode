/* file: xw.c		Paul Albrecht		May 1989
 			Last revised:		3 September 2008
X11 driver for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  10 October 2002: fixed solid line style
  23 February 2001: added color support
  20 March 2001: added grey, fill, line mode, line width
  23 March 2001: added font scaling, X resource support
  26 March 2001: added font rotation
  30 March 2001: added LN, CN, RN, label concatenation
  11 April 2001: general cleanup
  24 April 2001: added progressive display in quickplot mode
  10 May 2001: reduced size of exponents in elabel
  19 May 2001: added symbol scaling based on point size for fontgroup f
  3 September 2008: disabled font substitution warnings unless DEBUG is defined
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
#include <signal.h>
#include <sys/time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define X(N)	(int)(xScl*((N)-xMin))
#define Y(N)	(int)(yDim - (int)(yScl*((N)-yMin)))

#define CBGFILL	(Const)15     /* outline polygon with opaque white center */
#define QPFREQ	50	      /* maximum refresh rate (Hz) in quickplot mode;
				 QPFREQ must never be less than 1 or greater
				 than 1000000, but reasonable values are
				 between 10 and the lower of your monitor's
				 refresh rate and 100. */

static Display *dpy;
static int screen;
static Window window;
static Pixmap pixmap;
static GC gc;
static XFontStruct *font;
static XWindowAttributes wa;
static Ptype xLast, yLast;			/* Ptype is int or short */
static Ptype xMin, yMin, xDim, yDim;
static double xScl=1, yScl=1;
static XColor fgcolor;		/* current foreground color (background is
				   always white) */

/* Prototypes of functions defined in this module */
void openpl(void);
void closepl(void);
void erase(void);
void space(Ptype x0, Ptype y0, Ptype x1, Ptype y1);
void label(char *lbl);
void move(Ptype x, Ptype y);
void line(Ptype x0, Ptype y0, Ptype x1, Ptype y1);
void cont(Ptype x, Ptype y);
void plabel(char *lbl, Ptype x, Ptype y, char *just, double angle);
void alabel(char *what, char *lbl, Ptype x, Ptype y);
void elabel(char *what, char *base, char *exp, Ptype x, Ptype y);
void strsize(char *str, Ptype *xsize, Ptype *ysize, double angle);
void scatterplot(PltPtr plt, Ptype x, Ptype y, Ptype yneg, Ptype ypos);
void PolyDef(Ptype x, Ptype y, Const what);
void special(int what, PtrUnion arg0, PtrUnion arg1);

static void update_display(int ignored);
static void point(Ptype x, Ptype y);
static void linewidth(double lw);
static void setgray(double grey);
static void setcolor(char *color);
static void linemod(char *lineMode);
static void OpenPixelWindow(void);
static char *OpenPixelFont(void);
static void setfont(char *name, double ps);
static void CreatePltWindow(char *windowName);
static Boolean PltWindowFind(char *windowName);
static void label0(char *lbl, Ptype x, Ptype y, char *just);
static void label90(char *lbl, Ptype x, Ptype y, char *just);
static void pcont(Ptype x, Ptype y);

static int ErrorHandler() { return (0); }


#define DEF_FONT_H	"-*-helvetica-medium-r-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_HB	"-*-helvetica-bold-r-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_HO	"-*-helvetica-medium-o-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_HBO	"-*-helvetica-bold-o-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_C	"-*-courier-medium-r-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_CB	"-*-courier-bold-r-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_CO	"-*-courier-medium-o-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_CBO	"-*-courier-bold-o-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_T	"-*-times-medium-r-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_TB	"-*-times-bold-r-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_TI	"-*-times-medium-i-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_TBI	"-*-times-bold-i-*-*-%d-*-*-*-*-*-*-*"
#define DEF_FONT_S	"-*-symbol-medium-r-*-*-%d-*-*-*-*-*-*-*"

static struct {
    char *name;
    char *code;
} *f, fonts[] = {
    DEF_FONT_H, "h",
    DEF_FONT_HB, "hb",
    DEF_FONT_HO, "ho",
    DEF_FONT_HBO, "hbo", DEF_FONT_HBO, "hob", 	
    DEF_FONT_C, "c",
    DEF_FONT_CB, "cb",
    DEF_FONT_CO, "co",
    DEF_FONT_CBO, "cbo", DEF_FONT_CBO, "cob",
    DEF_FONT_T, "t", DEF_FONT_T, "tr",
    DEF_FONT_TI, "ti",
    DEF_FONT_TB, "tb",
    DEF_FONT_TBI, "tbi", DEF_FONT_TBI, "tib",
    DEF_FONT_S, "s",
    NULL, NULL };


void openpl(void)
{
  if (!dpy) {
    OpenPixelWindow();

    /* read X resource database and set defaults */
    for (f = fonts; f->code; f++) {
      char *p, resource_name[10];
    
      sprintf(resource_name, "font_%s", f->code);
      if (p = XGetDefault(dpy, "xplt", resource_name))
	f->name = p;
    }
  }
  setfont(DEFAULT_FONT, (double)DEFAULT_PS);
  linemod("solid");
  linewidth(0.);
  setcolor("Black");
  if (Plot.quickPlot)
      update_display(0);	/* enable progressive display */
}

/* update_display is the signal handler for SIGVTALRM (set in
   quickplot mode only to permit progressive display updating).  This
   signal handler can be installed by invoking it with signum = 0. */
static void update_display(int signum)
{
#ifdef SIGVTALRM	/* defined under BSD 4.2+, SVR4, Solaris, Linux ... */
    struct itimerval timer;

    signal(SIGVTALRM, SIG_IGN);
    if (signum == SIGVTALRM)
	closepl();  /* update display (force X server to render the pixmap) */
    timer.it_interval.tv_sec = timer.it_value.tv_sec = 0;
      timer.it_interval.tv_usec = timer.it_value.tv_usec = (long)(1e6/QPFREQ);
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    signal(SIGVTALRM, update_display);
#endif
}

void closepl(void)
{
    XExposeEvent *e;
    XEvent event;

    if (wa.map_state == IsViewable) {
	e = (XExposeEvent *)(&event);	/* Send message to window to redraw */
	e->type = Expose;
	e->send_event = True;
	e->display = dpy;
	e->window = window;
	e->x = e->y = 0;
	e->width = wa.width;
	e->height = wa.height;
	XSendEvent(dpy, window, True, ExposureMask, &event);
    }
    XFlush(dpy);
}

void erase(void)
{
    if (pixmap != window) {
	XSetForeground(dpy, gc, WhitePixel(dpy,screen));
	XFillRectangle(dpy, pixmap, gc, 0, 0, wa.width, wa.height);
	XSetForeground(dpy, gc, fgcolor.pixel=BlackPixel(dpy,screen));
    }

    if(wa.map_state == IsViewable)
	XClearWindow(dpy, window);
}

void space(Ptype x0, Ptype y0, Ptype x1, Ptype y1)
{
    xMin = x0;
    yMin = y0;
    xScl = (double)xDim/(x1-x0+1);
    yScl = (double)yDim/(y1-y0+1);
}

void move(Ptype x, Ptype y)
{
    xLast = x;
    yLast = y;
}

void cont(Ptype x, Ptype y)
{
    XDrawLine(dpy, pixmap, gc, X(xLast), Y(yLast), X(x), Y(y));
    xLast = x;
    yLast = y;
}

void line(Ptype x0, Ptype y0, Ptype x1, Ptype y1)
{
    XDrawLine(dpy, pixmap, gc, X(x0), Y(y0), X(x1), Y(y1));
    xLast = x1;
    yLast = y1;
}

static void point(Ptype x, Ptype y)
{
    XDrawPoint(dpy, pixmap, gc, X(x), Y(y));
    xLast = x;
    yLast = y;
}

void label(char *str)
{
    XDrawString(dpy, pixmap, gc, X(xLast), Y(yLast), str, strlen(str));
}

static void linewidth(double lw)
{
    static double prev_lw = -1;
    XGCValues gcvalues;

    if (lw < 0) {
	err(NO, "Bad line width: %.5lg", lw);
	lw = 0.3;
    }
    if (prev_lw != lw) {
	gcvalues.line_width = (int)(lw/5.0 + 0.5);
	gcvalues.join_style = JoinMiter;
	(void)XChangeGC(dpy, gc, GCLineWidth | GCJoinStyle, &gcvalues);
    }
}

static void setgray(double grey)
{
    char greystring[8];

    if (grey < 0 || grey > 1) {
	err(NO, "Bad grey value: %.5lg", grey);
	grey = 0;
    }
    sprintf(greystring, "grey%02d", (int)(grey*100.0));
    setcolor(greystring);
}

static void setcolor(char *color)
{
    Colormap default_cmap = DefaultColormap(dpy, screen);
  
    if (DefaultDepth(dpy, screen) > 1 &&
	XParseColor(dpy, default_cmap, color, &fgcolor) &&
	XAllocColor(dpy, default_cmap, &fgcolor))
	XSetForeground(dpy, gc, fgcolor.pixel);
    else
	XSetForeground(dpy, gc, fgcolor.pixel = BlackPixel(dpy, screen));
}

static void linemod(char *lineMode)
{
    static char solid[] = { 1 },
	        dotted[] = { 2, 4 },
		shortdashed[] = { 4, 4 },
		longdashed[] = { 8, 8 },
		dotdashed[] = { 2, 4, 6, 4 };

    static char oldLineMode[16];
    XGCValues gcvalues;
    char *dash_list = NULL;
    int n = 0;

    if (strcmp(lineMode,oldLineMode) == 0)
	return;

    if (strcmp(lineMode,"dotted") == 0) {
	dash_list = dotted;
	n = sizeof(dotted);
    }
    else if (strcmp(lineMode,"dotdashed") == 0) {
	dash_list = dotdashed;
	n = sizeof(dotdashed);
    }
    else if (strcmp(lineMode,"shortdashed") == 0) {
	dash_list = shortdashed;
	n = sizeof(shortdashed);
    }
    else if (strcmp(lineMode,"longdashed") == 0) {
	dash_list = longdashed;
	n = sizeof(longdashed);
    }
    else {
	lineMode = "solid";
	dash_list = solid;
	n = sizeof(solid);
    }

    if (n > 1)
	gcvalues.line_style = LineOnOffDash;
    else
	gcvalues.line_style = LineSolid;
    (void)XChangeGC(dpy, gc, GCLineStyle, &gcvalues);
    if (n > 1)
	XSetDashes(dpy, gc, 1, dash_list, n);
    strncpy(oldLineMode, lineMode, sizeof(oldLineMode)-1);
}

void special(int what, PtrUnion arg0, PtrUnion arg1)
{
    switch (what) {
    case SETCOLOR:
	setcolor(arg0.c);
	break;
    case SETFONT:
	setfont(arg0.c, *arg1.d);
	break;
    case SETGRAY:
    case SETGRAYD:
	setgray(*arg0.d);
	break;
    case SETLINEMODE:
	linemod(arg0.c);
	break;
    case SETLINEWIDTH:
	linewidth(*arg0.d);
	break;
    case SETPTERM:
	openpl();	/* defines xDim, yDim and font size information */
	PTERMLookup(arg0.c, "xw");
	p->xfull = p->xsquare = xDim*10;
	p->yfull = yDim*10;
	chwd = p->cw = font->max_bounds.width*10;
	chht = p->ch = (font->max_bounds.ascent+font->max_bounds.descent)*10;
	p->xinches = xDim/75.0;
	p->yinches = yDim/75.0;
	xinch = p->xfull/p->xinches;
	yinch = p->yfull/p->yinches;
	break;
    }
}

static void OpenPixelWindow(void)
{
    char *windowName, *getenv();

    if ((dpy = XOpenDisplay(NULL)) == NULL) {
	fprintf(stderr, "Can't open X server display\n");
	exit(1);
    }
    screen = DefaultScreen(dpy);
    if ((windowName = getenv("XPLTWIN")) == NULL || *windowName == 0)
	windowName = "Plt Window";

    if (!PltWindowFind(windowName)) {
	CreatePltWindow(windowName);
	if (!PltWindowFind(windowName))
	    sleep(3);	/* Time sharing, wait for window to show up */
	if (!PltWindowFind(windowName)) {
	    fprintf(stderr, "Missing xpltwin or something is botched!\n");
	    exit(1);
	}
    }

    XGetWindowAttributes(dpy, window, &wa);
    xDim = wa.width;
    yDim = wa.height;

    gc = XCreateGC(dpy, pixmap, 0L, (XGCValues *)0);
    XSetForeground(dpy, gc, BlackPixel(dpy,screen));

    (void)OpenPixelFont();
}

typedef	struct {
    short xMinDim;
    short yMinDim;
    char *preferredFontName;
    char *acceptableFontName;
    char *pleaseNotThisFontName;
} SizeSpec;

static SizeSpec pointSizes[] = {
    250, 350, "lucidasanstypewriter-8", "6x10", "fixed",
    300, 400, "lucidasanstypewriter-10", "6x12", "fixed",
    550, 450, "lucidasanstypewriter-bold-10","lucidasanstypewriter-10","8x13",
    650, 550, "lucidasanstypewriter-bold-12","lucidasanstypewriter-12","9x15",
    725, 625, "lucidasanstypewriter-bold-14", "10x20", "9x15",
    800, 700, "lucidasanstypewriter-bold-18", "12x24", "9x15",
   9999,9999, ""	 /* dummy, never used */
};				

static char *OpenPixelFont(void)
{
    SizeSpec *pt;
    char *fn;

    for (pt = pointSizes;  pt < ENDP(pointSizes)-1;  pt++) {
	if(pt[1].xMinDim > xDim || pt[1].yMinDim > yDim)
	    break;
    }

    if ((font = XLoadQueryFont(dpy, fn = pt->preferredFontName)) == 0 &&
	(font = XLoadQueryFont(dpy, fn = pt->acceptableFontName)) == 0 &&
	(font = XLoadQueryFont(dpy, fn = pt->pleaseNotThisFontName)) == 0) {
	fprintf(stderr, "%s font not found\n", pt->pleaseNotThisFontName);
	exit (1);
    }
    XSetFont(dpy, gc, font->fid);
    return (fn);
}

static double prev_ps = -1;
static char *prev_name;
static char fontcode[4] = "t";

static void setfont(char *name, double ps)
{
    short n = 0;
    char *fontname = name;

    while (*fontname && n < 3) {
	fontcode[n++] = *(fontname++) | 040;	/* save as lower-case */
	while (*fontname && *(fontname++) != '-')
	    ;
    }
    fontcode[n] = 0;
		
    for (f = fonts; f->code != NULL && strcmp(fontcode, f->code) != 0; f++)
	;

    if (f->code == NULL) {
	err( NO, "Unknown font: %s -- Using: %s", name, DEFAULT_FONT);
	f->name = DEFAULT_FONT;
    }
    if (ps < 0) {
	err( NO, "Bad font size: %.5lg -- Using %g", ps, DEFAULT_PS);
	ps = DEFAULT_PS;
    }

    if (prev_ps != ps || prev_name != f->name) {
	char fullfontname[64];
	int i, points = (int)((prev_ps = ps)*wa.width/670.0+0.5);

	sprintf(fullfontname, prev_name = f->name, points);
	font = XLoadQueryFont(dpy, fullfontname);
	/* If the desired font isn't available in the size we need, try
	   varying the size by up to 5 points in either direction. */
	for (i = 1; !font && i <= 5; i++) {
	    char subfontname[64];
	    sprintf(subfontname, prev_name, points+i);
	    if (!(font = XLoadQueryFont(dpy, subfontname)) && points > i) {
		sprintf(subfontname, prev_name, points-i);
		font = XLoadQueryFont(dpy, subfontname);
	    }
#ifdef DEBUG
	    if (font)
		fprintf(stderr, "substituting %s for %s\n",
			subfontname, fullfontname);
#endif
	}
	if (!font) {
	  char *p;

	  p = OpenPixelFont();
#ifdef DEBUG
	    fprintf(stderr, "substituting %s for %s\n", p, fullfontname);
#endif
	}
	else
	    XSetFont(dpy, gc, font->fid);
	chwd = font->max_bounds.width*10;
	chht = (font->max_bounds.ascent+font->max_bounds.descent)*10;
    }
}

static void CreatePltWindow(char *windowName)
{
    char systemCall[128];

    sprintf(systemCall, "xpltwin '%s'", windowName);
    system(systemCall);
    sleep(1);
}

static Boolean PltWindowFind(char *windowName)
{
    Drawable *drawables;
    Atom pltWindowAtom, actualType;
    long nItems, bytesAfter;
    int actualFormat;
    char atomName[100];

    sprintf(atomName, "_xpltwin_%d_%s", screen, windowName);
    pltWindowAtom = XInternAtom(dpy, atomName, True);
    if (pltWindowAtom == None)
	return(NO);
    XGetWindowProperty(dpy, RootWindow(dpy,screen), pltWindowAtom, 0, 2, False,
		       XA_DRAWABLE, &actualType, &actualFormat, &nItems,
		       &bytesAfter, (unsigned char **)&drawables);
    if (actualType != XA_DRAWABLE || actualFormat != 32 || nItems != 2)
	return(NO);
    window = drawables[0];
    pixmap = drawables[1];
    XFree(drawables);
    actualType = actualFormat = nItems = 0;
    XSetErrorHandler(ErrorHandler);
    XGetWindowProperty(dpy, window, pltWindowAtom, 0, 2, False,
		       XA_DRAWABLE, &actualType, &actualFormat, &nItems,
		       &bytesAfter, (unsigned char **)&drawables);
    XSetErrorHandler((void *)0);
    if (actualType != XA_DRAWABLE || actualFormat != 32 || nItems != 2 ||
	window != drawables[0])
	return(NO);
    XFree(drawables);
    return(YES);
}

static Ptype xax_off, yax_off, xax_roff, yax_roff, xsize, ysize, lastx, lasty;

static void label0(char *lbl, Ptype x, Ptype y, char *just) /* horizontal */
{
    short len;
    Boolean errflag;

    len = strlen(lbl);
    errflag = NO;

    strsize(lbl, &xsize, &ysize, 0.0);
    switch (just[0]) {
    case 'L': break;
    case 'R': x -= xsize; break;
    case 'C': x -= xsize/2; break;
    default:  errflag = YES; break;
    }
    switch (just[1]) {
    case 'T': y -= chht; break;
    case 'N': y -= 2*chht/3; break;
    case 'C': y -= chht/3; break;
    case 'B': break;
    default:  errflag = YES; break;
    }
    if (errflag)
	err(NO, "Unknown justification mode `%s'", just);
    else {
	move(x, y);
	label(lbl);
	lastx = x + xsize;
	lasty = y + chht/3;
    }
}

static void label90(char *lbl, Ptype x, Ptype y, char *just) /* vertical */
{
    Pixmap temppixmap;
    int xx, yy, lwidth, lheight, errflag = NO;

    /* First figure out how large an area is needed for the label. */
    strsize(lbl, &xsize, &ysize, 0.0);
    lwidth = xsize/10;
    lheight = ysize/6;

    switch (just[0]) {
    case 'L': y += xsize; break;
    case 'C': y += xsize/2; break;
    case 'R': break;
    default: errflag = YES; break;
    }
    switch (just[1]) {
    case 'T': x += chht/3; break;
    case 'N': break;
    case 'C': x -= chht/3; break;
    case 'B': x -= 2*chht/3; break;
    default:  errflag = YES; break;
    }
    if (errflag)
	err(NO, "Unknown justification mode '%s'", just);

    lastx = x + ysize/3;
    lasty = y;
    xx = X(x);
    yy = Y(y);

    if (xx < 0) { lheight += xx; xx = 0; }
    if (yy < 0) { lwidth += yy; yy = 0; }
    /* Crop out anything that would fall outside the window. */
    if (lheight > wa.width - xx) lheight = wa.width - xx;
    if (lwidth > wa.height - yy) lwidth = wa.height - yy;

    /* Do nothing if the label would be entirely outside the window. */
    if (lheight <= 0 || lwidth <= 0) return;

    /* Make a pixmap (off-screen drawing area) of the requisite size. */
    if (temppixmap = XCreatePixmap(dpy, window, lwidth, lheight,
				   DefaultDepth(dpy, screen))) {
	int ix, iy;
	XImage *xi, *xo;
	unsigned long whitepixel = WhitePixel(dpy, screen);

	/* Clear the temporary pixmap. */
	XSetForeground(dpy, gc, whitepixel);
	XFillRectangle(dpy, temppixmap, gc, 0, 0, lwidth, lheight);
	XSetForeground(dpy, gc, fgcolor.pixel);

	/* Draw the label into the temporary pixmap (horizontally). */
	XDrawString(dpy, temppixmap, gc, 0, lheight/2, lbl, strlen(lbl));

	/* Create two images (client-side pixmap equivalents).  The
	   first one, xi, gets a copy of the temporary pixmap, with the
	   label drawn into it.  The second one, xo, gets a copy of the
	   contents of the drawing window in the (vertically oriented)
	   region where the label will be drawn. */
	xi = XGetImage(dpy, temppixmap, 0, 0, lwidth, lheight, AllPlanes,
		       XYPixmap);
	xo = XGetImage(dpy, pixmap, xx, yy, lheight, lwidth,
		       AllPlanes, XYPixmap);

	/* Now we copy the non-background pixels from xi into the corresponding
	   locations in xo.  This has the effect of overlaying the label on
	   anything that was already in the specified location. */
	for (ix = 0; ix < lwidth; ix++)
	    for (iy = 0; iy < lheight; iy++) {
		unsigned long pixelvalue;

		if ((pixelvalue = XGetPixel(xi, ix, iy)) != whitepixel)
		    XPutPixel(xo, iy, lwidth-(ix+1), pixelvalue);
	    }

	/* Copy the image back to the main window's pixmap. */
	XPutImage(dpy, pixmap, gc, xo, 0, 0, xx, yy, lheight, lwidth);

	/* Release memory used for the images and for the temporary pixmap. */
	XDestroyImage(xo);
	XDestroyImage(xi);
	XFreePixmap(dpy, temppixmap);
    }
}

void plabel(char *lbl, Ptype x, Ptype y, char *just, double angle)
{
    if (lbl[0] == 0)
	return;
    if (x == DEFAULT) x = lastx;
    if (y == DEFAULT) y = lasty;
    if (fabs(angle) < 1)
	label0(lbl, x, y, just);
    else if (fabs(angle-90) < 1)
	label90(lbl, x, y, just);
    else
	err(NO, "Bad angle %g in for plabel(%s,...)", just, lbl);
}

/********* Output the tick labels and axis label for the axes *************/

void alabel(char *what, char *lbl, Ptype x, Ptype y)
{
    short n;

    if (strcmp(what,"XN") == 0) {
	n = (lbl[0] == '-') ? 2*chwd/3 : 0;
	label0(lbl, x-n, y, "CT");
	xax_off = chht;
    }
    else if (strcmp(what,"YN") == 0) {
	label0(lbl, x-chwd/3, y, "RC");
	strsize(lbl, &xsize, &ysize, 0.0);
	n = xsize + chwd/3;
	if (n > yax_off)
	    yax_off = n;
    }
    else if(strcmp(what,"XNR") == 0) {
	n = (lbl[0] == '-') ? 2*chwd/3 : 0;
	label0(lbl, x+n, y+chht/4, "CB");
	xax_roff = chht;
    }
    else if(strcmp(what,"YNR") == 0) {
	label0(lbl, x+chwd/3, y, "LC");
	strsize(lbl, &xsize, &ysize, 0.0);
	n = xsize + chwd/3;
	if (n > yax_roff)
	    yax_roff = n;
    }
    else if (strcmp(what,"XT") == 0)
	label0(lbl, x, y-xax_off, "CT");
    else if (strcmp(what,"YT") == 0)
	label90(lbl, x-yax_off, y, "CB");
    else if(strcmp(what,"XTR") == 0)
	label0(lbl, x, y+xax_roff, "CB");
    else if (strcmp(what,"YTR") == 0)
	label90(lbl, x+yax_roff, y, "CT");
    else
	err(YES, "Bad call: alabel(%s,...)", what);
}

/****************** Output log tick labels for the axes ********************/

void elabel(char *what, char *base, char *exponent, Ptype x, Ptype y)
{
    Ptype off;
    char *font_name = StringSave(fontcode);
    double base_ps = prev_ps, exp_ps = prev_ps*0.8;

    strsize(base, &xsize, &ysize, 0.0);
    off = xsize;
    setfont(font_name, exp_ps);
    strsize(exponent, &xsize, &ysize, 0.0);
    off += xsize;

    if (strcmp(what,"XE") == 0) {
	x += chwd/3;
	label0(exponent,  x+off/2, y, "RT");
	setfont(font_name, base_ps);
	label0(base, x-off/2, y-chht/3, "LT");
	xax_off = 3*chht/2;
    }
    else if (strcmp(what,"YE") == 0) {
	x -= chwd/3;
	setfont(font_name, base_ps);
	label0(base, x-off, y-chht/3, "LB");
	setfont(font_name, exp_ps);
	label0(exponent, x, y+(chht+3)/6, "RB");
	if (off > yax_off)
	    yax_off = off;
	setfont(font_name, base_ps);
    }
    else if (strcmp(what,"XER") == 0) {
	x += chwd/3;
	label0(exponent,  x+off/2, y+3*chht/4, "RB");
	setfont(font_name, base_ps);
	label0(base, x-off/2, y+chht/4, "LB");
	xax_roff = 3*chht/2;
    }
    else if (strcmp(what,"YER") == 0) {
	x += chwd/2;
	label0(exponent, x+off, y+(chht+3)/6, "RB");
	setfont(font_name, base_ps);
	label0(base, x, y-chht/3, "LB");
	if (off > yax_roff)
	    yax_roff = off;
    }
    else
	err(YES, "Bad call: elabel(%s,...)", what);
}

/****** Scatterplot characters or symbols with or without error bars *******/

static char circle[] = {100, 0, 89, 46, 57, 82, 12, 99, -34, 94, -74, 66,
			-96, 24, -96, -23, -74, -65, -34, -93, 12, -98, 57,
			-81, 89, -45, 100, 0};
static char diamond[] = {0, -15, 12, 0, 0, 15, -12, 0, 0, -15 };
static char triangle[] = {0, 1, -1, -1, 1, -1, 0, 1};
static char utriangle[] = {0, -1, -1, 1, 1, 1, 0, -1};
static char square[] = {-1, -1, -1, 1, 1, 1, 1, -1, -1, -1};

static	struct	syminf {
    char *pts;	/* x,y points specifying the symbol contour */
    short npts;
    double scl;	/* factor for converting *pts to inches */
} *sym, syms[] = {
    circle, sizeof(circle), 0.11,
    utriangle, sizeof(utriangle), 12,
    diamond, sizeof(diamond), 1,
    square, sizeof(square), 10,
    triangle, sizeof(triangle), 12,
    circle, sizeof(circle), 0.11,
    utriangle, sizeof(utriangle), 12,
    diamond, sizeof(diamond), 1,
    square, sizeof(square), 10,
    triangle, sizeof(triangle), 12
};

void scatterplot(PltPtr plt, Ptype x, Ptype y, Ptype yneg, Ptype ypos)
{
    static char lbl[2] = {0, 0};
    double xscl, yscl;
    Ptype n, xd, yd, xdmin, xdmax, ydmin, ydmax;
    int nsyms = sizeof(syms)/sizeof(syms[0]);

    if (plt->symbol) {
	int symno = (plt->pc - '0') % nsyms;

	sym = &syms[symno];
	xdmin = ydmin =  30000;
	xdmax = ydmax = -30000;
	xscl = xinch*sym->scl*xDim*prev_ps/(DEFAULT_PS*100000.0);
	yscl = yinch*sym->scl*xDim*prev_ps/(DEFAULT_PS*100000.0);

	for (n=0;  n < sym->npts;  n+=2) {
	    xd = sym->pts[n];
	    yd = sym->pts[n+1];
	    xd = xscl*xd + (xd < 0 ? -0.5 : 0.5);
	    yd = yscl*yd + (yd < 0 ? -0.5 : 0.5);
	    if (n == 0) PolyDef(x+xd, y+yd, CMOVE);
	    else if (n+2 < sym->npts) PolyDef(x+xd, y+yd, CCONT);
	    else if (symno < nsyms/2) PolyDef(x+xd, y+yd, CBGFILL);
	    else PolyDef(x+xd, y+yd, CFILL);
	    if(xd < xdmin) xdmin = xd;
	    if(xd > xdmax) xdmax = xd;
	    if(yd < ydmin) ydmin = yd;
	    if(yd > ydmax) ydmax = yd;
	}
    }
    else {
	if (plt->pc != '.') {
	    lbl[0] = plt->pc;
	    label0(lbl, x, y, "CC");
	}
	else	point(x, y);

	xdmax = chwd/2;
	xdmin = -xdmax;
	ydmax = chht/3;
	ydmin = -ydmax;
    }

    if (ypos-y >= ydmax) {
	move(x+xdmin, ypos);
	cont(x+xdmax, ypos);
	move(x, ypos);
	cont(x, y+ydmax);
    }
    if (yneg-y <= ydmin) {
	move(x+xdmin, yneg);
	cont(x+xdmax, yneg);
	move(x, yneg);
	cont(x, y+ydmin);
    }
}

int max_vertices = 0, nvertices = 0;
XPoint *vertices = NULL;

static void pcont(Ptype x, Ptype y)
{
    if (nvertices >= max_vertices) {
	XPoint *nv = (XPoint *)realloc(vertices,
				       (max_vertices + 1000)*sizeof(XPoint));
	if (nv) {
	    vertices = nv;
	    max_vertices += 1000;
	}
    }
    if (nvertices < max_vertices) {
	vertices[nvertices].x = X(x);
	vertices[nvertices++].y = Y(y);
    }
}

void PolyDef(Ptype x, Ptype y, Const what)
{
    static int x0, y0;
    XColor savecolor;

    switch (what) {
    case CMOVE:		/* start a new polygon with first vertex at (x,y) */
	if (nvertices > 0) {	/* flush any unfinished polygon */
	    pcont(x0, y0);	/* close the polygon first */
	    XDrawLines(dpy, pixmap, gc, vertices, nvertices, CoordModeOrigin);
	    nvertices = 0;
	}
	move(x, y);
	pcont(x, y);
	x0 = x;
	y0 = y;
	break;
    case CCONT:		/* add a vertex to the current polygon */
	if (oldGrayLevel != PS_WHITE)
	    pcont(x, y);
	break;
    case CBBCONT:
	pcont(x, y);
	break;
    case CFILL:
    case CFILLI:	/* fill polygon with current color */
	pcont(x, y);
	XFillPolygon(dpy, pixmap, gc, vertices, nvertices, Complex,
		     CoordModeOrigin);
	nvertices = 0;
	break;
    case CBGFILL:	/* outline polygon in current color, fill with white */
	pcont(x, y);
	savecolor = fgcolor;
	setcolor("White");
	XFillPolygon(dpy, pixmap, gc, vertices, nvertices, Complex,
		     CoordModeOrigin);
	fgcolor = savecolor;
	XSetForeground(dpy, gc,fgcolor.pixel);
	XDrawLines(dpy, pixmap, gc, vertices, nvertices, CoordModeOrigin);
	nvertices = 0;
	break;
    case CSTROKE:	/* draw the (outline of the) polygon */
	if (oldGrayLevel != PS_WHITE) {
	    pcont(x, y);
	    XDrawLines(dpy, pixmap, gc, vertices, nvertices, CoordModeOrigin);
	    nvertices = 0;
	}
	break;
    case COSTROKE:
	XDrawLines(dpy, pixmap, gc, vertices, nvertices, CoordModeOrigin);
	nvertices = 0;
	break;
    case CBBFILL:
    case CBBFILLI:	/* fill polygon with current color, outline in black */
	pcont(x, y);
	XFillPolygon(dpy, pixmap, gc, vertices, nvertices, Complex,
		     CoordModeOrigin);
	savecolor = fgcolor;
	setcolor("Black");
	pcont(x0, y0);
	XDrawLines(dpy, pixmap, gc, vertices, nvertices, CoordModeOrigin);
	fgcolor = savecolor;
	XSetForeground(dpy, gc, fgcolor.pixel);
	nvertices = 0;
	break;
    default:
	err(YES, "Bad PolyDef() code %d", what);
    }
}

/******* Give the x and y dimension of a string in device units ******/

void strsize(char *str, Ptype *xsize, Ptype *ysize, double angle)
{
    Ptype width =  XTextWidth(font, str, strlen(str))*10, height = chht;
    if (fabs(angle) < 1) {
	*xsize = width;
	*ysize = height;
    }
    else if (fabs(angle-90) < 1) {
	*xsize = height;
	*ysize = width;
    }
    else
	err(NO, "Bad angle %lg in for strsize(%s,...)", angle, str);
}

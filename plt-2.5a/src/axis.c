/* file: axis.c		Paul Albrecht		September 1984
			Last revised:	      14 November 2002
Axis and grid functions for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  12 April 2001: "-g ygrid -s x" now works; general cleanup
  21 October 2002: moved formerly global variables here from plt.h
  14 November 2002: fixed missing cast in azmem call
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

#define	NO_TICK		0
#define	IN_OUT_TICK	01
#define	IN_TICK		02
#define	OUT_TICK		03
#define	IN_OUT_MASK	03
#define	GRID		04
#define	SYMMETRIC	010
#define	SUB_GRID	020
#define	XGRID		040
#define	YGRID		0100

typedef	struct {
    AxisPtr a;
    double tick;
    double scl;
    char *lbl;
} *TickPtr;

static int gtype = OUT_TICK;

/* Prototypes of functions defined in this module */
void SetupAxes(void);
void AxisInit(Mode mode);
void XAxisDraw(void);
void YAxisDraw(void);
void TickDef(AxisPtr a, double tick, char *lbl, double scl, Boolean override);

static void AxisInitOne(AxisPtr a);
static void MinmaxSetup(AxisPtr a);
static void AxisSetup(AxisPtr a, AxisPtr oa);
static void LinearTickSetup(AxisPtr a);
static void LogTickSetup(AxisPtr a);
static void GridSetup(void);
static void GridDraw(AxisPtr a, AxisPtr oa);
static Ptype TickDrawAxis(AxisPtr a);
static int TickDraw(TickPtr t, Ptype ticklen);
static void TickLabel(TickPtr t, int off);

/* Public functions */

void SetupAxes(void)
{
    char xtitle[200];

    xwmin = p->xfull * ((xwmins == DEFAULT) ? p->xwmins : xwmins);
    xwmax = p->xfull * ((xwmaxs == DEFAULT) ? p->xwmaxs : xwmaxs);
    ywmin = p->yfull * ((ywmins == DEFAULT) ? p->ywmins : ywmins);
    ywmax = p->yfull * ((ywmaxs == DEFAULT) ? p->ywmaxs : ywmaxs);

    xfract = (double)(xwmax - xwmin)/p->xfull/(p->xwmaxs-p->xwmins);
    yfract = (double)(ywmax - ywmin)/p->yfull/(p->ywmaxs-p->ywmins);
    if (xfract > yfract) yfract = 0.1*xfract + 0.9*yfract;
    else	         xfract = 0.9*xfract + 0.1*yfract;

    MinmaxSetup(&xa);
    MinmaxSetup(&ya);

    xa.scl = (xwmax-xwmin)/(xa.max-xa.min);
    xa.off = xwmin - xa.min*xa.scl + 0.5;
    ya.scl = (ywmax-ywmin)/(ya.max-ya.min);
    ya.off = ywmin - ya.min*ya.scl + 0.5;

    AxisSetup(&xa, &ya);
    AxisSetup(&ya, &xa);
    GridSetup();

    if (xa.lbl == 0) {
	sprintf(xtitle, "%sX is %.5g to %.5g,  %sY is %.5g to %.5g",
		xa.logflg ? "Log " : "", xmin, xmax,
		ya.logflg ? "Log " : "", ymin, ymax);
	xa.lbl = StringSave(xtitle);
    }
}

void AxisInit(Mode mode)
{
    xa.name = 'x';
    AxisInitOne(&xa);
    ya.name = 'y';
    AxisInitOne(&ya);
    xmin = ymin = FHUGE;
    xmax = ymax = -FHUGE;
    xwmins = xwmaxs = ywmins = ywmaxs = FDEFAULT;
    gridfg  = "P12,W.1,Ldot";
    gridtype = "out";
}

void XAxisDraw(void)
{
    Ptype min, max, off, off2, xsize, ysize;

    if ((gtype & (GRID|XGRID)) && (xa.mode & GRIDMARKS))
	GridDraw(&xa, &ya);

    FontGroupSelect("a", xa.numfg);
    if (xa.mode & AXIS) {
	min = X(xa.min);
	max = X(xa.max);
	line(min, xa.lo, max, xa.lo);
	if (gtype & SYMMETRIC)
	    line(min, xa.hi, max, xa.hi);
    }

    off = TickDrawAxis(&xa);
    if (xa.lbl && (xa.mode & TITLES)) {
	strsize(xa.lbl, &xsize, &ysize, 0.0);
	off2 = yfract*(yinch/4 + ysize/2) - ysize/3;
	off -= xa.rev ? -off2 : off2;
	FontGroupSelect("t", xa.lblfg);
	alabel(xa.rev ? "XTR" : "XT", xa.lbl, (xwmin+xwmax)/2, off);
    }
}

void YAxisDraw(void)
{
    Ptype min, max, off, off2, xsize, ysize;

    if ((gtype & (GRID|YGRID)) && (ya.mode & GRIDMARKS))
	GridDraw(&ya, &xa);

    FontGroupSelect("a", ya.numfg);
    if (ya.mode & AXIS) {
	min = Y(ya.min);
	max = Y(ya.max);
	line(ya.lo, min, ya.lo, max);
	if (gtype & SYMMETRIC)
	    line( ya.hi, min, ya.hi, max);
    }

    off = TickDrawAxis(&ya);
    if (ya.lbl && (ya.mode & TITLES)) {
	strsize(ya.lbl, &xsize, &ysize, 90.0);
	off2 = xfract*(xinch/4 + xsize/2);
	off -= ya.rev ? -off2 : off2;
	FontGroupSelect("t", ya.lblfg);
	alabel(ya.rev ? "YTR" : "YT", ya.lbl, off, (ywmin+ywmax)/2);
    }
}

static TickPtr ticks;
static Uint maxTicks, nTicks;

void TickDef(AxisPtr a, double tick, char *lbl, double scl, Boolean override)
{
    TickPtr t;
    double delta, tmp;
    short n;

    if (a->min != DEFAULT && a->max != DEFAULT)
	delta = (a->max - a->min)/1e3;
    else {
	tmp = fabs(tick);
	delta = (tmp < 1e-3) ? 1e-3 : tmp/1e3;
    }

    for (n = 0; n < nTicks; n++) {
	t = &ticks[n];
	tmp = fabs(tick - t->tick);
	if (a->name == (t->a)->name && tmp < delta)
	    break;
    }

    if (n == nTicks) {
	if (nTicks == maxTicks)
	    ticks = (TickPtr)azmem(ticks, (size_t *)&maxTicks, 12,
				   sizeof(*ticks));
	t = &ticks[nTicks++];
    }
    else {
	if (!override)
	    return;
    }

    t->a = a;
    t->tick = tick;
    t->lbl = (lbl == 0) ? lbl : StringSave(lbl);
    t->scl = (scl == DEFAULT) ? 1.0 : scl;
    if (a->rev)
	t->scl = -(t->scl);
}

/* Private functions (used only within this module) */

static void AxisInitOne(AxisPtr a)
{
    a->min = a->max = a->cr = FDEFAULT;
    a->aoff = a->mlt = a->tick = a->tmark = FDEFAULT;
    a->tscl = 1;
    a->skp = DEFAULT;
    a->pfm = a->lbl = a->base = 0;	
    a->mode = 077777;
    a->logflg = a->rev = NO;
    a->scl = a->off = 0;
    a->acchi = a->acclo = 0.2;
    a->numfg = a->lblfg = a->extra = 0;
    a->lo = a->hi = 0;
    a->this = (a->name == 'x') ? X : Y;
    a->other = (a->name == 'x') ? Y : X;
}


static void MinmaxSetup(AxisPtr a)
{
    if (a->min != DEFAULT && a->max != DEFAULT && a->min > a->max)
	err(YES, "%c axis reversed", a->name);
    if (a->min != DEFAULT)
	a->acchi = 0;
    if (a->max != DEFAULT)
	a->acclo = 0;
    if (a->logflg)
	LogTickSetup(a);
    else {
	LinearTickLogic(a);
	LinearTickSetup(a);
    }
}

static void AxisSetup(AxisPtr a, AxisPtr oa)
{
    if (a->cr == DEFAULT) {
	a->cr = a->rev ? oa->max : oa->min;
	if (a->aoff != DEFAULT)
	    a->cr -= (a->rev ? -1 : 1)*a->aoff*(oa->max-oa->min);
    }
    a->lo = (*a->other)(a->cr);
    a->hi = (*a->other)(oa->max+oa->min-a->cr);
}

static void LinearTickSetup(AxisPtr a)
{
    double tick, tend, tscl = a->tscl;
    short k, n;
    char *tlbl = 0;

    switch (a->mode & (TICKMARKS|TICKNUMS)) {
    case NOTHING: return;
    case TICKMARKS: tlbl = ""; break;
    case TICKNUMS: tscl = 0; break;
    case TICKMARKS+TICKNUMS: break;
    }
    n = ceil((a->min - a->tmark)/a->tick - 0.0001);
    tick = a->tmark + n*a->tick;
    tend = a->max + 0.001*a->tick;
    for (k = 0; tick < tend; k++) {
	TickDef(a, tick, ((k+n)%a->skp) ? "" : tlbl, tscl, NO);
	tick += a->tick;
    }
}

static void LogTickSetup(AxisPtr a)
{
    double tick, tbeg, tend, tscl, sfract, ftmp, subtick[20];
    int k, n, nsubtick;
    char *tlbl;

    if (a->logflg && a->base == 0)
	a->base = "10";
    if(a->min == DEFAULT)
	a->min = floor((a->name == 'x' ? xmin : ymin) + 0.001);
    if(a->max == DEFAULT)
	a->max = ceil((a->name == 'x' ? xmax : ymax) - 0.001);
    if(a->tmark == DEFAULT)
	a->tmark = 0;
    if(a->pfm == 0)
	a->pfm = "%g";

    tlbl = 0;
    tscl = a->tscl;	
    switch (a->mode & (TICKMARKS|TICKNUMS)) {
    case NOTHING: return;
    case TICKMARKS: tlbl = ""; break;
    case TICKNUMS: tscl = 0; break;
    case TICKMARKS+TICKNUMS: break;
    }

    n  = ceil((a->min - a->tmark) - 0.0001);
    tbeg = a->tmark + n;
    tend = a->max;
    sfract = ((a->name == 'x') ? (double)(xwmax-xwmin)/p->xfull :
	      (double)(ywmax-ywmin)/p->yfull)/(tend-tbeg);

    if (ticklogic)
	err(NO, "tbeg, tend, sfract: %g %g %g\n", tbeg, tend, sfract);
    if (a->extra == NULL)
	a->extra = (sfract < 0.07) ? "n" : "y";
    nsubtick = 0;
    if (a->extra[0] == 'y' && (n=atoi(a->base)) > 2) {
	if (a->skp == DEFAULT)
	    a->skp = 1 + 0.05/sfract;
	ftmp = 1/log((double)n);
	for (k = 2; k < n; k++)
	    subtick[nsubtick++] = log((double)k)*ftmp;
    }
    else if (a->skp == DEFAULT)
	a->skp = 1 + 0.05/sfract;

    for (tick = tbeg; tick <= tend; tick += a->skp) {
	TickDef(a, tick, tlbl, tscl, NO);
	for (n = 0; n < a->skp;  n++) {
	    if (n > 0 && tick+n-.01 < tend)
		TickDef(a, tick+n, "", tscl, NO);
	    if (a->extra[0] == 'y' && tick+n+0.99 < tend) {
		for (k = 0; k < nsubtick; k++)
		    TickDef(a, tick+n+subtick[k], "", tscl, NO);
	    }
	}
    }
}

static void GridSetup(void)
{
    char *spec;

    if (*gridtype >= '0' && *gridtype <= '9') {
	gtype = LongNum(gridtype);	/* old-time compatibility */
	return;
    }

    while (*gridtype) {
	if (!getstr(&gridtype, &spec))
	    continue;			

	if (strcmp(spec,"out") == 0)
	    gtype = (gtype & ~IN_OUT_MASK) + OUT_TICK;
	else if(strcmp(spec,"in") == 0)
	    gtype = (gtype & ~IN_OUT_MASK) + IN_TICK;
	else if(strcmp(spec,"out") == 0)
	    gtype = (gtype & ~IN_OUT_MASK) + OUT_TICK;
	else if(strcmp(spec,"both") == 0)
	    gtype = (gtype & ~IN_OUT_MASK) + IN_OUT_TICK;
	else if(strncmp(spec,"no",2) == 0)
	    gtype = (gtype & ~IN_OUT_MASK);
	else if(strncmp(spec,"sym",3) == 0)
	    gtype |= SYMMETRIC;
	else if(strcmp(spec,"grid") == 0)
	    gtype |= GRID;
	else if(strcmp(spec,"xgrid") == 0)
	    gtype |= XGRID;
	else if(strcmp(spec,"ygrid") == 0)
	    gtype |= YGRID;
	else if(strncmp(spec,"sub",3) == 0)
	    gtype |= SUB_GRID;
	else
	    err(YES, "Unrecognized tick spec `%s'", spec);
	while(*gridtype == ',')
	    gridtype++;
    }
}

static void GridDraw(AxisPtr a, AxisPtr oa)
{
    TickPtr t;
    Ptype n, itick, min, max;

    FontGroupSelect("a", gridfg);
    min = (*oa->this)(oa->min);
    max = (*oa->this)(oa->max);

    for (n = 0; n < nTicks; n++) {
	t = &ticks[n];
	if ((t->a)->name == a->name &&
	    (t->lbl == 0 || *t->lbl != 0 || (gtype & SUB_GRID))) {
	    itick = (*a->this)(t->tick);
	    if (a->name == 'x')
		line(itick, min, itick, max);
	    else
		line(min, itick, max, itick);
	}
    }
}

static Ptype TickDrawAxis(AxisPtr a)
{
    TickPtr t;
    double fudge;
    Ptype off, toff, ticklen, ticksize;
    short n;
    Boolean lbl;

    fudge = 0.3 * (double)chht/yinch * (3+xfract+yfract); 
    ticksize = fudge * ((a->name == 'x') ? xinch : yinch)/5.0;
    toff = a->lo;

    for (n = 0, t = ticks; n < nTicks; n++, t++) {
	if ((t->a)->name == a->name) {
	    lbl = !(t->lbl && t->lbl[0] == 0);
	    ticklen = (lbl ? (3*ticksize+1)/2 : ticksize) * t->scl;
	    off = TickDraw(t, ticklen);
	    if (lbl)
		TickLabel(t, off);
	    if (a->rev ? (toff < off) : (toff > off))
		toff = off;
	}
    }
    return (toff);
}

#define		XYTICK(A,B) 	(isx ? line(itick, A, itick, off=B)	\
					: line(A, itick, off=B, itick))

static int TickDraw(TickPtr t, Ptype ticklen)
{
    AxisPtr	a;
    int isx, itick, off;

    a = t->a;
    itick = (*a->this)(t->tick);
    isx  = (a->name == 'x');

    if (gtype & SYMMETRIC)
	switch (gtype & IN_OUT_MASK) {
	case NO_TICK: break;
	case IN_TICK: XYTICK(a->hi, a->hi-ticklen); break;
	case OUT_TICK: XYTICK(a->hi+ticklen, a->hi); break;
	case IN_OUT_TICK: XYTICK(a->hi+ticklen, a->hi-ticklen); break;
	}

    switch (gtype & IN_OUT_MASK) {
    case NO_TICK: off = a->lo; break;
    case IN_TICK: XYTICK(a->lo+ticklen, a->lo); break;
    case OUT_TICK: XYTICK(a->lo, a->lo-ticklen); break;
    case IN_OUT_TICK: XYTICK(a->lo+ticklen, a->lo-ticklen); break;
    }
    return (off);
}

static void TickLabel(TickPtr t, int off)
{
    AxisPtr a;
    Boolean rev;
    char str[60];

    a = t->a;
    if (t->lbl == 0) {
	double tmp = fabs(t->tick);
	if((a->max-a->min)*1e-6 > tmp)
	    t->tick = 0;
	sprintf(str, a->pfm, t->tick);
	t->lbl = StringSave(str);
    }

    rev = (t->scl < 0);
    if (a->logflg) {
	if (a->name == 'x')
	    elabel(rev ? "XER" : "XE", a->base, t->lbl, X(t->tick), off);
	else
	    elabel(rev ? "YER" : "YE", a->base, t->lbl, off, Y(t->tick));
    }
    else {
	if (a->name == 'x')
	    alabel(rev ? "XNR" : "XN", t->lbl, X(t->tick), off);
	else
	    alabel(rev ? "YNR" : "YN", t->lbl, off, Y(t->tick));
    }
}

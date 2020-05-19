/*	plt/axis.c		Paul Albrecht		Sept 1984
 
	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	13 June 1995 (GBM)
	EMACS_MODES:	tabstop=4
*/

#include	"plt.h"
#include	"axis.h"
#include	"figs.h"

static void MinmaxSetup();
static void AxisSetup();
static void LinearTicSetup();
static void GridSetup();
static Ptype TicDrawAxis();
static void TicLabel();

#define		NO_TIC		0
#define		IN_OUT_TIC	01
#define		IN_TIC		02
#define		OUT_TIC		03
#define		IN_OUT_MASK	03
#define		GRID		04
#define		SYMMETRIC	010
#define		SUB_GRID	020
#define		XGRID		040
#define		YGRID		0100

static	int	gtype = OUT_TIC;


static void	PROTO( AxisInitOne, (AxisPtr,Mode) );


void	AxisInit( mode )
Mode	mode;
{
	xa.name = 'x';
	AxisInitOne( &xa );
	ya.name = 'y';
	AxisInitOne( &ya );

	xmin = ymin = FHUGE;
	xmax = ymax = -FHUGE;
	xwmins = xwmaxs = ywmins = ywmaxs = FDEFAULT;
	gridfg  = "P12,W.1,Ldot";
	gridtype = "out";
}

static void	AxisInitOne( a, mode )
AxisPtr	a;
Mode	mode;
{
	a->min = a->max = a->cr = FDEFAULT;
	a->aoff = a->mlt = a->tic = a->tmark = FDEFAULT;
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


/*********************************************************************/


SetupAxes()
{
char	xtitle[200];

	xwmin = p->xfull * ((xwmins == DEFAULT) ? p->xwmins : xwmins);
	xwmax = p->xfull * ((xwmaxs == DEFAULT) ? p->xwmaxs : xwmaxs);
	ywmin = p->yfull * ((ywmins == DEFAULT) ? p->ywmins : ywmins);
	ywmax = p->yfull * ((ywmaxs == DEFAULT) ? p->ywmaxs : ywmaxs);

	xfract = (double)(xwmax - xwmin)/p->xfull/(p->xwmaxs-p->xwmins);
	yfract = (double)(ywmax - ywmin)/p->yfull/(p->ywmaxs-p->ywmins);
	if( xfract > yfract )
		yfract = 0.1*xfract + 0.9*yfract;
	else	xfract = 0.9*xfract + 0.1*yfract;

	MinmaxSetup( &xa );
	MinmaxSetup( &ya );

	xa.scl = (xwmax-xwmin)/(xa.max-xa.min);
	xa.off = xwmin - xa.min*xa.scl + 0.5;
	ya.scl = (ywmax-ywmin)/(ya.max-ya.min);
	ya.off = ywmin - ya.min*ya.scl + 0.5;

	AxisSetup( &xa, &ya );
	AxisSetup( &ya, &xa );
	GridSetup();

	if( xa.lbl == 0 ) {
		sprintf( xtitle, "%sX is %.5g to %.5g,  %sY is %.5g to %.5g",
		xa.logflg ? "Log " : "", xmin, xmax,
		ya.logflg ? "Log " : "", ymin, ymax );
		xa.lbl = StringSave( xtitle );
	}
}


static void MinmaxSetup( a )
AxisPtr	a;
{
	if( a->min != DEFAULT && a->max != DEFAULT && a->min > a->max )
		estop( "%c axis reversed", a->name );

	if( a->min != DEFAULT )
		a->acchi = 0;
	if( a->max != DEFAULT )
		a->acclo = 0;

	if( a->logflg )
		LogTicSetup( a );
	else {
		LinearTicLogic( a );
		LinearTicSetup( a );
	}
}


static void AxisSetup( a, oa )
AxisPtr	a, oa;
{
	if( a->cr == DEFAULT ) {
		a->cr = a->rev ? oa->max : oa->min;
		if( a->aoff != DEFAULT )
			a->cr -= (a->rev ? -1 : 1)*a->aoff*(oa->max-oa->min);
	}
	a->lo = (*a->other)(a->cr);
	a->hi = (*a->other)(oa->max+oa->min-a->cr);
}


static void LinearTicSetup( a )
AxisPtr	a;
{
double	tic, tend, tscl;
short	k, n;
char	*tlbl;

	tlbl = 0;
	tscl = a->tscl;	
	switch( a->mode & (TICMARKS|TICNUMS) ) {
		case NOTHING:
			return;
		case TICMARKS:
			tlbl = "";
			break;
		case TICNUMS:
			tscl = 0;
		case TICMARKS+TICNUMS:
			break;
	}

	n = ceil( (a->min - a->tmark)/a->tic - 0.0001 );
	tic  = a->tmark + n*a->tic;
	tend = a->max + 0.001*a->tic;

	for( k=0;  tic < tend;  k++ ) {
		TicDef( a, tic, ((k+n)%a->skp) ? "" : tlbl, tscl, NO );
		tic += a->tic;
	}
}

LogTicSetup( a )
AxisPtr	a;
{
double	tic, tbeg, tend, tscl, sfract, ftmp, subtic[20];
int		k, n, nsubtic;
char	*tlbl;

	if( a->logflg && a->base == 0  )
		a->base = "10";
	if( a->min == DEFAULT )
		a->min = floor((a->name == 'x' ? xmin : ymin) + 0.001 );
	if( a->max == DEFAULT )
		a->max = ceil( (a->name == 'x' ? xmax : ymax) - 0.001 );
	if( a->tmark == DEFAULT )
		a->tmark = 0;
	if( a->pfm == 0 )
		a->pfm = "%g";

	tlbl = 0;
	tscl = a->tscl;	
	switch( a->mode & (TICMARKS|TICNUMS) ) {
		case NOTHING:
			return;
		case TICMARKS:
			tlbl = "";
			break;
		case TICNUMS:
			tscl = 0;
		case TICMARKS+TICNUMS:
			break;
	}

	n  = ceil( (a->min - a->tmark) - 0.0001 );
	tbeg = a->tmark + n;
	tend = a->max;
	sfract = ((a->name == 'x') ? (double)(xwmax-xwmin)/p->xfull :
		(double)(ywmax-ywmin)/p->yfull)/(tend-tbeg);

	if( ticlogic )
		eprintf( "tbeg, tend, sfract: %g %g %g\n", tbeg, tend, sfract );


	if( a->extra == NULLS )
		a->extra = (sfract < 0.07) ? "n" : "y";

	nsubtic = 0;
	if( a->extra[0] == 'y' && (n=atoi(a->base)) > 2 ) {
		if( a->skp == DEFAULT )
			a->skp = 1 + 0.05/sfract;
		ftmp = 1/log( (double)n );
		for( k=2;  k < n;  k++ )
			subtic[nsubtic++] = log((double)k)*ftmp;
	}
	else {
		if( a->skp == DEFAULT )
			a->skp = 1 + 0.05/sfract;
	}

	for( tic=tbeg;  tic <= tend;  tic += a->skp ) {
		TicDef( a, tic, tlbl, tscl, NO );
		for( n=0; n < a->skp;  n++ ) {
			if( n > 0 && tic+n-.01 < tend )
				TicDef( a, tic+n, "", tscl, NO );
			if( a->extra[0] == 'y' && tic+n+0.99 < tend ) {
				for( k=0;  k < nsubtic;  k++ )
					TicDef( a, tic+n+subtic[k], "", tscl, NO );
			}
		}
	}
}


TicDef( a, tic, lbl, scl, override )
AxisPtr	a;
char	*lbl;
double	tic, scl;
Boolean	override;
{
TicPtr	t;
double	delta, tmp;
short	n;
	
	if( a->min != DEFAULT && a->max != DEFAULT )
		delta = (a->max - a->min)/1e3;
	else {
		tmp = fabs(tic);
		delta = (tmp < 1e-3) ? 1e-3 : tmp/1e3;
	}

	for( n=0;  n < ntics;  n++ ) {
		t = &tics[n];
		tmp = fabs(tic - t->tic);
		if( a->name == (t->a)->name && tmp < delta )
			break;
	}

	if( n == ntics ) {
		if( ntics == maxtics )
			tics = (TicPtr)azmem(tics,&maxtics,12,sizeof(*tics));
		t = &tics[ntics++];
	}
	else {
		if( !override )
			return;
	}

	t->a = a;
	t->tic = tic;
	t->lbl = (lbl == 0) ? lbl : StringSave(lbl);
	t->scl = (scl == DEFAULT) ? 1.0 : scl;
	if( a->rev )
		t->scl = -(t->scl);
}


static void GridSetup()
{
char	*spec;

	if( *gridtype >= '0' && *gridtype <= '9' ) {
		gtype = LongNum(gridtype);	/* old-time compatibility */
		return;
	}

	while( *gridtype ) {
		if( !getstr(&gridtype, &spec) )
			continue;			

		if( strcmp(spec,"out") == 0 )
			gtype = (gtype & ~IN_OUT_MASK) + OUT_TIC;
		else	if( strcmp(spec,"in") == 0 )
				gtype = (gtype & ~IN_OUT_MASK) + IN_TIC;
		else	if( strcmp(spec,"out") == 0 )
				gtype = (gtype & ~IN_OUT_MASK) + OUT_TIC;
		else	if( strcmp(spec,"both") == 0 )
				gtype = (gtype & ~IN_OUT_MASK) + IN_OUT_TIC;
		else	if( strncmp(spec,"no",2) == 0 )
				gtype = (gtype & ~IN_OUT_MASK);
		else	if( strncmp(spec,"sym",3) == 0 )
				gtype |= SYMMETRIC;
		else	if( strcmp(spec,"grid") == 0 )
				gtype |= GRID;
		else	if( strcmp(spec,"xgrid") == 0 )
				gtype |= XGRID;
		else	if( strcmp(spec,"ygrid") == 0 )
				gtype |= YGRID;
		else	if( strncmp(spec,"sub",3) == 0 )
				gtype |= SUB_GRID;
		else	err( YES, "Unrecognized tic spec `%s'", spec );
		while( *gridtype == ',' )
			gridtype++;
	}
}


/************************************************************************/

XAxisDraw()
{
Ptype	min, max, off, off2, xsize, ysize;

	if( (gtype & (GRID|XGRID)) && (xa.mode & GRIDMARKS) )
		GridDraw( &xa, &ya );

	FontGroupSelect( "a", xa.numfg );
	if( xa.mode & AXIS ) {
		min = X(xa.min);
		max = X(xa.max);
		line( min, xa.lo, max, xa.lo );
		if( gtype & SYMMETRIC )
			line( min, xa.hi, max, xa.hi );
	}

	off = TicDrawAxis( &xa );
	if( xa.lbl && (xa.mode & TITLES) ) {
		strsize( xa.lbl, &xsize, &ysize, 0.0 );
		off2 = yfract*(yinch/4 + ysize/2) - ysize/3;
		off -= xa.rev ? -off2 : off2;
		FontGroupSelect( "t", xa.lblfg );
		alabel( xa.rev ? "XTR" : "XT", xa.lbl, (xwmin+xwmax)/2, off );
	}
}


YAxisDraw()
{
Ptype	min, max, off, off2, xsize, ysize;

	if( (gtype & (GRID|YGRID)) && (xa.mode & GRIDMARKS) )
		GridDraw( &ya, &xa );

	FontGroupSelect( "a", ya.numfg );
	if( ya.mode & AXIS ) {
		min = Y(ya.min);
		max = Y(ya.max);
		line( ya.lo, min, ya.lo, max );
		if( gtype & SYMMETRIC )
			line(  ya.hi, min, ya.hi, max );
	}

	off = TicDrawAxis( &ya );
	if( ya.lbl && (ya.mode & TITLES) ) {
		strsize( ya.lbl, &xsize, &ysize, 90.0 );
		off2 = xfract*(xinch/4 + xsize/2);
		off -= ya.rev ? -off2 : off2;
		FontGroupSelect( "t", ya.lblfg );
		alabel( ya.rev ? "YTR" : "YT", ya.lbl, off, (ywmin+ywmax)/2 );
	}
}


GridDraw( a, oa )
AxisPtr	a, oa;
{
TicPtr	t;
Ptype	n, itic, min, max;

	FontGroupSelect( "a", gridfg );
	min = (*oa->this)(oa->min);
	max = (*oa->this)(oa->max);

	for( n=0;  n < ntics;  n++ ) {
		t = &tics[n];
		if( (t->a)->name == a->name && (t->lbl == 0 ||
			*t->lbl != 0 || (gtype & SUB_GRID)) ) {
			itic = (*a->this)(t->tic);
			if( a->name == 'x' )
				line( itic, min, itic, max );
			else
				line( min, itic, max, itic );
		}
	}
}


static Ptype TicDrawAxis( a )
AxisPtr	a;
{
TicPtr	t;
double	fudge;
Ptype	off, toff, ticlen, ticsize;
short	n;
Boolean lbl;

	fudge = 0.3 * (double)chht/yinch * (3+xfract+yfract); 
	ticsize = fudge * ((a->name == 'x') ? xinch : yinch)/5.0;
	toff = a->lo;

	for( n=0, t=tics;  n < ntics;  n++, t++ ) {
		if( (t->a)->name == a->name ) {
			lbl = !(t->lbl && t->lbl[0] == 0);
			ticlen = (lbl ? (3*ticsize+1)/2 : ticsize) * t->scl;
			off = TicDraw( t, ticlen );
			if( lbl )
				TicLabel( t, off );
			if( a->rev ? (toff < off) : (toff > off) )
				toff = off;
	 	}
	}
	return( toff );
}

/***********************************************************************/

#define		XYTIC(A,B) 	( isx ? line(itic, A, itic, off=B)	\
					: line(A, itic, off=B, itic) )

TicDraw( t, ticlen )
TicPtr	t;
Ptype	ticlen;
{
AxisPtr	a;
int	isx, itic, off;

	a = t->a;
	itic = (*a->this)(t->tic);
	isx  = (a->name == 'x');

	if( gtype & SYMMETRIC )
	   switch( gtype & IN_OUT_MASK ) {
		case NO_TIC:
			break;
		case IN_TIC:
			XYTIC(a->hi, a->hi-ticlen);		break;
		case OUT_TIC:
			XYTIC(a->hi+ticlen, a->hi);		break;
		case IN_OUT_TIC:
			XYTIC(a->hi+ticlen, a->hi-ticlen);	break;
	}

	switch( gtype & IN_OUT_MASK ) {
		case NO_TIC:
			off = a->lo;				break;
		case IN_TIC:
			XYTIC(a->lo+ticlen, a->lo);		break;
		case OUT_TIC:
			XYTIC(a->lo, a->lo-ticlen);		break;
		case IN_OUT_TIC:
			XYTIC(a->lo+ticlen, a->lo-ticlen);	break;
	}
	return( off );
}


static void TicLabel( t, off )
TicPtr	t;
{
AxisPtr	a;
Boolean	rev;
double	tmp;		/* Done this way to overcome a 386 compiler bug */
char	str[60];

	a = t->a;
	if( t->lbl == 0 ) {
		tmp = fabs(t->tic);
		if( (a->max-a->min)*1e-6 > tmp )
			t->tic = 0;
		sprintf( str, a->pfm, t->tic );
		t->lbl = StringSave(str);
	}

	rev = (t->scl < 0);
	if( a->logflg ) {
		if( a->name == 'x' )
			elabel( rev ? "XER" : "XE", a->base, t->lbl, X(t->tic), off );
		else
			elabel( rev ? "YER" : "YE", a->base, t->lbl, off, Y(t->tic) );
	}
	else {
		if( a->name == 'x' )
			alabel( rev ? "XNR" : "XN", t->lbl, X(t->tic), off );
		else
			alabel( rev ? "YNR" : "YN", t->lbl, off, Y(t->tic) );
	}
}

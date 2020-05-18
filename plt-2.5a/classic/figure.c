/*	plt/figure.c		Paul Albrecht		Feb 1988

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4
*/


#include	"plt.h"
#include	"figs.h"
#include	"plot.h"
#include	"axis.h"



void	FigureInit( mode )
Mode	mode;
{
	Figure.xlPos = Figure.ylPos = FDEFAULT;
	Figure.xlDel = Figure.xlBoxScl = FDEFAULT;
}


/***************************************************************************/


FigureDef( type, coord, x0, y0, x1, y1, fgName )
char	type, *fgName;
double	x0, y0, x1, y1;
Const	coord;
{
FigPtr	f;

	if( Figure.nFigs == Figure.maxFigs )
		Figure.figs = (FigPtr)azmem(Figure.figs,&Figure.maxFigs,5,sizeof(*Figure.figs));

	f = &Figure.figs[Figure.nFigs++];

	f->type = type;
	f->coord = coord;
	f->x0 = x0;
	f->y0 = y0;
	f->x1 = x1;
	f->y1 = y1;
	f->fgName = fgName;

	if( f->coord == DATAC ) {
		minmax( x0, y0 );
		minmax( x1, y1 );
	}
}


FigureDraw( f )
FigPtr	f;
{
double	x, y, scl;
Ptype	x0, y0, x1, y1, dx, dy;

	x0 = f->x0;
	y0 = f->y0;
	x1 = f->x1;
	y1 = f->y1;

	transform( &x0, &y0, f->x0, f->y0, 0.0, f->coord );
	transform( &x1, &y1, f->x1, f->y1, 1.0, f->coord );
	FontGroupSelect( "f", f->fgName );

	switch( f->type ) {
		case ARROW:
			move( x1, y1 );
			cont( x0, y0 );
			x = (double)(x1-x0)/xinch;
			y = (double)(y1-y0)/yinch;
			scl = (old_ps == -1) ? 1 : old_ps/DEFAULT_PS;
			scl *= 0.08/sqrt(x*x+y*y) * (p->xinches+p->yinches)/14.0;
			x1 = x0 + scl*(x1-x0);
			y1 = y0 + scl*(y1-y0);
			dx = 0.4*scl*y*xinch + 0.5;
			dy = 0.4*scl*x*yinch + 0.5;
			move( x1+dx, y1-dy );
			cont( x0, y0 );
			cont( x1-dx, y1+dy );
			break;
		case BOX:
			move( x0, y0 );
			cont( x0, y1 );
			cont( x1, y1 );
			cont( x1, y0 );
			cont( x0, y0 );
			break;
		case DARKBOX:
			PolyDef( x0, y0, CMOVE );
			PolyDef( x0, y1, CCONT );
			PolyDef( x1, y1, CCONT );
			PolyDef( x1, y0, CCONT );
			PolyDef( x0, y0, CFILL );
			break;
		case CONNECT:
			move( x0, y0 );
			cont( x1, y1 );
			break;
	}
}

minmax( x, y )
double	x, y;
{
	if( x != DEFAULT ) {
		if( x < xmin )
			xmin = x;
		else
			if( x > xmax )
				xmax = x;
	}
	if( y != DEFAULT ) {	
		if( y < ymin )
			ymin = y;
		else
			if( y > ymax )
				ymax = y;
	}
}


transform( xp, yp, xdbl, ydbl, dflt, coord )
Ptype	*xp, *yp;
double	xdbl, ydbl, dflt;
Const	coord;
{
	switch( coord ) {
		case DATAC:
			if( xdbl == DEFAULT )
				*xp = dflt*(xwmax-xwmin)+xwmin+0.5;
			else
				*xp = X(xdbl) + 0.5;
			if( ydbl == DEFAULT )
				*yp = dflt*(ywmax-ywmin)+ywmin+0.5;
			else
				*yp = Y(ydbl) + 0.5;
			break;
		case WINC:
			*xp = ((xdbl == DEFAULT) ? dflt : xdbl)*(xwmax-xwmin) + xwmin+0.5;
			*yp = ((ydbl == DEFAULT) ? dflt : ydbl)*(ywmax-ywmin) + ywmin+0.5;
			break;
		case PDEVC:
			*xp = xdbl + 0.5;
			*yp = ydbl + 0.5;
		break;
	}
}


LegendDef( lineNum, pltNum, name )
long	lineNum, pltNum;
char	*name;
{
LegPtr	l;

	if( Figure.nLegs == Figure.maxLegs )
		Figure.legs = (LegPtr)azmem(Figure.legs,&Figure.maxLegs,5,sizeof(*Figure.legs) );

	l = &Figure.legs[Figure.nLegs++];
	l->lineNum = lineNum;
	l->pltNum = pltNum;
	l->text = name;
}


LegendDraw()
{
PltPtr	plt;
LegPtr	l;
double	gray;
Ptype	n, xb0, yb0, xba, xb1, yb1, xMargin, yMargin, textWidth,
		x, y, x0, x1, xsize, ysize;
Boolean	lineSeg, havePlts;

	if( Figure.xlPos == DEFAULT )
		Figure.xlPos = 0.7;
	if( Figure.ylPos == DEFAULT )
		Figure.ylPos = 0.85;
	if( Figure.xlDel == DEFAULT )
		Figure.xlDel = 1.0;

	Figure.xlPos = xa.min + Figure.xlPos*(xa.max-xa.min);
	Figure.ylPos = ya.min + Figure.ylPos*(ya.max-ya.min);
	if( Figure.xlDel < 0.25 )
		Figure.xlDel /= 0.05;

	havePlts = lineSeg = NO;

	xba = X(Figure.xlPos);
	yb0 = Y(Figure.ylPos);
	yb1 = p->yfull;
	textWidth = 0;

	FontGroupSelect( "f", Figure.legfg );
	xMargin = chwd;
	yMargin = chht/3;

	for( n=0;  n < Figure.nLegs;  n++ ) {
		l = &Figure.legs[n];
		if( l->lineNum == DEFAULT )
			l->lineNum = n;
		strsize( l->text, &xsize, &ysize, 0.0 );
		l->yPos = yb0 - ysize*(3*l->lineNum+2)/3 - yMargin;
		if( yb1 > l->yPos )
			yb1 = l->yPos;
		if( xsize > textWidth )
			textWidth = xsize;
		if( l->pltNum == DEFAULT )
			continue;
		if( l->pltNum < 0 || l->pltNum >= Plot.nPlts ) {
			err( YES, "Bad plot number %d for legend", l->pltNum );
			continue;
		}
		havePlts = YES;
		plt = &Plot.plts[l->pltNum];
		if( plt->pm != SCATTER && plt->pm != SCATTER_STD )
			lineSeg = YES;
	}
	if( (fixed_font && Figure.xlBoxScl != 0) || Figure.xlBoxScl == DEFAULT )
		Figure.xlBoxScl = 1;

	x1 = xba - xMargin;
	if( havePlts )
		x0 = x1 - (int)(lineSeg ? 2.5*Figure.xlDel*chwd : chwd);
	else
		x0 = x1;
	xb0  = x0 - xMargin;
	xb1  = xba + (int)(textWidth*Figure.xlBoxScl) + xMargin;
	yb1 -= yMargin;
	
	if( Figure.eStat == 0 )
		Figure.eStat = (omode & ERASE) ? "yes" : "no";

	if( *Figure.eStat == 'y' || *Figure.eStat == 'Y') {
		PtrUnion	arg0, arg1;
		gray = oldGrayLevel;
		oldGrayLevel = PS_WHITE;
		arg0.d = &oldGrayLevel;
		special( SETGRAY, arg0, arg1 );
		PolyDef( xb0, yb0, CMOVE );
		PolyDef( xb1, yb0, CCONT );
		PolyDef( xb1, yb1, CCONT );
		PolyDef( xb0, yb1, CFILL );
		oldGrayLevel = gray;
		special( SETGRAY, arg0, arg1 );
	}

	if( Figure.xlBoxScl > 0 ) {
		PolyDef( xb0, yb0, CMOVE );
		PolyDef( xb1, yb0, CCONT );
		PolyDef( xb1, yb1, CCONT );
		PolyDef( xb0, yb1, CCONT );
		PolyDef( xb0, yb0, CSTROKE );
	}

	for( n=0;  n < Figure.nLegs;  n++ ) {
		l = &Figure.legs[n];

		FontGroupSelect( "f", Figure.legfg );
		plabel( l->text, xba, l->yPos, "LB", 0.0 );
		x = (x0+x1)/2;
		y = l->yPos + chht/3;

		if( l->pltNum == DEFAULT )
			continue;
		plt = &Plot.plts[l->pltNum];
		FontGroupSelect( "p", plt->fgName );

		switch( plt->pm ) {
			case NORMAL:
			case NCOLZERO:
			case LINES:
				move( x0, y );
				cont( x1, y );
				break;
			case FILLED:
			case FILLBETWEEN:
			case OUTLINE:
				PolyDef( x0, y-chht/3, CMOVE );
				PolyDef( x0, y+chht/3, CCONT );
				PolyDef( x1, y+chht/3, CCONT );
				PolyDef( x1, y-chht/3, CCONT );
				if( plt->pm == OUTLINE )
					PolyDef( x0, y-chht/3, CSTROKE );
				else
					PolyDef( x0, y-chht/2, CFILL );
				break;
			case OUTLINEFILL:
				PolyDef( x0, y-chht/3, CMOVE );
				PolyDef( x0, y+chht/3, CBBCONT );
				PolyDef( x1, y+chht/3, CBBCONT );
				PolyDef( x1, y-chht/3, CBBCONT );
				PolyDef( x0, y-chht/3, CBBFILL );
				break;
			case SYMBOL:
			case SCATTER:
			case SYMBOL_STD:
			case SCATTER_STD:
				if( plt->pc == ' ' )
					SmallBox( x, y );
					else	scatterplot( plt, x, y, y, y );
				break;
			default:
				err( NO, "No legend for plot type `%c'", plt->pm );
		}
	}
}

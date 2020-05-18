/*	plt/data.c		Paul Albrecht		Sept 1987

	Last Update:	13 June 1995 (GBM)

	Copyright (C) Paul Albrecht 1988.  All rights reserved.
*/

#include	<fcntl.h>
#include	"plt.h"
#include	"plot.h"
#include	"axis.h"
#include	"text.h"

static void DataLimits();

#ifndef		MAXNPTS
#define		MAXNPTS		500000
#endif


DataInit( argv )
char	*argv[];
{
	if( df[0] )
		argv = &df[-1];
	Data.nCols = argv[1] ? data_files(argv) : 0;

	if( Plot.pModes == 0 )
		Plot.pModes = zmem( 10, 1 );

	switch( Plot.xDrive ? Data.nCols+1 : Data.nCols ) {
		case  1:
			Plot.xDrive = YES;
			if( Plot.xFrom == DEFAULT )
				Plot.xFrom = 0;
			if( Plot.xIncr == DEFAULT )
				Plot.xIncr = 1;
			break;
		case  2:
			break;
		case  3:
			Plot.defaultPMode = CDRIVEN;
			break;
	}
	if( Plot.xDrive )
		Data.nCols++;

	if( Data.nCols )
		Data.row = (double *)zmem( Data.nCols, sizeof(double) );
}


ReadPoints()
{
Uint	addPts;
short	n;

	if( Data.nCols == 0 )
		return;

	while( data_read(Plot.xDrive ? &Data.row[1] : &Data.row[0]) ) {
		if( Plot.xDrive ) {
			Data.row[0] = Plot.xFrom;
			Plot.xFrom += Plot.xIncr;
		}
		if( Data.nPts+Data.nCols >= Data.maxPts ) {
			if( Data.maxPts >= MAXNPTS )
				err( YES, "Too many points" );
			addPts = 10000;
			if( Data.maxPts+addPts > MAXNPTS )
				addPts = MAXNPTS-Data.maxPts;
			Data.pts = (float *)azmem(Data.pts,&Data.maxPts,addPts,sizeof(*Data.pts));
		}

		for( n=0;  n < Data.nCols;  n++ )
			Data.pts[Data.nPts++] = Data.row[n];

		DataLimits();
	}
}

/***********************************************************************/

static void DataLimits()
{
register PltPtr	plt;
register double	cx, cy;
int		n;

	for( n=0, plt=Plot.plts;  n < Plot.nPlts;  n++, plt++ ) {
		switch( plt->pm ) {
			case LINES:
					cx = Data.row[plt->c2];
					if( cx < xmin ) xmin = cx;
					if( cx > xmax ) xmax = cx;
					cy = Data.row[plt->c3];
					if( cy < ymin ) ymin = cy;
					if( cy > ymax )	ymax = cy;
			case NORMAL:	/* LINES must flow into this */
			case DARKNORMAL:
			case FILLED:
			case LABEL_N:
			case CDRIVEN:
			case IMPULSE:
			case SYMBOL:
			case SCATTER:
					cx = Data.row[plt->c0];
					if( cx < xmin ) xmin = cx;
					if( cx > xmax ) xmax = cx;
			case NCOLZERO:
					cy = Data.row[plt->c1];
					if( cy < ymin ) ymin = cy;
					if( cy > ymax ) ymax = cy;
					break;
			case OUTLINE:
			case OUTLINEFILL:
			case FILLBETWEEN:
					cx = Data.row[plt->c0];
					if( cx < xmin ) xmin = cx;
					if( cx > xmax ) xmax = cx;
					cy = Data.row[plt->c1];
					if( cy < ymin ) ymin = cy;
					if( cy > ymax ) ymax = cy;
					cy = Data.row[plt->c2];
					if( cy < ymin ) ymin = cy;
					if( cy > ymax ) ymax = cy;
					break;
			case SYMBOL_STD:
			case SCATTER_STD:
					cy = Data.row[plt->c1]+Data.row[plt->c2];
					if( cy > ymax ) ymax = cy;
					cy -= Data.row[plt->c2]*2;
					if( cy < ymin ) ymin = cy;
					cx = Data.row[plt->c0];
					if( cx < xmin ) xmin = cx;
					if( cx > xmax ) xmax = cx;
					break;
		}
	}
}


/***********************************************************************/

DataRead( initialize )
Boolean	initialize;
{
static	long	curpts;
int	n;

	if( initialize ) {
		if( Plot.quickPlot )
			return( NO );	/* NO is for lint */
		curpts = 0;
		return( NO );	/* NO is for lint */
	}

	if( Plot.quickPlot ) {
		if( Plot.xDrive ) {
			Data.row[0] = Plot.xFrom;
			Plot.xFrom += Plot.xIncr;
			return( data_read(&Data.row[1]) );
		}
		return( data_read(&Data.row[0]) );
	}

	if( curpts < Data.nPts ) {
		for( n=0;  n < Data.nCols;  n++ )
			Data.row[n] = Data.pts[curpts++];
		return( YES );
	}
	return( NO );
}


ReverseDataRead( initialize )
{
static	long	curPts;
unsigned int	n;
float	*fltPtr;

	if( initialize ) {
		if( Plot.quickPlot )
			return( NO );	/* NO is for lint */
		curPts = 0;
		return( NO );	/* NO is for lint */
	}

	if( Plot.quickPlot ) {
		if( Plot.xDrive ) {
			Data.row[0] = Plot.xFrom;
			Plot.xFrom += Plot.xIncr;
			return( data_read(&Data.row[1]) );
		}
		return( data_read(&Data.row[0]) );
	}

	if( curPts < Data.nPts ) {
		curPts += Data.nCols;
		fltPtr = &Data.pts[Data.nPts-curPts];
		for( n=0;  n < Data.nCols;  n++ )
			Data.row[n] = *fltPtr++;
		return( YES );
	}
	return( NO );
}

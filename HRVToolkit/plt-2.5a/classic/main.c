/*	plt/main.c			Paul Albrecht		Sept 1984

	First version written in 1983.  Rewritten in 1984, 1986, and 1988.

	Last Update:	May 5, 1989

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Caution:
		The order of the various calls made here is quite important since some
		subroutines count on the environment being set up by the time they are
		called.
*/

#define		MAIN

#include	<signal.h>
#include	"plt.h"
#include	"plot.h"
#include	"axis.h"
#include	"figs.h"
#include	"text.h"
#include	"optn.h"


void	main( argc, argv )
char	*argv[];
{
PtrUnion	arg0, arg1;
FILE		*fp;
short		n;
char		*getenv();

	programName = argv[0];

	UtilInit( fullInit );
	TextInit( 0 );		/* must come before argument processing */
	FigureInit( fullInit );
	AxisInit( fullInit );
	TicInit( fullInit );
	PlotInit( fullInit );

	argvOpts( argv+1, argc-1 );

	if( pterm == 0 )
		pterm = getenv( "PTERM" );
	if( pterm == 0 )
		pterm = "";

	if( npso )
		PTERMSpecificOpts();
	arg0.c = pterm;
	special( SETPTERM, arg0, arg1 );	/* must follow pterm_specific_opts() */

	DataInit( argv );

	if( Data.nCols != 0 )
		PlotDef( Plot.pModes );

	if( pstr.str != 0 )
		ReadStrings( &pstr );
	if( fgstr.str != 0 )
		ReadStrings( &fgstr );

	gsupress();

	if( xa.min == DEFAULT || xa.max == DEFAULT ||
		ya.min == DEFAULT || ya.max == DEFAULT ) {
		if( Data.nCols == 0 )
			estop( "Must specify axis limits" );
	}
	else {	if( Data.nStreams == 1 && Plot.nPlts == 1 ) {
			Plot.quickPlot = YES;
			if( xa.lbl == 0 )
				xa.lbl = "";
		}
	}

	if( Data.nCols != 0 ) {
		if( !Plot.quickPlot )
			ReadPoints();
	}

	SetupAxes();

	if( axisfile ) {
		fp = fopen( axisfile, "w" );
		if( fp == 0 )
			err( YES, "Can't open %s for writing axis specs", axisfile );

		fprintf( fp, "xa %g %g %g %s %d %g\n",
			xa.min, xa.max, xa.tic, xa.pfm, xa.skp, xa.cr );
		fprintf( fp, "ya %g %g %g %s %d %g\n",
			ya.min, ya.max, ya.tic, ya.pfm, ya.skp, ya.cr );
		fprintf( fp, "W %g %g %g %g\n", xwmins, ywmins, xwmaxs, ywmaxs );
		if( fileno(fp) > 2 )
			fclose( fp );
	}

	pinit();

	space( 0, 0, p->xsquare, p->yfull );

	if( omode & ERASE )
		erase();

	if( xa.mode != 0 )
		XAxisDraw();
	if( ya.mode != 0 )
		YAxisDraw();

	MakeGraphTitle( argv+1 );

	/* The ordering below is important for the LaserWriter */

	if( omode & PLOTS ) {
		for( n=0;  n < Plot.nPlts;  n++ )
			PlotDraw( &Plot.plts[n] );
	}

	if( omode & FIGURES ) {
		if( Figure.nLegs > 0 )
			LegendDraw();
		for( n=0;  n < Figure.nFigs;  n++ )
			FigureDraw( &Figure.figs[n] );
	}

	if( omode & LABELS ) {
		for( n=0;  n < nlbls;  n++ )
			TextDraw( &lbls[n] );
	}

	if( Plot.excluded )
		err( NO, "*** Excluded %ld points ***", Plot.excluded );

	pquit(0);
}

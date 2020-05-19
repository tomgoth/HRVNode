/*	plt/plot.h		Paul Albrecht		Sept 1984

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4
*/

#define		CDRIVEN		'c'
#define		FILLED		'C'
#define		FILLBETWEEN	'f'
#define		IMPULSE		'i'
#define		LABEL_N		't'
#define		LINES		'l'
#define		NCOLZERO	'm'
#define		NORMAL		'n'
#define		DARKNORMAL	'N'
#define		OUTLINE		'o'
#define		OUTLINEFILL	'O'
#define		SCATTER		's'
#define		SCATTER_STD	'e'
#define		SYMBOL		'S'
#define		SYMBOL_STD	'E'

#define		UP_STD			'+'
#define		DOWN_STD		'-'
#define		SYMMETRIC_STD	':'

#define		CCONT		(Const)0
#define		CMOVE		(Const)1
#define		CDOT		(Const)2
#define		CBOX		(Const)3

#define		CBBCONT		(Const)7
#define		COSTROKE	(Const)8	/* stroke path without adding (x,y) */
#define		CSTROKE		(Const)9

#define		CFILL		(Const)10
#define		CBBFILL		(Const)11
#define		CFILLI		(Const)12
#define		CBBFILLI	(Const)14

#define		CHANGEBEGIN	(Const)20
#define		CHANGEFNT	(Const)20
#define		CHANGEPS	(Const)21
#define		CHANGELW	(Const)22
#define		CHANGELM	(Const)23
#define		CHANGEGRAY	(Const)24
#define		CHANGECOLOR	(Const)25
#define		CHANGEEND	25

#define		CSYMBOL		(Const)30
#define		CTEXT		(Const)100


#define		NO_COL		(-1)


typedef	struct {
	short	c0;
	short	c1;
	short	c2;
	short	c3;
	char	*fgName;
	char	pm;
	char	subpm;
	char	symbol;
	char	pc;
	char	*name;
	}	PltInfo, *PltPtr;


typedef	struct {	/* in the future we'll use this */
	float	*pts;
	long	nrows;	/* number of rows in the stream this came from */
	short	ncols;	/* number of columns in the stream this came from */
	double	min;
	double	max;
	}	ColInfo, *ColPtr;
	

void	PROTO( PlotInit, (Mode) );
void	PROTO( PlotDef, (char *) );


COMMON	struct	{
		PltPtr	plts;
		Uint	nPlts;
		Uint	maxPlts;
		double	xFrom;
		double	xIncr;
		Boolean	xDrive;
		Boolean	quickPlot;
		Boolean	exclude;
		long	excluded;
		char	*supress;
		char	*pModes;
		char	defaultPMode;
		}	Plot;


COMMON	struct	{
		float	*pts;
		double	*row;
		ColPtr	cols;
		Uint	nPts;
		Uint	maxPts;
		Uint	nCols;
		Uint	maxCols;
		short	nStreams;
		}	Data;


COMMON	char	*df[7];


/*	plt/figs.h		Paul Albrecht		Feb 1988

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4
*/


#define		ARROW		'A'
#define		BOX			'B'
#define		CONNECT		'C'
#define		DARKBOX		'D'

#define		DATAC		1
#define		WINC		2
#define		PDEVC		3



typedef	struct {
	char	type;
	char	coord;
	double	x0;
	double	y0;
	double	x1;
	double	y1;
	char	*fgName;
	}	FigInfo, *FigPtr;


typedef struct	{
	short	pltNum;
	short	lineNum;
	Ptype	yPos;
	char	*text;
	}	LegInfo, *LegPtr;


COMMON	struct	{		/* use this later */
		FigPtr	figs;
		Uint	nFigs;
		Uint	maxFigs;
		LegPtr	legs;
		Uint	nLegs;
		Uint	maxLegs;
		double	xlPos;
		double	ylPos;
		double	xlDel;
		double	xlBoxScl;
		char	*legfg;
		char	*eStat;
		}	Figure;


void	PROTO( FigureInit, (Mode) );

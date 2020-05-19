/*	plt/optn.h		Paul Albrecht		Sept 1987

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4
*/

#define		MAXFLDS		7
#define		MAXLINE		200


#define		ALLINE		(Const)1
#define		ALLINE2		(Const)2
#define		ALLINE3		(Const)3
#define		ALLINE4		(Const)4
#define		ALLMASK		(Const)017

#define		NOTARGV		(Const)020
#define		SPECIAL		(Const)040
#define		TYPEMASK	(~(NOTARGV|SPECIAL|ALLMASK))

#define		COMMENT		(Const)0100
#define		FIGURE		(Const)0200
#define		HELPTHEM	(Const)0400
#define		FIELDS		(Const)01000
#define		SLABEL		(Const)02000

#define		SPACE_DELIM		(Const)01
#define		SKIP_CR			(Const)02
#define		SEMI_TERM		(Const)04
#define		DEL_SPACE		(Const)010
#define		SAVE_TERM		(Const)020
#define		VERBATIM		(Const)040

#define		IN_OBJ			(DEL_SPACE | SPACE_DELIM | SEMI_TERM )

#define		END_INPUT		(Const)01
#define		END_LINE		(Const)02
#define		HAVE_OBJ		(Const)04


typedef	struct	{
	char	*name;
	char	*help;
	short	minflds;
	short	maxflds;
	short	mode;
	char	*flds;
	char	**fgnamep;
	char	*ptrs[MAXFLDS];
	}	OptInfo, *OptPtr;


typedef	struct	{
	char	*pterm;
	char	*options;
	}	PSOInfo, *PSOPtr;


OptPtr	PROTO( lookup, (char *) );
OptPtr	PROTO( lookup2, (char **, short *) );

COMMON	PSOPtr	psos;
COMMON	Boolean	optn;
COMMON	Uint	npso, maxpsos;
COMMON	char	*fgs;

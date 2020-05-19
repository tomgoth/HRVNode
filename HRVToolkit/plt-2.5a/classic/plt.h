/*	plt/plt.h		Paul Albrecht		Aug 1984

	Last Edit:	Oct 14, 1988

	NOTE:	Edit tabs should be set to every 4 chars.

	Copyright (C) Paul Albrecht 1988.  All rights reserved.
*/

#include	<stdio.h>
#include	<math.h>


#ifdef		PROTOTYPES
#define		PROTO( NAME, ARGS )		NAME(ARGS)
#else
#define		PROTO( NAME, ARGS )		NAME()
#endif


#ifdef		NDEBUG
#define		ASSERT(CONDITION,TEXT)		;
#else
#define		ASSERT(CONDITION,TEXT)	\
			if( !(CONDITION) )	assert( TEXT, __FILE__, __LINE__ );
#endif

#define		FHUGE		1e38
#define		YES			1
#define		NO			0
#define		DEFAULT		(Const)(-32767)
#define		FDEFAULT	(-32767.0)

#ifdef		MAIN
#define		COMMON
#define		SVAL(X)		= X
#else
#define		COMMON		extern
#define		SVAL(X)
#endif

#define		ENDP(ARRAY)	(&ARRAY[sizeof(ARRAY)/sizeof(ARRAY[0])])

#define		FREE(PTR)	{ 	char *charPtr=(char *)(PTR);				\
							ASSERT( charPtr != 0, "Freeing null ptr" );	\
							free( charPtr );	}


typedef		int				Const;
typedef		unsigned int	Uint;
typedef		unsigned int	Boolean;
typedef		short			Ptype;	/* Type expected by plotting routines */

#define		SETPTERM		(Const)1
#define		GRAPHELEMENT	(Const)2
#define		SETFONT			(Const)3
#define		SETLINEWIDTH	(Const)4
#define		SETGRAY			(Const)5
#define		SETGRAYD		(Const)6
#define		SETLINEMODE		(Const)7
#define		SETCOLOR		(Const)8

#define		EAXIS			'a'
#define		EFIGURE			'f'
#define		ELABEL			'l'
#define		EPLOT			'p'
#define		ETITLE			't'

#define		SEP_GRAPH		(Const)01
#define		SPECIAL_FNT		(Const)02
#define		ROT_LABEL		(Const)04

#define		DEFAULT_FONT	"Times-Roman"
#define		DEFAULT_PS		16.0
#define		DEFAULT_LW		3.0


#define		PS_BLACK		0.0
#define		PS_WHITE		1.0

#define		NULLS		((char *)0)

#define		NOTHING			(Const)0
#define		ERASE			(Const)01
#define		TITLES			(Const)02
#define		LABELS			(Const)04
#define		PLOTS			(Const)010
#define		FIGURES			(Const)020
#define		TICMARKS		(Const)0100
#define		TICNUMS			(Const)0200
#define		GRIDMARKS		(Const)0400
#define		AXIS			(Const)01000

typedef struct	pdev	{
	char	*pterm;		/* identifying name of plot device */
	short	mode;		/* info on device capabilities */
	Ptype	xfull;		/* full dimension in device units */
	Ptype	xsquare;	/* x dimension for equal to yfull */
	Ptype	yfull;		/* full dimension in device units */
	float	xwmins;		/*  x0 for plot area in (0,0)->(1,1) units */
	float	xwmaxs;		/*  y0 for plot area in (0,0)->(1,1) units */
	float	ywmins;		/*  x1 for plot area in (0,0)->(1,1) units */
	float	ywmaxs;		/*  y1 for plot area in (0,0)->(1,1) units */
	Ptype	ch;			/* Char height in y device units */
	Ptype	cw;			/* Char width in x device units */
	double	xinches;	/* horizontal size of plot display */
	double	yinches;	/* vertical size of plot display */
	} *PTERM;

typedef	union	{
	double	*d;
	long	*l;
	Boolean	*b;
	char	**s;
	char	*c;
	}	PtrUnion;


typedef	enum {	fullInit }	Mode;


COMMON	PTERM	p;

COMMON	double	dn(), default_ps SVAL(DEFAULT_PS),
		chhtscl SVAL(1), chwdscl SVAL(1),
		fscl SVAL(FDEFAULT), xdim SVAL(FDEFAULT),
		ydim SVAL(FDEFAULT), xorig SVAL(FDEFAULT), yorig SVAL(FDEFAULT),
		oldGrayLevel SVAL(-1), old_ps SVAL(-1), old_lw SVAL(-1);

COMMON	Ptype	chht, chwd, xinch, yinch;

COMMON	short	omode SVAL(ERASE|TITLES|LABELS|PLOTS|FIGURES);

COMMON	Boolean	fixed_font SVAL(DEFAULT);

COMMON	char	*pterm, *old_font, *old_lm SVAL(""), *programName;


void	PROTO( assert, (char *,char *,int) );


double	DoubleNum();
long	LongNum();
char	*zmem(), *azmem(), *StringSave();

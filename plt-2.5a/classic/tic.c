/*	plt/tic.c		Paul Albrecht		March 1988
 
	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	13 June 1995 (GBM)
	EMACS_MODES:	tabstop=4

	Attempt to make a smart choice for the tic marks.  To avoid worries
	about roundoff errors, all computation is done with long integers.

*/

#include	"plt.h"
#include	"axis.h"

static void TicSetup();
static void TicRound();
static void TicConfigCost();
static short DecimalPlaces();
static void MakeLongs();
static void GetString();
static void GetLong();

#define		WORKDIG		7		/* don't go over 8 	*/
#define		SIGDIG		5		/* max sig dig for tics	*/
#define		WORKLONG	(10000000L)	/* 10**WORKDIG		*/
#define		PLT_MINLONG	(100L)		/* 10**(WORKDIG-SIGDIG)	*/


typedef	struct	{
	double	dbl;	
	long	l;		/* the shifted WORKDIG digit representation dbl	*/
	short	exp;	/* the exponent of dbl	*/
	char	str[20];/* string representation of dbl	*/
	}	NumInfo, *NumPtr;


typedef	struct	{
	short	mult;	/* must divide WORKLONG without remainder! */
	float	icost;
	float	cost;
	long	tic;
	long	min;
	long	max;
	long	tmark;
	short	ntic;
	short	ticdp;
	short	mindp;
	short	maxdp;
	short	minfit;
	short	maxfit;
	}	TLWInfo, *TLWPtr;		/* Tic mark logic workspace	*/


static	NumInfo	lo,		/* minimum axis value	*/
			hi,			/* maximum axis value	*/
			diff,		/* length of the axis	*/
			tmark;		/* where a labeled tic mark should fall */

static	TLWInfo	tcs[]= { {10}, {50}, {25}, {20}, {30,1}, {40}, {60,1}, {75} };
static	double	tenf;
static	int	havetmark;


void	TicInit( mode )
Mode	mode;
{
}


LinearTicLogic( a )
AxisPtr	a;
{
TLWPtr	tc, tcbest;

	if( a->min == DEFAULT )
		lo.dbl = (a->name == 'x') ? xmin : ymin;
	else	lo.dbl = a->min;

	if( a->max == DEFAULT )
		hi.dbl = (a->name == 'x') ? xmax : ymax;
	else	hi.dbl = a->max;

	if( lo.dbl == hi.dbl ) {
		err( NO, "All %c values are %lg", a->name, lo.dbl );
		if( lo.dbl == 0 )
			hi.dbl = 1;
		else {	lo.dbl = floor( lo.dbl );
			hi.dbl = lo.dbl + 1;
		}
		a->acclo = a->acchi = 0;
	}

	if( a->skp == DEFAULT || a->skp < 1 )
		a->skp = 2;

	diff.dbl  = hi.dbl - lo.dbl;
	tmark.dbl = a->tmark;
	havetmark = !(tmark.dbl == DEFAULT || tmark.dbl < lo.dbl || tmark.dbl > hi.dbl);
		
	MakeLongs();

	if( ticlogic ) {
		eprintf( "\nGIVEN: %g  %g  (%g)\n", lo.dbl, hi.dbl, tmark.dbl );
		eprintf( "    MIN     MAX     TIC   NTIC    DEC PLACES    COST   TMARK\n" );
	}

	tcbest = tcs;
	for( tc=tcs; tc < ENDP(tcs);  tc++ ) {
		TicSetup( a, tc );
		TicRound( a, tc );

		tc->ticdp = DecimalPlaces( tc->tic );
		tc->mindp = DecimalPlaces( tc->min );
		tc->maxdp = DecimalPlaces( tc->max );

		TicConfigCost( a, tc );
		if( tcbest->cost > tc->cost )
			tcbest = tc;

		if( ticlogic )
			eprintf( "%8.3f%8.3f%8.3f%6d%5d%3d/%d%3d/%d%8.3f%8.3f\n",
			tc->min*tenf, tc->max*tenf, tc->tic*tenf,
			tc->ntic, tc->ticdp, tc->mindp, tc->minfit,
			tc->maxdp, tc->maxfit, tc->cost, tc->tmark*tenf );
		if( a->tic != DEFAULT || a->mlt != DEFAULT )
			break;
	}

	a->min = tcbest->min * tenf;
	a->max = tcbest->max * tenf;
	a->tic = tcbest->tic/a->skp * tenf;
	if( !havetmark )
		a->tmark = tcbest->tmark * tenf;

	if( a->pfm == NULLS ) {
		if( tcbest->ticdp <=  0 || tcbest->ticdp > SIGDIG )
			a->pfm = "%lg";
		else {	char fmt[30];
			sprintf( fmt, "%%.%dlf", (int)tcbest->ticdp );
			a->pfm = StringSave( fmt );
		}
	}

	if( a->tmark == DEFAULT ) {
		a->tmark = (a->name == 'x') ? ya.cr : xa.cr;
		if( a->tmark < a->min )
			a->tmark = a->min; 
		if( a->tmark > a->max )
			a->tmark = a->max;
	}

	if( ticlogic )
		eprintf( "CHOOSE: %g  %g  %g using %s  (%g)\n",
			a->min, a->max, a->tic, a->pfm, a->tmark );
}

/*************************************************************************/

static void TicSetup( a, tc )
AxisPtr	a;
TLWPtr	tc;
{
double	dbl;

	if( a->tic == DEFAULT ) {
		if( a->mlt != DEFAULT ) {
			dbl = a->skp*a->mlt/tenf;
			while( dbl < PLT_MINLONG )
				dbl *= 10;
			tc->tic = dbl + 0.5;
		}
		else	tc->tic = tc->mult;
		while( tc->tic*20 < diff.l )
			tc->tic *= 10;
	}
	else	tc->tic = a->skp*a->tic/tenf + 0.5;


	tc->min = lo.l;
	tc->max = hi.l;
	tc->tmark = havetmark ? tmark.l : (lo.l+tc->tic)/tc->tic*tc->tic;
	tc->cost = tc->icost;
}


/* Extend the axes to a multiple of the tic spacing */
static void TicRound(a, tc)
AxisPtr	a;
TLWPtr	tc;
{
long	test, stic;
int	n;

	tc->minfit = tc->maxfit = 0;
	for( n=0;  n < a->skp;  n++ ) {
		stic = tc->tic*(n+1)/a->skp;
		test = tc->tmark - (tc->tmark-lo.l+stic-1)/stic*stic;
		if( (test == 0 || tc->minfit == 0) && (double)(lo.l-test)/diff.l < a->acclo ) {
			tc->min = test;
			tc->minfit = (test/tc->tic*tc->tic == test); 
		}

		test = tc->tmark + (hi.l-tc->tmark+stic-1)/stic*stic;
		if( (test == 0 || tc->maxfit == 0) && (double)(test-hi.l)/diff.l < a->acchi ) {
			tc->max = test;
			tc->maxfit = (test/tc->tic*tc->tic == test);
		}
	}

	if( tc->max > 0 )
		tc->ntic = tc->max/tc->tic;
	else	tc->ntic = (tc->max - (tc->tic-1))/tc->tic;
	if( tc->min > 0 )
		tc->ntic -= (tc->min + (tc->tic-1))/tc->tic;
	else	tc->ntic -= tc->min/tc->tic;
	tc->ntic += 1;
}


/* Compute undesirability of tic configuration */
static void TicConfigCost(a, tc)
AxisPtr	a;
TLWPtr	tc;
{
AxisPtr	oa;
double	dbl;
int	n;

	n = (tc->ticdp > 0) ? tc->ticdp : 0;
	tc->cost += n;
	tc->cost += tc->minfit ? ((tc->mindp > 0) ? tc->mindp : 0) : n;
	tc->cost += tc->maxfit ? ((tc->maxdp > 0) ? tc->maxdp : 0) : n;

	n = tc->maxfit+tc->minfit;
	dbl = 3 + n/2.0;
	tc->cost += (tc->ntic < dbl) ?  2*(dbl-tc->ntic) : 0;;
	dbl = 6 + n/2.0;
	tc->cost += (tc->ntic > dbl) ?  2*(tc->ntic-dbl) : 0;;

	oa = (a->name == 'x' ? &ya : &xa);
	if( oa->cr == DEFAULT || oa->cr == a->min )
		tc->cost -= tc->minfit/2.0;
	else	if( oa->cr == a->max )
			tc->cost -= tc->maxfit/2.0;
}


/* How many decimal places are needed to print lnum */
static short DecimalPlaces( lnum )
long	lnum;
{
short	dp;

	dp = -(WORKDIG + diff.exp);
	while( YES ) {
		lnum = 10*(lnum - lnum/WORKLONG*WORKLONG);
		if( lnum == 0 )
			break;
		dp++;
	}
	return( dp );
}

/**************************************************************************

	Subroutine GetString(num) takes fabs(num->dbl) and puts the first
	WORKDIG significant digits into the string num->str.  The base 10
	exponent is stored in num->exp;

	The variable eref is the largest base 10 exponent value of the
	following three numbers: the axis min, axis max, and axis max-min.

	Subroutine GetLong(num,eref) produces a long integer num->l which
	is equal to num->dbl*WORKLONG/10**eref.  The exponent num->exp becomes
	eref.  The actual value of num->l is always less than WORKLONG.
	This provides the first WORKDIG working digits for the three relevant
	numbers (min, max, max-min).  The numbers are scaled so that the
	is greater than or equal to WORKLONG/10.
*/

static void MakeLongs()
{
int	eref;

	GetString( &lo );
	GetString( &hi );
	GetString( &diff );

	eref = (lo.exp > hi.exp) ? lo.exp : hi.exp;
	if( diff.exp > eref )
		eref = diff.exp;

	GetLong( &lo, eref );
	GetLong( &hi, eref );
	GetLong( &diff, eref );

	if( diff.l < PLT_MINLONG ) {
		diff.l = PLT_MINLONG;
	}
	if( havetmark ) {
		GetString( &tmark );
		GetLong( &tmark, eref );
	}

	tenf = pow( 10.0, (double)eref );
}


static void GetString( num )
NumPtr	num;
{
int		n;		/* must be int */
short	ndig;
char	*str;


	str = num->str;
	sprintf( str, "%.8le", num->dbl );
	while( *str && (*str == '0' || *str == '-' || *str == ' ') )
		str++;

	ndig = 0;
	while( *str && *str != 'e' && *str != 'E' ) {
		if( *str == '.' )
		 	num->exp = ndig;
		else {	if( ndig < WORKDIG )
				num->str[ndig++] = *str;
		}
		str++;
	}
	while( ndig < WORKDIG )
		num->str[ndig++] = '0';
	num->str[ndig] = 0;

	sscanf( str+1, "%d", &n );
	num->exp += n - WORKDIG;
}


static void GetLong( num, eref )
NumPtr	num;
{
long	atol();
short	ndigs;

	ndigs = WORKDIG - (eref - num->exp);
	if( ndigs > 0 ) {
		num->str[ndigs] = 0;
		num->l = atol(num->str);
	}
	else	num->l = 0;

	num->exp = eref;
	if( num->dbl < 0 )
		num->l = -num->l;
}

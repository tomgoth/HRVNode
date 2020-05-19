/* file: tick.c		Paul Albrecht		   March 1988
			Last revised:		15 April 2001
Tick selection for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  15 April 2001: general cleanup
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

Attempt to make a smart choice for the tick marks.  To avoid worries
about roundoff errors, all computation is done with long integers.
*/

#include "plt.h"

#define WORKDIG		7	/* number of digits in intermediate results;
				   don't go over 8 */
#define SIGDIG		5	/* max number of significant digits in ticks */
#define	WORKLONG	(10000000L)	/* 10**WORKDIG		*/
#define	PLT_MINLONG	(100L)		/* 10**(WORKDIG-SIGDIG)	*/

typedef	struct {
    double dbl;	
    long l;		/* the shifted WORKDIG digit representation dbl	*/
    short exp;		/* the exponent of dbl	*/
    char str[20];	/* string representation of dbl	*/
} NumInfo, *NumPtr;


typedef struct {
    short mult;	/* must divide WORKLONG without remainder! */
    float icost;
    float cost;
    long tick;
    long min;
    long max;
    long tmark;
    short ntick;
    short tickdp;
    short mindp;
    short maxdp;
    short minfit;
    short maxfit;
} TLWInfo, *TLWPtr;	/* Tick mark logic workspace	*/

/* Prototypes of functions defined in this module. */
void LinearTickLogic(AxisPtr a);

static void TickSetup(AxisPtr a, TLWPtr tc);
static void TickRound(AxisPtr a, TLWPtr tc);
static void TickConfigCost(AxisPtr a, TLWPtr tc);
static short DecimalPlaces(long lnum);
static void MakeLongs(void);
static void GetString(NumPtr num);
static void GetLong(NumPtr num, int eref);

static NumInfo lo;	/* minimum axis value	*/
static NumInfo hi;	/* maximum axis value	*/
static NumInfo diff;	/* length of the axis	*/
static NumInfo tmark;	/* where a labeled tick mark should fall */

static TLWInfo tcs[]= { {10}, {50}, {25}, {20}, {30,1}, {40}, {60,1}, {75} };
static double tenf;
static int havetmark;

void LinearTickLogic(AxisPtr a)
{
    TLWPtr tc, tcbest;

    lo.dbl = (a->min == DEFAULT) ? ((a->name == 'x') ? xmin : ymin) : a->min;
    hi.dbl = (a->max == DEFAULT) ? ((a->name == 'x') ? xmax : ymax) : a->max;
    if (lo.dbl == hi.dbl) {
	err(NO, "All %c values are %lg", a->name, lo.dbl);
	if (lo.dbl == 0)
	    hi.dbl = 1;
	else {
	    lo.dbl = floor(lo.dbl);
	    hi.dbl = lo.dbl + 1;
	}
	a->acclo = a->acchi = 0;
    }
    if (a->skp == DEFAULT || a->skp < 1)
	a->skp = 2;
    diff.dbl  = hi.dbl - lo.dbl;
    tmark.dbl = a->tmark;
    havetmark = !(tmark.dbl == DEFAULT ||
		  tmark.dbl < lo.dbl ||
		  tmark.dbl > hi.dbl);
    MakeLongs();
    if (ticklogic) {
	err(NO, "\nGIVEN: %g  %g  (%g)\n", lo.dbl, hi.dbl, tmark.dbl);
	err(NO,
	    "    MIN     MAX     TICK  NTICK   DEC PLACES    COST   TMARK\n");
    }
    tcbest = tcs;
    for (tc = tcs; tc < ENDP(tcs);  tc++) {
	TickSetup(a, tc);
	TickRound(a, tc);
	tc->tickdp = DecimalPlaces(tc->tick);
	tc->mindp = DecimalPlaces(tc->min);
	tc->maxdp = DecimalPlaces(tc->max);
	TickConfigCost(a, tc);
	if (tcbest->cost > tc->cost)
	    tcbest = tc;
	if (ticklogic)
	    err(NO,"%8.3f%8.3f%8.3f%6d%5d%3d/%d%3d/%d%8.3f%8.3f\n",
		tc->min*tenf, tc->max*tenf, tc->tick*tenf,
		tc->ntick, tc->tickdp, tc->mindp, tc->minfit,
		tc->maxdp, tc->maxfit, tc->cost, tc->tmark*tenf);
	if (a->tick != DEFAULT || a->mlt != DEFAULT)
	    break;
    }
    a->min = tcbest->min * tenf;
    a->max = tcbest->max * tenf;
    a->tick = tcbest->tick/a->skp * tenf;
    if (!havetmark)
	a->tmark = tcbest->tmark * tenf;

    if (a->pfm == NULL) {
	if (tcbest->tickdp <=  0 || tcbest->tickdp > SIGDIG)
	    a->pfm = "%lg";
	else {
	    char fmt[30];
	    sprintf(fmt, "%%.%dlf", (int)tcbest->tickdp);
	    a->pfm = StringSave(fmt);
	}
    }
    if (a->tmark == DEFAULT) {
	a->tmark = (a->name == 'x') ? ya.cr : xa.cr;
	if (a->tmark < a->min)
	    a->tmark = a->min; 
	if (a->tmark > a->max)
	    a->tmark = a->max;
    }
    if (ticklogic)
	err(NO, "CHOOSE: %g  %g  %g using %s  (%g)\n",
	    a->min, a->max, a->tick, a->pfm, a->tmark);
}

static void TickSetup(AxisPtr a, TLWPtr tc)
{
    double dbl;

    if (a->tick == DEFAULT) {
	if (a->mlt != DEFAULT) {
	    dbl = a->skp*a->mlt/tenf;
	    while (dbl < PLT_MINLONG)
		dbl *= 10;
	    tc->tick = dbl + 0.5;
	}
	else
	    tc->tick = tc->mult;
	while (tc->tick*20 < diff.l)
	    tc->tick *= 10;
    }
    else
	tc->tick = a->skp*a->tick/tenf + 0.5;
    tc->min = lo.l;
    tc->max = hi.l;
    tc->tmark = havetmark ? tmark.l : (lo.l+tc->tick)/tc->tick*tc->tick;
    tc->cost = tc->icost;
}

/* Extend the axes to a multiple of the tick spacing */
static void TickRound(AxisPtr a, TLWPtr tc)
{
    long test, stick;
    int n;

    tc->minfit = tc->maxfit = 0;
    for (n = 0;  n < a->skp;  n++) {
	stick = tc->tick*(n+1)/a->skp;
	test = tc->tmark - (tc->tmark-lo.l+stick-1)/stick*stick;
	if ((test == 0 || tc->minfit == 0) &&
	    (double)(lo.l-test)/diff.l < a->acclo) {
	    tc->min = test;
	    tc->minfit = (test/tc->tick*tc->tick == test); 
	}
	test = tc->tmark + (hi.l-tc->tmark+stick-1)/stick*stick;
	if ((test == 0 || tc->maxfit == 0) &&
	    (double)(test-hi.l)/diff.l < a->acchi) {
	    tc->max = test;
	    tc->maxfit = (test/tc->tick*tc->tick == test);
	}
    }
    if (tc->max > 0)
	tc->ntick = tc->max/tc->tick;
    else
	tc->ntick = (tc->max - (tc->tick-1))/tc->tick;
    if (tc->min > 0)
	tc->ntick -= (tc->min + (tc->tick-1))/tc->tick;
    else
	tc->ntick -= tc->min/tc->tick;
    tc->ntick += 1;
}

/* Compute undesirability of tick configuration */
static void TickConfigCost(AxisPtr a, TLWPtr tc)
{
    AxisPtr oa;
    double dbl;
    int n;

    n = (tc->tickdp > 0) ? tc->tickdp : 0;
    tc->cost += n;
    tc->cost += tc->minfit ? ((tc->mindp > 0) ? tc->mindp : 0) : n;
    tc->cost += tc->maxfit ? ((tc->maxdp > 0) ? tc->maxdp : 0) : n;
    n = tc->maxfit + tc->minfit;
    dbl = 3 + n/2.0;
    tc->cost += (tc->ntick < dbl) ?  2*(dbl-tc->ntick) : 0;;
    dbl = 6 + n/2.0;
    tc->cost += (tc->ntick > dbl) ?  2*(tc->ntick-dbl) : 0;;
    oa = (a->name == 'x' ? &ya : &xa);
    if (oa->cr == DEFAULT || oa->cr == a->min)
	tc->cost -= tc->minfit/2.0;
    else if (oa->cr == a->max)
	tc->cost -= tc->maxfit/2.0;
}

/* How many decimal places are needed to print lnum */
static short DecimalPlaces(long lnum)
{
    short dp = -(WORKDIG + diff.exp);

    while (lnum = 10*(lnum - lnum/WORKLONG*WORKLONG))
	dp++;
    return (dp);
}

static void MakeLongs(void)
{
    int eref;

    GetString(&lo);
    GetString(&hi);
    GetString(&diff);
    eref = (lo.exp > hi.exp) ? lo.exp : hi.exp;
    if (diff.exp > eref)
	eref = diff.exp;
    GetLong(&lo, eref);
    GetLong(&hi, eref);
    GetLong(&diff, eref);
    if (diff.l < PLT_MINLONG)
	diff.l = PLT_MINLONG;
    if (havetmark) {
	GetString(&tmark);
	GetLong(&tmark, eref);
    }
    tenf = pow(10.0, (double)eref);
}

/* GetString(num) takes fabs(num->dbl) and puts the first WORKDIG significant
   digits into the string num->str.  The base 10 exponent is stored in
   num->exp.

   The variable eref is the largest base 10 exponent value of the following
   three numbers: the axis min, axis max, and axis max-min.

   Subroutine GetLong(num,eref) produces a long integer num->l, which is equal
   to num->dbl*WORKLONG/10**eref.  The exponent num->exp becomes eref.  The
   actual value of num->l is always less than WORKLONG.  This provides the
   first WORKDIG working digits for the three relevant numbers (min, max,
   max-min).  The numbers are scaled so that num->l (?) is greater than or
   equal to WORKLONG/10.  */

static void GetString(NumPtr num)
{
    int n;		/* must be int */
    short ndig;
    char *str;

    str = num->str;
    sprintf(str, "%.8le", num->dbl);
    while (*str && (*str == '0' || *str == '-' || *str == ' '))
	str++;

    ndig = 0;
    while (*str && *str != 'e' && *str != 'E') {
	if (*str == '.')
	    num->exp = ndig;
	else if (ndig < WORKDIG)
	    num->str[ndig++] = *str;
	str++;
    }
    while (ndig < WORKDIG)
	num->str[ndig++] = '0';
    num->str[ndig] = 0;
    sscanf(str+1, "%d", &n);
    num->exp += n - WORKDIG;
}

static void GetLong(NumPtr num, int eref)
{
    long atol();
    short ndigs;

    ndigs = WORKDIG - (eref - num->exp);
    if (ndigs > 0) {
	num->str[ndigs] = 0;
	num->l = atol(num->str);
    }
    else
	num->l = 0;
    num->exp = eref;
    if (num->dbl < 0)
	num->l = -num->l;
}

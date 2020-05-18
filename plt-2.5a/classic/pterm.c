/*	plt/pterm.c		Paul Albrecht		Sept 1984

	Last Edit:	May 5, 1989

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	All xfull and yfull dimensions should be in the thousands
	in order to avoid roundoff errors in small motions.
*/


#include	"plt.h"


struct	pdev	pdevs[] = {
	"go140", 0,
		4096, 4096, 3120, 0.16, 0.97, 0.15, 0.90, 88, 56, 0, 0,
	"xw", 0,
		4096, 4096, 3120, 0.16, 0.97, 0.15, 0.90, 88, 56, 0, 0,
	"sun",   0,
		4604, 4604, 3596, 0.20, 0.95, 0.20, 0.85, 72, 44, 14, 11,
	/* lw is special - initialization determines xfull, yfull */
	"lw",	SEP_GRAPH | SPECIAL_FNT | ROT_LABEL,
		0, 0, 0, 0.20, 0.95, 0.20, 0.85, 0, 0, 7.2, 6,
	"ega", 	SEP_GRAPH,
		3200, 2250, 1750, 0.17, 0.95, 0.15, 0.90, 50, 40, 8.5, 6,
	"pga", 	SEP_GRAPH,
		2556, 2556, 1916, 0.17, 0.95, 0.20, 0.85, 60, 32, 0, 0,
	"go235", 0,
		4096, 4096, 3168, 0.16, 0.97, 0.15, 0.90, 88, 56, 9, 7,
	"seltek", 0,
		4096, 4096, 3120, 0.17, 0.95, 0.15, 0.90, 88, 60, 0, 0,
	"4014",	0,
		4096, 4096, 3120, 0.20, 0.95, 0.20, 0.85, 80, 48, 0, 0,
	"mcd",	SEP_GRAPH,
		1664, 1664, 1200, 0.15, 0.95, 0.15, 0.85, 32, 18, 12.75, 9,
	"laser", SEP_GRAPH,
		1023, 780, 780, 0.20, 0.95, 0.20, 0.85, 24, 14, 0, 0,
	"7470",	SEP_GRAPH,
		3328, 3328, 2400, 0.15, 0.95, 0.15, 0.85, 68, 46, 0, 0,
	"4662", SEP_GRAPH,
		4095, 2732, 2732, 0.20, 0.95, 0.20, 0.85, 88, 56, 0, 0,
	"vt52",	0,
		800, 640, 240, 0.10, 0.95, 0.15, 0.99, 10, 10, 0, 0,
	"dumb",	0,
		630, 630, 230, 0.15, 0.95, 0.15, 0.90, 10, 10, 0, 0,
	};



PTERMLookup( ptermName, hardwired )
char	*ptermName, *hardwired;
{
	if( hardwired ) {
		for( p=pdevs;  p < ENDP(pdevs);  p++ ) {
			if( strcmp(hardwired,p->pterm) == 0 )
				break;
		}
		if( p == ENDP(pdevs) )
			err( NO, "Driver error, no PTERM `%s'" );
		if( strcmp(ptermName,hardwired) != 0 )
			err( NO, "Program hardwired for PTERM `%s'", hardwired );
	}
	else {	for( p=pdevs;  p < ENDP(pdevs);  p++ ) {
			if( strcmp(ptermName,p->pterm) == 0 )
				break;
		}
		if( p == ENDP(pdevs) )
			err( NO, "Unrecognized PTERM `%s'", ptermName );
	}
	if( p == ENDP(pdevs) )
		p = pdevs;

	if( (p->mode&SEP_GRAPH) == 0 && !isatty(fileno(stdout)) )
		p->mode |= SEP_GRAPH;

	if( fixed_font == DEFAULT )
		fixed_font = !(p->mode & SPECIAL_FNT);

	chht = p->ch;
	chwd = p->cw;

	if( p->xinches == 0 )
		p->xinches = 8;
	if( p->yinches == 0 )
		p->yinches = 6;

	xinch = p->xfull/p->xinches;
	yinch = p->yfull/p->yinches;
}

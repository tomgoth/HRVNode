/*	go.c		Paul Albrecht		Sept 1984
 
	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4

	Graphics primitives for Graphon and other Tektronix emulators.
*/

#include	"plt.h"
#include	<signal.h>
/* #include	<sgtty.h>
*/

static	double	xmin, ymin, xscl = 1, yscl = 1;
static	Ptype	oldx, oldy;
static	char	ohiy = -1, ohix = -1, oloy = -1;

static	struct	{
	char	*pterm;
	PTERM	p;
	char	*open;
	char	*close;
	char	*erase;
	}	tty_codes[] = {
	{ "go140", 0, "\0331\033*\035", "\035\0332", "\033\014" },
	{ "go235", 0, "\0331\033*\035", "\035\0332", "\033\014" },
	{ "seltek",0, "\0332\033H\033J\0331\033*\035", "\035\0332", "\033\014" },
	{ "4014",  0, "\035",           "\035\0332", "\033\014" },
	}, *codes;


static	void quit(sig)
int sig;
{
static	short	reset;

	signal( SIGINT, quit );

	if( !reset ) {
		reset = 1;
		closepl();
		exit( 1 );
	}
}

openpl()
{
	printf( codes->open );

	signal( SIGINT, quit );
}

closepl()
{
	if( codes != 0 ) {
		printf( codes->close );
		fflush( stdout );
	}
}

erase()
{
	printf( codes->erase );
	ohix = ohiy = oloy =  -1;
}

space( x0, y0, x1, y1 )
Ptype	x0, y0, x1, y1;
{
	xmin = x0;
	ymin = y0;

	xscl = ((double)(codes->p)->xsquare-1)/(x1-x0);
	yscl = ((double)(codes->p)->yfull-1)/(y1-y0);
}

point( x, y )
Ptype	x, y;
{
	move( x, y );
	cont( x, y );
}


line( x0, y0, x1, y1 )
Ptype	x0, y0, x1, y1;
{
	move( x0, y0 );
	cont( x1, y1 );
}

label( s )
char	*s;
{
	putchar( 037 );
	while( *s ) {
		putchar( *s );
		s++;
	}
	putchar( 035 );
}

move( x, y )
Ptype	x, y;
{
	putchar( 035 );
	cont( x, y );
}

cont( x, y )
Ptype	x, y;
{
	oldx = (x-xmin) * xscl + 0.5;
	oldy = (y-ymin) * yscl + 0.5;
	xmt( oldx, oldy );
}

xmt( x, y )
Ptype	x, y;
{
char	hix, hiy, lox, loy;

	hix = (x>>7) & 037;
	hiy = (y>>7) & 037;
	lox = (x>>2) & 037;
	loy = (y>>2) & 037;
	
	if( hiy != ohiy ) {
		putchar( hiy|040 );
		ohiy = hiy;
	}
	if( hix != ohix ){
		putchar( loy|0140 );
		putchar( hix|040 );
		ohix = hix;
		oloy = loy;
	}
	if( loy != oloy ) {
		putchar( loy|0140 );
		oloy = loy;
	}
	putchar( lox|0100 );
}

/************************************************************************/

special( what, arg0, arg1 )
PtrUnion	arg0, arg1;
{
	switch( what ) {
		case SETPTERM:
			PTERMLookup( pterm, NULLS );
			for( codes=tty_codes;  codes < ENDP(tty_codes);  codes++ ) {
				if( strcmp(pterm,codes->pterm) == 0 )
					break;
			}
			if( codes == ENDP(tty_codes) )
				codes = tty_codes;
			codes->p = p;
			break;
		case SETLINEMODE:
			linemod( arg0.c );
			break;
	}
}

linemod( pattern )
char	*pattern;
{
		if( strcmp(pattern,"dotted") == 0 )
			printf( "\033a" );
	else	if( strcmp(pattern,"dotdashed") == 0 )
			printf( "\033b" );
	else	if( strcmp(pattern,"shortdashed") == 0 )
			printf( "\033c" );
	else	if( strcmp(pattern,"longdashed") == 0 )
			printf( "\033d" );
	else	if( strcmp(pattern,"solid") == 0 )
			printf( "\033`" );
}

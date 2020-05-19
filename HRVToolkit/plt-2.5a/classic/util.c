/*	plt/util.c		Paul Albrecht		Sept 1984

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4

	Beware:	subroutine err() is very possibly machine dependent.
*/

#include	<signal.h>
#include	"plt.h"
#include	"plot.h"

#define		MAXERR		6


static	Boolean	firstQuit;
static	short	nErrors, opened;
static	char	*errors[MAXERR];
void pquit();

void	UtilInit( mode )
Mode	mode;
{
short	n;

	firstQuit = YES;
	for( n=0;  n < nErrors;  n++ ) {
		FREE( errors[n] );
	}
	nErrors = 0;
}

/***************************************************************************/

char	*zmem( n, size )
Uint	n, size;
{
char	*ptr, *calloc();

	if( n*size < 1 || (ptr=calloc(n,size)) == 0 )
		err( YES, "Out of memory.  Can't allocate %u bytes", n*size  );

	return( ptr );
}

char	*azmem( ptr, max, n, size )
char	*ptr;
Uint	*max, n, size;
{
Uint	oldb, newb;
char	*realloc(), *calloc();
register char *c, *cend;

	oldb = (*max) * size;
	newb = oldb + n*size;
	*max += n;

	if( ptr == 0 || oldb == 0 )
		ptr = newb ? calloc(n,size) : 0;
	else
	 	ptr = realloc(ptr, newb);

	if( ptr == 0 )
		err( YES, "Out of memory.  Can't allocate %u bytes", n*size  );

	for( c= &ptr[oldb], cend= &ptr[newb];  c < cend;  c++ )
			*c = 0;

	return( ptr );
}


char	*StringSave( str )
char	*str;
{
Uint	len;
char	*ptr, *malloc();

	if( str == 0 ) {
		err( NO, "BUG: called StringSave(0)" );
		return( 0 );
	}

	len = strlen(str) + 1;
	ptr = malloc( len );
	strcpy( ptr, str );
	return( ptr );
}

/************************ Error Handling ******************************/

#define		ARGS		a0, a1, a2, a3, a4, a5, a6, a7, a8, a9

/* VARARGS2 */
err( fatal, fmt, ARGS )
int		fatal;
char	*fmt;
long	ARGS;
{
short	n;
char	str[200];

	sprintf( str, "%s: ", programName );
	n = strlen( str );
	sprintf( &str[n], fmt, ARGS );
	if( p && p->mode & SEP_GRAPH )
		fprintf( stderr,"%s\n", str );
	else
		errors[nErrors++] = StringSave( str );

	if( nErrors == MAXERR-1 ) {
		sprintf( str, "%s: Aborting...", programName );
		if( p && p->mode & SEP_GRAPH )
			fprintf( stderr, "%s\n", str );
		else
			errors[nErrors++] = StringSave( str );
		pquit(0);
	}

	if( fatal )
		pquit(0);
}





void	assert( text, file, line )
char	*text;
char	*file;
int		line;
{
	err( YES, "Assertion false at %s(%d): %s\n",  file, line, text );
}


void pquit(signo)
int signo;
{
int	n;

	signal( SIGINT, SIG_IGN );
	if( firstQuit ) {
		firstQuit = NO;
		if( opened ) {
			closepl();
			opened = NO;
		}
		if( p == 0 || (p->mode&SEP_GRAPH) == 0 )
			drain_output();
		for( n=0;  n < nErrors;  n++ )
			fprintf( stderr, "%s\n", errors[n] );
	}
	exit( 0 );
}


pinit()
{
	signal( SIGINT, pquit );
	openpl();
	opened = YES;
}

drain_output()
{
#include	<termio.h>
		ioctl( fileno(stdout), TCSBRK, (struct termio*)1 );
}

/*************** Convert a string to a (double) or (int) ***************/

double	DoubleNum( str )
char	*str;
{
double	dbl;
char	line[200], check_char;

	if( strcmp("-",str) == 0 )
		return( DEFAULT );

	strcpy( line, str );
	strcat( line, "?" );

	check_char = 0;
	sscanf( line, "%lf%c", &dbl, &check_char );

	if( check_char != '?' )
		err( YES, "Expected number, found `%s'", str );

	return( dbl );
}


long	LongNum( str )
char	*str;
{
double	dbl;

	dbl = DoubleNum( str );

	if( (int)dbl != dbl )
		err( YES, "Expected integer, found `%s'", str );

	return( (int)dbl );
}

/*	io.c			Paul Albrecht		March 1983

	Terminal and file I/O routines.

	eprintf()	Like printf().  Output error message with program name.
	estop()		Like eprintf() with exit(1) afterward.

	fofilen( name, mode )	Like fopen() but prints message if error.
	ofilen( name, mode )	Like open() but prints message if error.

	fofile( name, mode )	Like fofilen() but stops if error.
	ofile( name, mode )		Like ofilen() but stops if error.

	Note:	Both fofilen() and ofilen() treat name == 0 as a special error
			which assumes that the user forgot to initalize the name.
			Also, the names "stdin", "stdout", and "stderr" are special.
*/

#include	<stdio.h>

static FILE *checkname();

char	*program_name;		/* global program name variable */

#ifndef __STDC__
estop( fmt, args )
char *fmt;
{
char	*pterm, *getenv();

	pterm = getenv( "PTERM" );
	if( pterm != 0 && strcmp(pterm,"go140") == 0 ||
	 strcmp(pterm,"go235") == 0 || strcmp(pterm,"seltek") == 0 )
		eprintf( "\0332" );

	eprintf( "%s:\t", program_name );
        _doprnt(fmt, &args, stderr);
	putc( '\n', stderr );
	exit(1);
}


eprintf( fmt, args )
char	*fmt;
{
	_doprnt( fmt, &args, stderr );
	fflush( stderr );
}

#else
#include <stdarg.h>
void eprintf(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void)vfprintf(stderr, fmt, args);
    va_end(args);
    fflush( stderr );
}

estop(char *fmt, ...)
{
    char *pterm, *getenv();
    va_list args;

    pterm = getenv( "PTERM" );
    if( pterm != 0 && strcmp(pterm,"go140") == 0 ||
       strcmp(pterm,"go235") == 0 || strcmp(pterm,"seltek") == 0 )
	eprintf( "\0332" );

    eprintf( "%s:\t", program_name );
    
    va_start(args, fmt);
    (void)vfprintf(stderr, fmt, args);
    va_end(args);
    putc( '\n', stderr );
    exit(1);
}
#endif


FILE	*fofile( name, type )
char	*name, *type;
{
FILE	*fp, *fofilen();

	if( (fp=fofilen(name,type)) == 0 ) {
		eprintf( "Can't fopen %s\n", name );
		exit( 1 );
	}
	return( fp );
}


FILE	*fofilen( name, type )
char	*name, *type;
{
FILE	*fp, *checkname();

	if( (fp=checkname(name)) != 0 )
		return( fp );
	fp = fopen( name, type );
	return( fp );
}


ofile( name, oflag, mode )
char	*name;
{
int	fd;

	if( (fd=ofilen(name,oflag,mode)) == -1 ) {
		eprintf( "Can't open %s\n", name, oflag );
		exit( 1 );
	}
	return( fd );
}


ofilen( name, oflag, mode )
char	*name;
{
FILE	*fp, *checkname();
int	fd;

	if( (fp=checkname(name)) != 0 )
		return( fileno(fp) );
	fd = open( name, oflag, mode );
	return( fd );
}


static	FILE	*checkname( name )
char	*name;
{
	if( name == 0 )
		estop( "No name specified for required input file");

	if( strcmp(name,"stdin") == 0 )
		return( stdin );
	if( strcmp(name,"stdout") == 0 )
		return( stdout );
	if( strcmp(name,"stderr") == 0 )
		return( stderr );

	return( 0 );
}

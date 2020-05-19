/*	args.c			Paul Albrecht		Jan 1988
.				Last revised: 13 May 1995 (by GBM)
	Utility subroutines for command line argument processing.
	Options and their corresponding arguments are stripped out
	of argv[]; argc is adjusted appropriately.
*/


#include	<stdio.h>
#ifdef __STDC__
#include	<limits.h>
#else
#define	MIN(TYPE)	( -1*(1 << (sizeof(TYPE)*8-1)) )
#define	MAX(TYPE)	( -(MIN(TYPE)+1) )
#define	SHRT_MIN	MIN(short)
#define SHRT_MAX	MAX(short)
#define INT_MIN		MIN(int)
#define	INT_MAX		MAX(int)
#define LONG_MIN	MIN(long)
#define LONG_MAX	MAX(long)
#endif

#include	"args.h"

		
static int	*argcp,		/* pointer to argc in main program */
			argcc,		/* number of next argument to be processed */
			opt_mode,	/* 1=found new option, 2=test options, 3=matched option */
			help_mode;	/* 0=no help, 1=printing help but no option
						info printed yet, 2=printing option info */

static char	**argv, 	/* value of argv in the main program */
			*option,	/* name of option found in argv[] */
			*holding_arg;/* leftover chars; for 1 char options only  */




opt_init( argc_ptr, argv_value )	/* initialize argument processin */
int	*argc_ptr;
char	**argv_value;
{
int		i;

	argcp = argc_ptr;
	argv  = argv_value;
	argcc = 1;
	holding_arg = 0;
	program_name = argv[0];

	for( i=1;  i < *argcp;  i++ ) {
		if( strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0 ) {
			help_mode = 1;
			printf( "=========================================\n" );
			break;
		}
	}
}


#ifndef __STDC__
opt_help( fmt, args )
char	*fmt;
{
	if( help_mode ) {
		_doprnt( fmt, &args, stdout );
		printf( "\n" );
	}
}
#else
#include <stdarg.h>
opt_help(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    if( help_mode ) {
	vprintf(fmt, args);
	printf( "\n" );
    }
    va_end(args);
}
#endif

opt_get()
/* Parse the argument list and return a 1 if an option is found. */
{
int	i;

	if( help_mode ) {
		if( opt_mode > 0 ) {
			printf( "=========================================\n" );
			exit( 0 );
		}
	}
	else	if( opt_mode == 1 || opt_mode == 2 )
			estop( "Unrecognized option `-%s'", option );
	
	if( holding_arg )
		estop( "Garbage trailing option `-%c'", *option );

	opt_mode = 0;

	while( argcc < *argcp && opt_mode == 0 ) {
		option = argv[argcc];
		if(  option[0] == '-' && option[1] != 0 ) {
			for( i=argcc; i < *argcp;  i++ )
				argv[i] = argv[i+1];

			(*argcp)--;
			option++;
			opt_mode = 1;
		}
		else 	argcc++;
	}

	return( opt_mode );
}


opt_match( optname, help_string )	/* Return a 1 if *optname matches *option. */
char	*optname, *help_string;
{
	switch( opt_mode ) {
		case 1:	if( help_mode )
				printf( "\n" );
			opt_mode = 2;
			/* flow through */
		case 2:	if( help_mode && help_string != 0 ) {
				printf( "-%s\t", optname );
				while( *help_string != 0 && *help_string != '\3' ) {
					putchar( *help_string );
					help_string++;
				}
				if( *help_string != '\3' )
					putchar( '\n' );

				return( 0 );
			}
			if( strlen(optname) == 1 ) {
				if( optname[0] == option[0] ) {
					if( option[1] != 0 )
						holding_arg = option+1;
					option = optname;
					opt_mode = 3;
				}
			}
			else {	if( strcmp(optname,option) == 0 ) {
					option = optname;
					opt_mode = 3;
				}
			}

			return( opt_mode == 3 );
	}
	return( 0 );
}


static 	struct	{
	char	*name;
	short	type;
	size_t	size;
	short	integer;
	long	min, max;
	}	*t, types[] = {
	"short", TYPE_SHORT, sizeof(short), 1, SHRT_MIN, SHRT_MAX,
	"int", TYPE_INT, sizeof(int), 1, INT_MIN, INT_MAX,
	"long",	TYPE_LONG, sizeof(long), 1, LONG_MIN, LONG_MAX,
	"float", TYPE_FLOAT, sizeof(float), 0, 0, 0,
	"double", TYPE_DOUBLE, sizeof(double), 0, 0, 0 };


void	opt_argset( address, size, type )
char	*address;
{
double	dbl, floor();
char	line[120], check_char, *arg;

	arg = opt_arg();

	for( t=types; t->type != type ; t++ ) {
		if( t->type == 0 )
			estop( "Programming error on option `-%s'", option );
	}

	if( size != t->size )
		estop( "Program bug.  Bad variable type used for `-%s'", option );

	if( type == TYPE_STR_PTR ) {
		*(char **)address = arg;
		return;
	}

	check_char = 0;
	sprintf( line, "%s\007", arg );
	sscanf( line, "%lf%c", &dbl, &check_char );

	if( check_char != 7 )
		estop( "`-%s' expected a number, found `%s'", option, arg );

	if( t->integer && (floor(dbl) != dbl || dbl < t->min || dbl > t->max) )
		estop( "`-%s' expected type (%s), found `%s'", option, t->name, arg );

	switch( type ) {
		case TYPE_SHORT:	*(short *)address = dbl;	break;
		case TYPE_INT:		*(int *)address = dbl;		break;
		case TYPE_LONG:		*(long *)address = dbl;		break;
		case TYPE_FLOAT:	*(float *)address = dbl;	break;
		case TYPE_DOUBLE:	*(double *)address = dbl;	break;
	}
}


char	*opt_arg()
/* Return the address of string following the last detected option. */
{
int	i;
char	*argument;

	if( holding_arg != 0 ) {
		argument = holding_arg;
		holding_arg = 0;
	}
	else {	argument=argv[argcc];
		if( argument == 0 )
			estop( "Ran out of arguments for `-%s'", option );

		for( i=argcc; i < *argcp; i++ )
			argv[i] = argv[i+1];

		(*argcp)--;
	}

	return( argument );
}


double	opt_atof()	/* return the double value of the next arg */
{
double	d;

	opt_argset( (char *)&d, sizeof(d), TYPE_DOUBLE );
	return( d );
}


long	opt_atol()	/* return the long value of the next arg */
{
long	l;

	opt_argset( (char *)&l, sizeof(l), TYPE_LONG );
	return( l );
}


opt_atoi()	/* return the int value of the next arg */
{
int	i;

	opt_argset( (char *)&i, sizeof(i), TYPE_INT );
	return( i );
}

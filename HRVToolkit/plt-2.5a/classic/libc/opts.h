/*	opts.h			Paul Albrecht			April 1983

	Definitions for command line argument processing and other commonly
	needed program functions.
*/

#ifndef		COMMON

#ifndef		FILE
#include	<stdio.h>
#endif

#ifndef		HUGE
#include	<math.h>
#endif
#define		FHUGE		1e38

#define		YES		1
#define		NO		0

#ifdef		MAIN
#define		COMMON
#define		SVAL(X)		= X
#else
#define		COMMON		extern
#define		SVAL(X)
#endif

#define		ENDP(ARRAY)	(&ARRAY[sizeof(ARRAY)/sizeof(ARRAY[0])])

#define		BEGIN_OPTIONS	opt_init( &argc, argv );	\
				while( opt_get() ) { 		\

#define		HELP		;opt_help

#define		CASE( OPT, HELP_STRING, COMMAND )		\
			;if( opt_match(OPT,HELP_STRING) ) COMMAND;

#define		END_OPTIONS	;}

#define		ATOF		opt_atof()
#define		ATOL		opt_atol()
#define		ATOI		opt_atoi()
#define		ARG		opt_arg()

extern	FILE	*fofile(), *fofilen();
extern	double	atof(), opt_atof();
extern	long	atol(), opt_atol(), lseek();
extern	int		opt_atoi();
extern	char	*zmem(), *azmem(), *strsave(), *opt_arg(), *program_name,
				*get_zmem(), *add_zmem();

#endif

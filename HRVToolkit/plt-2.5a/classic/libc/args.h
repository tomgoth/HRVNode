/*	args.h			Paul Albrecht			Jan 1988

	Auxiliary definitions needed for command line argument processing.
	The basic definitions are included in file opts.h
*/

#ifndef		BEGIN_OPTIONS
#include	"opts.h"
#endif

#define		TYPE_SHORT		2
#define		TYPE_INT		3
#define		TYPE_LONG		4
#define		TYPE_FLOAT		5
#define		TYPE_DOUBLE		6
#define		TYPE_STR_PTR		7

#define		ARGSET( NAME, TYPE )			\
		opt_argset( (char *)&NAME, sizeof(NAME), TYPE)

#define		ICASE( OPT, HELP_STRING, INT )		\
		CASE( OPT, HELP_STRING, ARGSET(INT,TYPE_INT) )

#define		LCASE( OPT, HELP_STRING, LONG )		\
		CASE( OPT, HELP_STRING, ARGSET(LONG,TYPE_LONG) )

#define		SHCASE( OPT, HELP_STRING, SHORT )	\
		CASE( OPT, HELP_STRING, ARGSET(SHORT,TYPE_SHORT) )

#define		FCASE( OPT, HELP_STRING, FLOAT )	\
		CASE( OPT, HELP_STRING, ARGSET(FLOAT,TYPE_FLOAT) )

#define		DCASE( OPT, HELP_STRING, DOUBLE )	\
		CASE( OPT, HELP_STRING, ARGSET(DOUBLE,TYPE_DOUBLE) )

#define		SCASE( OPT, HELP_STRING, STR_PTR )		\
		CASE( OPT, HELP_STRING, ARGSET(STR_PTR,TYPE_STR_PTR) )

#define		HELP_DATA	HELP( "" )				\
		CASE( "D", "Output double precision", data_fmt(NULL) )	\
		CASE( "F", NULL, (data_fmt(ARG), data_ascii(YES)) )	\
		CASE( "P", "Output columns in ascii", data_ascii(YES) )

#define		DATA_OPTIONS	HELP_DATA

void	opt_argset();

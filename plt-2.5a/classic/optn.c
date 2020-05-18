/*	plt/optn.c		Paul Albrecht		Sept 1987

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4
*/

#include	"plt.h"
#include	"optn.h"
#include	<ctype.h>

#define		ISOPT(STR)	( (STR)[0] == '-' && isalpha((STR)[1]) )


SetPTERMSpecificOpt( specificPterm, options )
char	*specificPterm, *options;
{
PSOPtr	pso;

	if( npso == maxpsos )
		psos = (PSOPtr)azmem(psos,&maxpsos,10,sizeof(*psos));

	pso = &psos[npso++];
	pso->pterm = specificPterm;
	pso->options = options;
}


PTERMSpecificOpts()
{
int	n;

	for( n=0;  n < npso;  n++ ) {
		if( strcmp(psos[n].pterm,pterm) == 0 )
			StringOptions( psos[n].options );
	}
}


argvOpts( argv, argc )
char	*argv[];
int		argc;
{
OptPtr	o;
short	k, n, nflds;
char	*fld, *s[MAXFLDS], *getenv();

	n = 0;

	while( n < argc ) {
		while( n < argc && !ISOPT(argv[n]) )
			n++;

		if( n == argc )
			break;

		o = lookup2( argv, &n );
		if( o->mode & NOTARGV )
			err( YES, "Can't use `%s' on command line", o->name );

		nflds = 0;
		fgs = 0;
		while( n < argc && nflds < o->maxflds && !ISOPT(argv[n]) ) {
			fld = argv[n];
			if( o->fgnamep != 0 && nflds == 0 && (*fld == '(' || *fld == '[') )
				*o->fgnamep = fld;
			else
				s[nflds++] = fld;
			argv[n++]  = NULLS;
		}

		OneLiner( o, s, nflds );
	}

	for( k=n=0;  n < argc;  n++ )
		if( argv[n] )
			argv[k++] = argv[n];

	argv[k] = 0;
}


StringOptions( string )
char	*string;
{
	in_string( string, NO );
	Options();
}


FileOptions( name )
char	*name;
{
	in_file( name, NO );
	Options();
}


SetParams( o, s, nflds )
OptPtr	o;
char	*s[];
int	nflds;
{
PtrUnion	ptr;
short		n, nf, np, type, default_mode;

	if( o->flds == 0 )
		return;

	default_mode = NO;

	n = nf = np = 0;
	while( YES ) {
		type = o->flds[nf++];
		ptr.c = o->ptrs[np];
		if( type == 0 && ptr.c == 0 )
			break;
		if( type == 0 )
			err( YES, "Missing format spec for `%s'", o->name );
		if( ptr.c == 0 )
			err( YES, "Null otable address for `%s'", o->name );

		switch( type ) {
			case '-':
				default_mode = NO;
				break;
			case '+':
				default_mode = YES;
				break;
			case 'F':
				if( n < nflds && strcmp(s[n],"-") != 0 )
					*ptr.d = DoubleNum(s[n]);
				else {
					if( default_mode )
						*ptr.d = DEFAULT;
				}
				n++; np++;
				break;
			case 'l':
				if( n < nflds && strcmp(s[n],"-") != 0 )
					*ptr.l = LongNum(s[n]);
				else {
					if( default_mode )
						*ptr.l = DEFAULT;
				}
				n++; np++;
				break;
			case 's':
			case 'S':
				if( n < nflds && strcmp(s[n],"-") != 0  )
					*ptr.s = StringSave(s[n]);
				else {
					if( default_mode )
						*ptr.s = (type == 's') ? 0 : "";
				}
				n++; np++;
				break;
			case 'y':
				*ptr.b  = YES;
				np++;
				break;
			case 'n':
				*ptr.b  = NO;
				np++;
				break;
			default:
				err( YES, "Table err for `%s'", o->name );
		}
	}
}


optnHelp( otable, s, nflds )
OptPtr	otable;
char	*s[];
short	nflds;
{
OptPtr	o;
short	len, matches, n;
char	*name;

	if( nflds  > 0 ) {
		for( n=0;  n < nflds;  n++ ) {
			name = s[n];
			len = strlen( name );
			matches = 0;
			for( o=otable;  *o->name;  o++ ) {
				if( o->help && strncmp(name,o->name,len) == 0 ) {
					OneHelp( o );
					matches++;
				}
			}
			if( matches == 0 )
				err( NO, "No option begins with `%s'", name );
		}
	}
	else {	printf( "Use `-h xx' to get all options beginning with `xx'\n\n" );
		while( *otable->name ) {
			OneHelp( otable );
			otable++;
		}
		printf( "\nNote: a `-' argument leaves a field unchanged\n" );
	}
	pquit(0);
}


OneHelp( o )
OptPtr	o;
{
short	n, set, reset;
char	*fld;

	if( o->help == 0 )
		return;

	printf( "-%s\t", o->name );

	set = reset = n = 0;

	if( *o->help ) {
		printf( "%s\n", o->help );
	}
	else {	for( fld=o->flds;  fld && *fld;  fld++ ) {
	 	  switch( *fld ) {
			case 'y':	set++;		break;
			case 'n':	reset++;	break;
			case 'F':	printf( "(float) " );	n++;	break;
			case 'i':	printf( "(int) " );	n++;	break;
			case 's':	printf( "(string) " );	n++;	break;
	 	  }
		}
		if( set || reset ) {
			if( n > 0 )
				printf( "-- also " );
			if( set )
				printf( "sets flag" );
			if( reset )
				printf( "%sresets flag", set ? ", " : "" );
		}
		printf( "\n" );
	}
}

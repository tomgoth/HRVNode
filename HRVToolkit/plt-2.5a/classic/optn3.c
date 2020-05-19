/*	plt/optn3.c		Paul Albrecht		Sept 1987

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4
*/

#include	"plt.h"
#include	"optn.h"
#include	<ctype.h>


#define		MAXOLEV		6

#define		INFILE		01
#define		STRING		02


static	struct	{
	int	type;
	FILE	*fp;
	char	*sp;
	char	*name;
	short	line;
	Boolean	lock;
	}	input[MAXOLEV], *in;

static	short	nilev = 0;


in_file( name, lock )
char	*name;
Boolean	lock;
{
	if( nilev == MAXOLEV )
		err( YES, "Too many nested input streams" );

	in = &input[nilev];

	in->fp = fopen( name, "r" );
	if( in->fp == NULL )
		err( YES, "Can't read file `%s'", name );

	in->type = INFILE;
	in->name = name;
	in->line = 0;
	in->lock = lock;

	nilev++;
}


in_string( string, lock )
char	*string;
Boolean	lock;
{
	if( nilev == MAXOLEV )
		err( YES, "Too many nested input streams" );

	in = &input[nilev];

	in->type = STRING;
	in->sp   = string;
	in->name = "format string";
	in->line = 0;
	in->lock = lock;

	nilev++;
}


in_close( nlev )
short	nlev;
{
	if( nlev < -1 || nlev > nilev )
		nlev = nilev;

	while( nlev-- > 0 ) {
		nilev--;
		if( input[nilev].type == INFILE )
			fclose( input[nilev].fp );
	}

	in = &input[nilev-1];

	return( nilev );
}

/************************************************************************/

in_char()
{
short	c;

	if( nilev == 0 )
		return( EOF );

	switch( in->type ) {
		case	INFILE:	c = getc( in->fp );
				break;
		case	STRING:	c = *in->sp;
				if( c == 0 )
					c = EOF;
				else
					in->sp++;
				break;
	}

	if( c == EOF && !in->lock ) {
		in_close( (short)1 );
		return( in_char() );
	}
	else
		return( c );
}


in_unchar( c )
short	c;
{
	if( c != EOF ) {
		switch( in->type ) {
			case	INFILE:	ungetc( c, in->fp );
					break;
			case	STRING:	in->sp--;
					break;
		}
	}
}

/************************************************************************/

in_parse( s, maxflds, obj, maxchrs )
short	maxflds, maxchrs;
char	*s[], *obj;
{
short	status, nflds, nchrs, n;

	status = nflds = 0;
	while( nflds < maxflds && !(status & END_LINE) ) {
		status = in_obj( obj, maxchrs, IN_OBJ );
		if( status & HAVE_OBJ ) {
			if( nflds == maxflds )
				err( YES, "Too many fields" );

			s[nflds++] = StringSave( obj );
			nchrs = strlen( obj );
			obj += (nchrs + 1);
			maxchrs -= (nchrs + 1);
		}

	}

	if( optn ) {
		eprintf( "PARSE:" );
		for( n=0;  n < nflds;  n++ )
			eprintf( "  (%s)", s[n] );
		eprintf( "\n" );	
	}
	return( nflds );
}

/*********************************************************************/

in_obj( obj, maxchrs, mode )
char	*obj;
short	maxchrs;
Const	mode;
{
register short	nchrs, c;
short	 shield, quote, first, nbrack, status, more, outchar;

	shield = NO;
	more = first = YES;
	status = nbrack = quote = nchrs  = 0;

	if( mode & DEL_SPACE ) {
		do {	c = in_char();
			switch( c ) {
				case '\t':
				case ' ':	break;
				case '\n':	if( !(mode & SKIP_CR) )
							more = NO;
						break;
				default:	more = NO;
						break;
			}
		} while( more && c != EOF );
	}
	else
		c = in_char();

	do {
		outchar = NO;
		switch( (c!= EOF && shield) ? 0 : c ) {
			case  0:	if( quote != 0 ) {
						if( quote == c ) {
							shield = NO;
							quote = 0;
						}
						else
							outchar = YES;
					}
					else {
						shield = NO;
						outchar = YES;
					}
					break;
			case '\\':	if( !(mode & VERBATIM) )
						shield = YES;
					break;
			case '"':
			case '\'':	if( !(mode & VERBATIM) && quote == 0 ) {
						shield = YES;
			 			quote = c;
					}
					break;
			case '{':	nbrack++;
					break;
			case '}':	if( nbrack == 0 )
						err( NO, "Stray '}' -- line %d of %s\n", in->line, in->name );
					if( --nbrack == 0 )
						status |= END_INPUT;
					break;
			case ';':	if( mode & SEMI_TERM ) {
						if(  mode & SAVE_TERM )
							in_unchar( c );
						status |= END_LINE;
					}
					else
						outchar = YES;
					break;
			case '\n':	if( !(mode & SKIP_CR) ) {
						if( mode & SAVE_TERM )
							in_unchar( c );
						status |= END_LINE;
					}
					break;
			case '\t':
			case ' ':	if( mode & SPACE_DELIM )
						status |= HAVE_OBJ;
					else
						outchar = YES;
					break;
			case EOF:	status |= (END_LINE|END_INPUT);
					break;
			default:	outchar = YES;
					break;
		}

		if( outchar ) {
			if( maxchrs > 1 ) {
				obj[nchrs++] = c;
				maxchrs--;
			}
			else {
				if( first ) {
					err( NO, "Line `%.25s...' too long", obj );
					first = NO;
				}
			}
		}

		if( status == 0)
			c = in_char();
	} while( status == 0 );

	if( nbrack > 0 )
		err( NO, "Missing '}' for `%.10s...'\n", obj );

	if( nchrs > 0 )
		status |= HAVE_OBJ;
	obj[nchrs] = 0;
	if( optn )
		eprintf( "OBJECT: (%s)\n", obj );

	if( quote )
		err( NO, "Unmatched quote in format specification" );

	return( status );
}

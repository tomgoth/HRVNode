/*	plt/text.c		Paul Albrecht		Sept 1987

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4
*/

#include	"plt.h"
#include	"axis.h"
#include	"text.h"
#include	"optn.h"


TextInit( psdel )	/* initalize the font groups a, f, l, p and t */
{
char	changes[MAXLINE];

	sprintf( changes, "P%g,W3,Ea", 14.0+psdel );
	FontGroupDef( "a", changes );
	sprintf( changes, "P%g,W3,Ef", 14.0+psdel );
	FontGroupDef( "f", changes );
	sprintf( changes, "P%g,W3,El", 15.0+psdel );
	FontGroupDef( "l", changes );
	sprintf( changes, "P%g,W2,Ep", 12.0+psdel );
	FontGroupDef( "p", "P12,W2,Ep" );
	sprintf( changes, "P%g,W4,Et", 16.0+psdel );
	FontGroupDef( "t", changes );
}


MakeGraphTitle( argv )
char	*argv[];
{
long	time(), seconds;
Ptype	n, x, y, xsize, ysize;
char	*ptr, *ctime(), *getenv(), str[50], titleText[120];

	if( !(omode & TITLES) )
		return;
			
	if( title.text == 0 ) {
		titleText[0] = 0;
		while( *argv ) {
			strcat(titleText, *argv++);
			strcat(titleText, " " );
		}
		if( strlen(titleText) > 0 )
			strcat( titleText, "-- <" );

		ptr = getenv( "LOGNAME" );
		if( ptr ) {
			strcat( titleText, ptr );
			strcat( titleText, "-" );
		}

		time( &seconds );
		ptr = ctime( &seconds );
		n = (ptr[8] == ' ') ? 9 : 8;
		sprintf( str, "%.5s-%.4s%d>",
			&ptr[11], &ptr[4], atoi(&ptr[n]) );
		strcat( titleText, str );
		title.text = titleText;
	}
	else
		if( strlen(title.text) == 0 )
			return;

	FontGroupSelect( "t", title.fgName );

	x = (xwmin + xwmax)/2;
	if( theight == 0 ) {
		strsize( title.text, &xsize, &ysize, 0.0 );
		y = ywmax + (int)((yfract+0.4)*ysize + yfract*yinch/12);
		title.just = "CB";
	}
	else
		y = ywmin + (int)((ywmax-ywmin)*theight);

	plabel( title.text, x, y, title.just, 0.0 );
}

/****************** Output user defined labels ***********************/

TextDraw( l )
LblPtr	l;
{
short	n, nc, x, y, xsize, ysize;
char	c, lbl[MAXLINE];

	if( l->text == 0 || *l->text == 0 )
		return;

	FontGroupSelect( "l", l->fgName );
	transform( &x, &y, l->xpos, l->ypos, 0.5, l->coord );

	nc = 0;
	c = 1;
	while( c ) {
		n = 0;
		do {	c = l->text[nc++];
			lbl[n++] = (c == '\n') ? 0 : c;
		} while( c && c != '\n');

		plabel( lbl, x, y, l->just, l->angle );
		strsize( lbl, &xsize, &ysize, l->angle );
		if( fabs(l->angle) < 1 )
			y -= ysize;
		else	x += xsize;
	}
}

/******* Read plotting labels for text scatterplots **********/

ReadStrings( ps )	/* can be called to add to ps also */
PStrPtr	ps;
{
FILE	*fp;
short	k;
char	c, *str, line[200];

	if( ps->file ) {
		fp = fopen( ps->str, "r" );
		if( fp == 0 )
			err( YES, "Can't open string file %s", ps->str );
	}
	else	str = ps->str;

	while( YES ) {
		if( ps->file ) {
			if( fgets(line,sizeof(line),fp) == NULLS )
				break;
			k = strlen(line);
			if( line[k-1] == '\n' )
				line[k-1] = 0;
		}
		else {	if( *str == 0 )
				break;
			for( k=0;  (c=str[k]) && c != ' ' && c != '\n'; k++ )
				line[k] = c;
			line[k] = 0;
			str += (c == 0) ? k : k+1;
		}
		if( ps->n == ps->nmax )
			ps->list = (char **)azmem(ps->list,&ps->nmax,50,sizeof(*ps->list) );
		ps->list[ps->n++] = StringSave( line );
	}

	ps->just = (ps->just == 0) ? "CC" : JustMap( ps->just );

	if( ps->file )
		fclose( fp );
}

/******** Map user specified justification to a two character string *********/

char	*JustMap( userJust )
char	*userJust;
{
static	char	just[3];
short	n;

	for( n=0;  n < 2;  n++ ) {
		just[n] = (userJust[n] == 0) ? 0 : userJust[n];
		if( just[n] >= 'a' && just[n] <= 'z' )
			just[n] &= ~040;
	}

	if( just[0] == 'C' && just[1] == 0 )
			return( "CC" );		
	else	if(  just[1] == 0 && (just[0] == 'L' || just[0] == 'D') )
			return( "LB" );		
	else	if(  just[1] == 0 && (just[0] == 'R' || just[0] == 'U') )
			return( "RB" );		

	return( StringSave(just) );
}

/*************** Set font type, point size, etc ******************/

FontGroupSelect( dflt_fgName, fgName )
char	*dflt_fgName, *fgName;
{
PtrUnion	arg0, arg1;
FgPtr		fg;
double		scl;

	if( fgName != 0 ) {
		if( (fg=FontGroupFind(fgName,NO)) == 0 )
			/* use fgName as changes to make to default fgroup */
			fg = FontGroupCat( dflt_fgName, fgName );
	}
	else {
		if( dflt_fgName == 0 )
			err( YES, "Bad call to FontGroupSelect()" );
		fg = FontGroupFind( dflt_fgName, YES );
	}

	arg0.c = fg->gelem;
	special( GRAPHELEMENT, arg0, arg1 );

	if( strcmp(old_lm,fg->lm) != 0 ) { /* must precede to point size set */
		arg0.c = fg->lm;
		special( SETLINEMODE, arg0, arg1 );
	}
	arg0.c = fg->color;
	special( SETCOLOR, arg0, arg1 );

	if( oldGrayLevel < 0 || oldGrayLevel != fg->gray ) {
		oldGrayLevel = fg->gray;
		arg0.d = &oldGrayLevel;
		special( SETGRAY, arg0, arg1 );
	}
	if( old_lw < 0 ||  old_lw != fg->lw ) {
		old_lw = fg->lw;
		arg0.d = &old_lw;
		special( SETLINEWIDTH, arg0, arg1 );
	}

	if( fixed_font || (old_font != 0 &&
		strcmp(fg->font,old_font) == 0 && fg->ps == old_ps) )
			return;

	scl  = fg->ps/default_ps;
	chht = p->ch * scl + 0.5;
	chwd = p->cw * scl + 0.5;

	old_font = fg->font;
	old_ps = fg->ps;
	arg0.c = old_font;
	arg1.d = &old_ps;
	special( SETFONT, arg0, arg1 );
}

/****** Define the parameters for a font group ********/

FontGroupDef( fgName, specs )
char	*fgName, *specs;
{
FgPtr	fg;

	if( specs == 0 )
		return;

	if( fgName == 0 || strcmp(fgName,"all") == 0 ) {
		FontGroupDef( "a", specs );
		FontGroupDef( "f", specs );
		FontGroupDef( "l", specs );
		FontGroupDef( "p", specs );
		FontGroupDef( "t", specs );
	}	
	else {	if( (fg=FontGroupFind(fgName, NO)) == 0 )
			fg = FontGroupInit( fgName );

		FontGroupModify( fg, specs );
	}
}

/****** Create a new font group as a modified version of an old one ******/

FgPtr	FontGroupCat( fgName, changes )
char	*fgName, *changes;
{
FgPtr	fg, FontGroupInit();
char	newfgName[MAXLINE];

	sprintf( newfgName, "%s%s", fgName, changes );
	fg = FontGroupFind( newfgName, NO );
	if( fg != 0 )
		return( fg );

	fg = FontGroupInit( newfgName );
	*fg = *FontGroupFind( fgName, YES );
	fg->name = StringSave( newfgName );

	FontGroupModify( fg, changes );
	return( fg );
}

/***** Modify an existing font group according to the specs in `changes' ****/

FontGroupModify( fg, changes )
FgPtr	fg;
char	*changes;
{
FgPtr	fgnew;
short	errFlag;
char	*cp, *fgName;

	errFlag = NO;
	cp = changes;

	if( *cp == '(' || *cp == '[' )
		cp++;

	while( *cp ) {
		if( *cp == ')' || *cp == ']' ) {
			cp++;
			while( *cp == ' ' )
				cp++;
			if( *cp == 0 )
				break;
			/* go get the error */
		}
		switch( *cp++ ) {
			case ' ':
			case ',':
			case '\t':
				break;
			case GCOLOR:
				errFlag |= !getstr( &cp, &fg->color );
				break;
			case GELEMENT:
				errFlag |= !getstr( &cp, &fg->gelem );
				break;
			case GFONT:
				errFlag |= !getstr( &cp, &fg->font );
				break;
			case GGRAY:
				errFlag |= !getdbl(&cp,&fg->gray);
				if( fg->gray > 1)
					fg->gray = 1;
				break;
			case GLINEMODE:
				errFlag |= !getstr( &cp, &fg->lm );
				checklm( fg );
				break;
			case GPS:
				errFlag |= !getdbl( &cp, &fg->ps );
				break;
			case GLINEWIDTH:
				errFlag |= !getdbl( &cp, &fg->lw );
				break;
		   default:
				cp--;
				errFlag |= !getstr( &cp, &fgName );
				fgnew = FontGroupFind( fgName, NO );
				if( fgnew == 0 )
					errFlag = YES;
				else {
					fgName = fg->name;
					*fg = *fgnew;
					fg->name = fgName;
				}
				break;
		}

		if( errFlag ) {
			err( YES, "Font group syntax error `%s'", changes );
			break;
		}
	}
}


FgPtr	FontGroupFind( fgName, mustFind )
char	*fgName;
Boolean	mustFind;
{
FgPtr	fg;
short	n;
	
	for( n=0;  n < nfgrps;  n++ ) {
		fg = &fgrps[n];
		if( strcmp(fg->name,fgName) == 0 )
			return( fg );
	}
	if( mustFind )
		err( YES, "No font group `%s'", fgName );
	return( 0 );
}


FgPtr	FontGroupInit( fgName )
char	*fgName;
{
FgPtr	fg;

	if( nfgrps == maxfgrps )		
		fgrps = (FgPtr)azmem(fgrps,&maxfgrps,6,sizeof(*fgrps));

	fg = &fgrps[nfgrps++];
	fg->name = fgName;
	fg->font = DEFAULT_FONT;
	fg->ps = DEFAULT_PS;
	fg->lw = DEFAULT_LW;
	fg->gray = PS_BLACK;
	fg->lm = "solid";
	fg->color = "";
	fg->gelem = "";
	return( fg );
}


getstr( cpp, strp )
char	**cpp, **strp;
{
short	n;
char	*cp, str[MAXLINE];

	while( **cpp == ' ' )
		(*cpp)++;
	n = 0;
	cp = *cpp;
	while( *cp ) {
		if( *cp == ' ' || *cp == ',' || *cp == ')' || *cp == ']' )
			break;
		str[n++] = *cp++;
	}
	str[n] = 0;

	if( *cpp != cp ) {
		*cpp = cp;
		*strp = StringSave( str );
		return( YES );
	}
	else	return( NO );

}


getdbl( cpp, dblp )
char	**cpp;
double	*dblp;
{
double	dbl, dec;
char	*cp, operation;

	while( **cpp == ' ' )
		(*cpp)++;
	dbl = 1e-6;
	cp = *cpp;

	if( *cp == '+' || *cp == '-' || *cp == '*' || *cp == '/' )
		operation = *cp++;
	else
		operation = 0;

	while( *cp >= '0' && *cp <= '9' ) {
		dbl = 10*dbl + ((*cp++ -'0')&0177);
	}

	if( *cp == '.' ) {
		dec = .1;
		cp++;
		while( *cp >= '0' && *cp <= '9' ) {
			dbl += dec * ((*cp++ -'0')&0177);
			dec *= 0.1;
		}
	}

	if( *cpp != cp ) {
		*cpp = cp;
		switch( operation ) {
			case '+':	*dblp += dbl;	break;
			case '-':	*dblp -= dbl;	break;
			case '*':	*dblp *= dbl;	break;
			case '/':	*dblp /= dbl;	break;
			default:	*dblp = dbl;	break;
		}
		return( YES );
	}
	else	return( NO );
}


checklm( fg )
FgPtr	fg;
{
static	struct	{
	char	*name;
	short	n;
	}	*l, lm[] = {	/* the first 5 must appear first !!!!!!! ... */
	"solid", 0, "dotted", 1, "shortdashed", 2,
	"dotdashed", 3, "longdashed", 4, 
	"0", 0, "1", 1, "2", 2, "3", 3, "4", 4, "5", 5,
	"dash", 2 };
short	n;

	for( l=lm;  l < ENDP(lm);  l++ ) {
		n = strlen(fg->lm);
		if( strcmp(l->name,fg->lm) == 0 || strncmp(l->name,fg->lm,n) == 0 ) {
			fg->lm = lm[l->n].name;	/* ... and this is why !!!!!!!! */
			return;
		}
	}
	err( YES, "No linemode type `%s'", fg->lm );
}

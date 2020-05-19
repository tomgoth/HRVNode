/*	plt/text.h		Paul Albrecht		Sept 1984

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4

	All output falls under the domain of a font group.  There
	are 5 default font groups:

		a	axes
		f	figures and legends
		l	labels
		p	plots
		t	x, y, and graph title

	These font groups can be modified, or new font groups can be
	created using the `sf' option.  Note that each font group
	contains a field (gelem) which tells what part of the graph
	it belongs to.  This field is not used by plt, but can be used
	it the plot driver if it wants to handle certain parts of the
	graph differently.  The default value of gelem is the name of
	of the font group.
*/

#define		GCOLOR 		'C'
#define		GELEMENT	'E'
#define		GFONT		'F'
#define		GGRAY		'G'
#define		GLINEMODE	'L'
#define		GPS			'P'
#define		GSYMBOL		'S'
#define		GLINEWIDTH	'W'


typedef	struct	{
	char	*name;		/* name of this font group 			*/
	char	*font;		/* name of the font 				*/
	double	ps;			/* point size of the font group		*/
	double	lw;			/* line width for the font group	*/
	double	gray;		/* gray scale:  0=black, 1=white	*/
	char	*lm;		/* line mode associated with the group	*/
	char	*color;		/* color of the font				*/
	char	*gelem;		/* where on the graph this fgroup is used */
	}	FgInfo, *FgPtr;


/*	Label justification is coded as two character string.  In general, 
	the first character (C, R or L) specifies the justification in the
	axis of the text, while the second specifies the justification
	(C, T, or B) in the perpendicular direction.
*/

typedef	struct	{
	char	*text;		/* label string; '\n' separates lines	*/
	char	*fgName;	/* font group to use in printing label	*/
	char	*just;		/* justification mode for the label	*/
	double	xpos;		/* x position in window coordinates	*/
	double	ypos;		/* y position in window coordinates	*/
	double	angle;		/* angle at which to print the label	*/
	Const	coord;		/* coordinate system for position	*/
	}	LblInfo, *LblPtr;

/*	Information about labels which are to be used for plotting with
	the LABEL_N plot type is in struct psinf
*/

typedef	struct	{
	char	*str;		/* string to read strings from */
	char	**list;		/* argv[] type list of string pointers */
	char	*just;		/* justification to use in printing strings */
	Boolean	file;		/* file to read strings from */
	Uint	n;			/* number of strings in list */
	Uint	nmax;		/* max possible strings in list */
	}	PStrInfo, *PStrPtr;


COMMON	FgPtr	fgrps;

COMMON	LblPtr	lbls;

COMMON	PStrInfo	pstr, fgstr;

COMMON	double	theight;

COMMON	Uint	nlbls, maxlbls, nfgrps, maxfgrps;

COMMON	LblInfo	title;
COMMON	char	*JustMap();


FgPtr	PROTO( FontGroupFind, (char *, Boolean) );
FgPtr	PROTO( FontGroupCat, (char *, char *) );
FgPtr	PROTO( FontGroupInit, (char *) );


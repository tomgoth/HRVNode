/*	plt/axis.h		Paul Albrecht		Feb 1988

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Last Update:	May 21, 1989
	EMACS_MODES:	tabstop=4
*/


typedef struct	{
	char	name;	/* 'x' or 'y'	*/
	double	min;	/* lower limit	*/
	double	max;	/* upper limit	*/
	double	cr;		/* where this axis crosses the other axis	*/
	double	aoff;	/* offset from other axis (alternative to cr)	*/
	double	mlt;	/* if != DEFAULT make axis and tics = 0 mod mlt	*/
	double	tic;	/* spacing between tic marks			*/
	double	tmark;	/* a tic which must be maked and labeled	*/
	double	tscl;	/* scale factor for tic mark length		*/
	long	skp;	/* label only every `skp'th tick mark		*/
	char	*pfm;	/* format (e.g. %4.2f) for outputing tic values	*/
	char	*lbl;	/* label for the axis	*/
	char	*base;	/* base for log plots	*/
	short	mode;	/* what to show		*/
	Boolean	logflg;	/* YES for log axis	*/
	Boolean	rev;	/* put reverse the axis tics and labels		*/
	double	scl;	/* scaling from user to device coordinates	*/
	double	off;	/* offset from user to device coordinates	*/
	double	acchi;	/* how much the xmax can be increased		*/
	double	acclo;	/* how much the xmin can be decreased		*/
	char	*numfg;	/* name of font group to use axis numbers	*/
	char	*lblfg;	/* name of font group to use for axis label	*/
	char	*extra;
	Ptype	lo;		/* same as cr but in plot device coordinates	*/
	Ptype	hi;		/* opposite end from lo				*/
	Ptype	(*this)(); /* subr for this axis device coordinate	*/ 
	Ptype	(*other)();/* subr for the other axis device coordinate	*/
	}	AxisInfo, *AxisPtr;

typedef	struct	{
	AxisPtr	a;
	double	tic;
	double	scl;
	char	*lbl;
	}	TicInfo, *TicPtr;



void	PROTO( AxisInit, (Mode) );
void	PROTO( TicInit, (Mode) );
Ptype	PROTO( X, (double) );
Ptype	PROTO( Y, (double) );



COMMON	AxisInfo	xa, ya;

COMMON	TicPtr	tics;

COMMON	double	xfract, yfract,
		xmin, xmax, ymin, ymax,
		xwmins, xwmaxs, ywmins, ywmaxs;


COMMON	Boolean	ticlogic;

COMMON	Ptype	xwmin, xwmax, ywmin, ywmax;

COMMON	Uint	ntics, maxtics;

COMMON	char	*axisfile, *gridfg, *gridtype;

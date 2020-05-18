/*	unixpl.c		Paul Albrecht		Feb 1988

	Last Edit:	Oct 9, 1988

	Copyright (C) Paul Albrecht 1988.  All rights reserved.

	Copyright (C) Paul Albrecht 1988.  All rights reserved.
	Supplementary routines to a standard Unix plot library.
	These subroutines when loaded with iface.o and a plot
	library should be sufficient for plt.  For any new device,
	a new entry should be put in file pterm.c.
*/

#include	"plt.h"



special( what, arg0, arg1 )
PtrUnion	arg0, arg1;
{
	switch( what ) {
	   case SETPTERM:
		PTERMLookup( arg0.c, NULLS );
		break;
	   case SETCOLOR:
		/* arg0.c is pointer to string with color name */
		break;
	   case SETLINEMODE:
		linemod( arg0.c );
		break;
	}
}

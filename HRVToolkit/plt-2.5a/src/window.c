/* file: window.c	Paul Albrecht		September 1984
			Last revised:		12 April 2001
Subwindow definitions for plt

Copyright (C) Paul Albrecht 1988

Recent changes (by George Moody, george@mit.edu):
  12 April 2001: added input checking in WindowDef, rewrote SuppressionDef
		 to avoid buffer overflow, general cleanup
_______________________________________________________________________________
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA.

You may contact the maintainer by e-mail (george@mit.edu) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________
*/

#include "plt.h"

/* Prototypes of functions defined in this module. */
void WindowDef(char *config, char *subsect);
void SuppressionDef(char *str);
void gsuppress(void);

static void turnoff(AxisPtr a, Const flag);

/* Public functions */

void WindowDef(char *config, char *subsect)
{
    static float
	mgrid[] = {0, .935, 1, .935,   0, .941, 1, .941,   -1 },
	mwin[]  = {.12, .1, .94, .85 },
	*msgrid = mgrid,
	mswin[]  = {.2, .17, .92, .85 },
	bgrid[] = {0, .935, 1, .935,   0, .941, 1, .941,
		   0, .463, 1, .463,   0, .468, 1, .468,   -1 },
	bwin[]  = {.074, .53, .96, .88,
		   .074, .06, .96, .41 },
	*bsgrid = bgrid,
	bswin[] = {.1, .53, .945, .85,
		   .1, .07, .945, .39 },
	qgrid[] = { 0, .935, 1, .935,   0, .941, 1, .941,
		    0, .463, 1, .463,   0, .468, 1, .468,
		   .498, 0, .498, .94,  .502, 0, .502, .94,   -1},
	qwin[]  = {.074, .53, .454, .88,   .58, .53, .96, .88,
		   .074, .06, .454, .41,   .58, .06, .96, .41 },
	qpgrid[] = { 0, .935, 1, .935,   0, .941, 1, .941,
		     0, .463, 1, .463,   0, .468, 1, .468,
		    .498, 0, .498, .94,  .503, 0, .503, .94,  -1},
	qpwin[]  = {.08, .54, .45, .87,    .59, .54, .96, .87,
		    .08, .07, .45, .40,    .59, .07, .96, .40 },
	*qsgrid = qgrid,
	qswin[] = {.1, .53, .435, .85,     .6, .53, .945, .85,
		   .1, .07, .435, .39,     .6, .07, .945, .39 };
    float *wl, *gr;
    char subsectmax;

    if (config == 0)
	config = "m";
    if (subsect == 0)
	subsect = "0";

    if (strcmp(config,"m") == 0) {
	gr = mgrid;
	wl = mwin;
	title.text = "";
	subsectmax = '1';
    }
    else if (strcmp(config,"ms") == 0) {
	gr = msgrid;
	wl = mswin;
	title.text = "";
	subsectmax = '1';
    }
    else if (strcmp(config,"b") == 0) {
	gr = bgrid;
	wl = bwin;
	subsectmax = '2';
    }
    else if (strcmp(config,"bs") == 0) {
	gr = bsgrid;
	wl = bswin;
	subsectmax = '2';
    }
    else if (strcmp(config,"q") == 0) {
	gr = qgrid;
	wl = qwin;
	subsectmax = '4';
    }
    else if (strcmp(config,"qs") == 0) {
	gr = qsgrid;
	wl = qswin;
	subsectmax = '4';
    }
    else if (strcmp(config,"qp") == 0) {
	gr = qpgrid;
	wl = qpwin;
	subsectmax = '4';
    }
    
    if (*subsect < '1' || *subsect > subsectmax) {
	xa.min = ya.min = xmin = ymin = xwmins = ywmins = 0;
	xa.max = ya.max = xmax = ymax = xwmaxs = ywmaxs = 1;
	
	theight = 0.97;
	title.just = "CC";
	
	while (*gr >= 0) {
	    FigureDef(CONNECT, WINC, gr[0], gr[1], gr[2], gr[3], NULL);
	    gr += 4;
	}
	FigureDef(BOX, WINC, 0.0, 0.0, 1.0, 1.0, NULL);
	SuppressionDef("xy");
    }
    else {
	wl += (*subsect-'1') * sizeof(*wl);
	xwmins = wl[0];
	ywmins = wl[1];
	xwmaxs = wl[2];
	ywmaxs = wl[3];
	omode &= ~ERASE;
	title.just = "CB";
	theight = 1.07;
	if (config[0] != 'm')
	    TextInit(-1);
	SuppressionDef("e");
	xa.lbl = "";
    }
}

/* Suppress the generation of parts of the graph */
void SuppressionDef(char *str)
{
    size_t len1 = Plot.suppress ?  strlen(Plot.suppress) : 0;
    size_t len2 = str ? strlen(str) : 0;
    char *ptr;

    ptr = zmem(len1 + len2 + 1, 1);
    if (Plot.suppress) {
	strcpy(ptr, Plot.suppress);
	free(Plot.suppress);
    }
    if (str)
	strcpy(ptr+len1, str);
    Plot.suppress = ptr;
}

void gsuppress(void)
{
    AxisPtr a;
    char chr;

    a = 0;
    if (Plot.suppress)
	while (chr = *Plot.suppress++) {
	    switch (chr) {
	    case 'a': turnoff(a, AXIS); break;
	    case 'e': turnoff(a, ERASE); break;
	    case 'f': turnoff(a, FIGURES); break;
	    case 'g': turnoff(a, GRIDMARKS); break;
	    case 'm': turnoff(a, TICKMARKS); break;
	    case 'n': turnoff(a, TICKNUMS); break;
	    case 'l': turnoff(a, LABELS); break;
	    case 'p': turnoff(a, PLOTS); break;
	    case 't': turnoff(a, TITLES); break;
	    case 'x': xa.mode = 0; break;
	    case 'y': ya.mode = 0; break;
	    case 'C': omode = xa.mode = ya.mode = 077777; a = 0; break;
	    case 'A': a = 0; break;
	    case 'X': a = &xa; break;
	    case 'Y': a = &ya; break;
	    default: err(NO, "Bad element `%c' for suppress", chr);
	    }
	}
}

/* Private functions (callable only from within this module) */

static void turnoff(AxisPtr a, Const flag)
{
    short mask = ~flag;

    if (a == 0)	omode &= mask;
    if (a == 0 || a == &xa) xa.mode &= mask;
    if (a == 0 || a == &ya) ya.mode &= mask;
}

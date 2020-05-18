/* file: main.c		Paul Albrecht		September 1984
			Last revised:	       21 October 2002
plt: a set of programs for making 2D plots

Copyright (C) Paul Albrecht 1988

First version written in 1983.  Rewritten in 1984, 1986, and 1988.

Recent changes (by George Moody, george@mit.edu):
  11 April 2001: general cleanup
  12 April 2001: removed vestigial TicInit()
  21 October 2002: more cleanup
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

This main() function can be compiled with different driver modules
to produce a device-specific version of `plt'.  Currently there are
only two actively maintained drivers (xw.c, for X11; and lw.c, for
PostScript;  unmaintained drivers for a variety of mostly obsolete
devices are available in `../classic').  Refer to `Makefile' to see
how `xplt' and `lwplt' are compiled from these and other sources
in this directory.

Caution: The order of the various calls made here is quite important,
since some functions count on the environment being set up by the time
they are called.
*/

#define MAIN		/* see plt.h */
#include <stdlib.h>
#include <signal.h>
#include "plt.h"

int main(int argc, char **argv)
{
  PtrUnion arg0, arg1;
  FILE *fp;
  short	n;

  programName = argv[0];
  UtilInit(fullInit);
  TextInit(0);		/* must come before argument processing */
  FigureInit(fullInit);
  AxisInit(fullInit);
  PlotInit(fullInit);

  argvOpts(argv+1, argc-1);

  if (pterm == NULL) pterm = getenv("PTERM");
  if (pterm == NULL) pterm = "";
  PTERMSpecificOpts();
  arg0.c = pterm;
  special(SETPTERM, arg0, arg1);	/* must follow PTERMSpecificOpts() */

  DataInit(argv);

  if (Data.nCols != 0) PlotDef(Plot.pModes);
  if (pstr.str != 0) ReadStrings(&pstr);
  if (fgstr.str != 0) ReadStrings(&fgstr);

  gsuppress();

  if (xa.min == DEFAULT || xa.max == DEFAULT ||
      ya.min == DEFAULT || ya.max == DEFAULT) {
    if (Data.nCols == 0) err(YES, "Must specify axis limits");
  }
  else if (Plot.nPlts == 1) {
    Plot.quickPlot = YES;
    if (xa.lbl == 0) xa.lbl = "";
  }

  if (Data.nCols != 0 && !Plot.quickPlot) ReadPoints();

  SetupAxes();

  if (axisfile) {
    fp = fopen(axisfile, "w");
    if (fp == 0)
      err(YES, "Can't open %s for writing axis specs", axisfile);
    fprintf(fp, "xa %g %g %g %s %d %g\n",
	    xa.min, xa.max, xa.tick, xa.pfm, xa.skp, xa.cr);
    fprintf(fp, "ya %g %g %g %s %d %g\n",
	    ya.min, ya.max, ya.tick, ya.pfm, ya.skp, ya.cr);
    fprintf(fp, "W %g %g %g %g\n", xwmins, ywmins, xwmaxs, ywmaxs);
    if (fileno(fp) > 2)	/* i.e., if fp does not point to stdin/out/err */
      fclose(fp);
  }

  pinit();
  space(0, 0, p->xsquare, p->yfull);
  if (omode & ERASE) erase();
  if (xa.mode) XAxisDraw();
  if (ya.mode) YAxisDraw();
  MakeGraphTitle(argv+1);

  /* The ordering below is important for PostScript output. */
  if (omode & PLOTS)
    for (n = 0; n < Plot.nPlts; n++)
      PlotDraw(&Plot.plts[n]);

  if (omode & FIGURES) {
    if (Figure.nLegs > 0)
      LegendDraw();
    for (n = 0; n < Figure.nFigs; n++)
      FigureDraw(&Figure.figs[n]);
  }

  if (omode & LABELS)
    for (n = 0; n < Figure.nLbls; n++)
      TextDraw(&Figure.lbls[n]);

  if (Plot.excluded)
    err(NO, "*** Excluded %ld points ***", Plot.excluded);

  pquit(0);
  return (0);	/* This is only here to satisfy some compilers -- pquit doesn't
		   return, so we never get here. */
}

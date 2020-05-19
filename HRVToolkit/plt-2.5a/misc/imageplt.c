/* file: imageplt.c     G. Moody         3 November 1991
					10 December 2010
Read image data and generate `plt' input

Usage:
    imageplt -d NROWS NCOLS -n [-x XLOW XHIGH] [-y YLOW YHIGH] [IFILE]
  where
    NROWS and NCOLS are the number of rows and columns in the image
    XLOW and XHIGH are the minimum and maximum abscissas (default: 0, NROWS-1)
    YLOW and YHIGH are the minimum and maximum ordinates (default: 0, NCOLS-1)
    IFILE is the name of the image input file (default: standard input)
The image file contains grey levels for each pixel (0 = white, 1 = black).
Each entry is an ASCII-coded decimal floating point number, separated from
adjacent entries by whitespace (one or more spaces, tabs, or newlines).
The first NROWS entries are the grey levels for column 0 of the image, botttom
to top, and each successive column from left to right of the image follows.
If ROWS is small, it may be convenient to arrange the image file in columns
and rows corresponding to those of the image, but this is not necessary.  In
no case should the length of a line of input exceed MAXLEN bytes (defined
below).

The -n option is used to generate a negative image (1 = white, 0 = black).

The output is in three columns, to be plotted using the -pc option of `plt',
e.g.,
    imageplt -d 10 10 foo | plt 0 1 2 -pc
*/

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#endif

#define MAXLEN 50000

FILE *ifile = NULL;

main(argc, argv)
int argc;
char *argv[];
{
    int i, j, nrows = 0, ncols = 0, nflag = 0;
    double xlow = 0., xhigh = 0., xinc, ylow = 0., yhigh = 0., yinc, x, y, z;
    double atof(), getentry();

    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') switch (argv[i][1]) {
	  case 'd':	/* specify image dimensions in next 2 arguments */
	    if (i >= argc-2) {
		fprintf(stderr, "%s: must specify NROWS and NCOLS after -d\n",
			argv[0]);
		exit(1);
	    }
	    nrows = atoi(argv[++i]);
	    ncols = atoi(argv[++i]);
	    if (nrows < 1 || ncols < 1) {
		fprintf(stderr, "%s: NROWS and NCOLS must be positive\n",
			argv[0]);
		exit(1);
	    }
	    break;
	  case 'n':	/* generate negative image */
	    nflag = 1;
	    break;
	  case 'x':	/* specify xlow, xhigh in next 2 arguments */
	    if (i >= argc-2) {
		fprintf(stderr, "%s: must specify XLOW and XHIGH after -x\n",
			argv[0]);
		exit(1);
	    }
	    xlow = atof(argv[++i]);
	    xhigh = atof(argv[++i]);
	    if (xlow == xhigh) {
		fprintf(stderr, "%s: XLOW and XHIGH cannot be equal\n",
			argv[0]);
		exit(1);
	    }
	    break;
	  case 'y':	/* specify ylow, yhigh in next 2 arguments */
	    if (i >= argc-2) {
		fprintf(stderr, "%s: must specify YLOW and YHIGH after -y\n",
			argv[0]);
		exit(1);
	    }
	    ylow = atof(argv[++i]);
	    yhigh = atof(argv[++i]);
	    if (ylow == yhigh) {
		fprintf(stderr, "%s: YLOW and YHIGH cannot be equal\n",
			argv[0]);
		exit(1);
	    }
	    break;
	}
	else {		/* input file specified */
	    if (ifile != NULL) {
		fprintf(stderr, "%s: only one input file may be specified\n",
			argv[0]);
		exit(1);
	    }
	    if ((ifile = fopen(argv[i], "r")) == NULL) {
		fprintf(stderr, "%s: can't open image file %s\n", argv[0],
			argv[i]);
		exit(2);
	    }
	}
    }
    if (ifile == NULL) ifile = stdin;
    if (nrows < 1 || ncols < 1) {
	fprintf(stderr,
	  "usage: %s -d NROWS NCOLS [-x XLOW XHIGH] [-y YLOW YHIGH] [IFILE]\n",
		argv[0]);
	exit(1);
    }
    if (xlow == xhigh) {
	xlow = 0.;
	xhigh = ncols - 1.;
    }
    if (ylow == yhigh) {
	ylow = 0.;
	yhigh = nrows - 1.;
    }
    xinc = (xhigh - xlow)/(ncols - 1.);
    yinc = (yhigh - ylow)/(nrows - 1.);
    for (i = 0; i < ncols; i++) {
	x = xlow + xinc*i;
        for (j = 0; j < nrows; j++) {
	    y = ylow + yinc*j;
	    z = getentry();
	    if (z < 0. || z > 1.) {
		fprintf(stderr,
		   "%s: x = %g, y = %g, z = %g (out of range, truncated)\n",
			argv[0], x, y, z);
		if (z < 0.) z = 0.;
		else z = 1.;
	    }
	    /* start a new path at (x,y) */
	    printf("%g %g 1\n", x, y);
	    /* continue path to upper left corner of pixel */
	    printf("%g %g 0\n", x, y + yinc);
	    /* continue path to upper right corner of pixel */
	    printf("%g %g 0\n", x + xinc, y + yinc);
	    /* continue path to lower right corner of pixel, close path,
	       and fill with grey at level specified by z */
	    printf("%g %g %g\n", x + xinc, y,
		   nflag ? z + 12. : 13. - z);
	}
    }
    fclose(ifile);
    exit(0);
}

double getentry()
{
    static char buf[MAXLEN+2], *bp = NULL;
    static int ateof = 0;
    double z, atof();

    if (ateof) return (0.);
    while (bp == NULL) {
	if ((fgets(buf, MAXLEN+2, ifile)) == NULL) {
	    fprintf(stderr, "warning: unexpected EOF in image file\n");
	    ateof = 1;
	    return (0.);
	}
	bp = strtok(buf, " \t\n");
    }
    z = atof(bp);
    bp = strtok(NULL, " \t\n");
    return (z);
}


/* file: henon.c		G. Moody	24 April 2001

Evaluate the Hénon recurrence
*/

#include <stdio.h>
#include <stdlib.h>

main(int argc, char **argv)
{
    long i = 0, n = 25000;
    double a = 1.4, b = .3, x, y, x0 = 0, y0 = 0;

    if (argc > 1 && strcmp(argv[1], "-h") == 0) {
	fprintf(stderr, "usage: %s [ A B [ X0 Y0 ] [ N ] ]\n"
	   "where A and B are the coefficients in the Hénon recurrence:\n"
	   "               H(x,y) = (y + 1 - Ax^2, Bx)\n"
	   "X0 and Y0 are the initial values for x and y, and N is the\n"
	   "number of iterations (defaults: A = 1.4, B = .3, X0 = Y0 = 0,\n"
	   "N = 25000).  If N < 0, this program will run until interrupted.\n",
	   argv[0]);
	exit(1);
    }
    if (argc > 1)
	sscanf(argv[1], "%lf", &a);
    if (argc > 2)
	sscanf(argv[2], "%lf", &b);
    if (argc == 4)
	sscanf(argv[3], "%ld", &n);
    else if (argc > 4) {
	sscanf(argv[3], "%lf", &x0);
	sscanf(argv[4], "%lf", &y0);
    }
    if (argc > 5)
	sscanf(argv[5], "%ld", &n);
    while (n) {
	if (n > 0) n--;
	x = y0 + 1 - a*x0*x0;
	y = b*x0;
	printf("%g %g\n", x0 = x, y0 = y);
	/* The next line is only a delay loop and can be removed. */
	for (i = 0; i < 5000; i++) y += b*i;
    }
    exit(0);
}

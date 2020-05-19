/* file: ftable.c	G. Moody	24 February 2002
			Last revised:	10 December 2010

Make a script for generating a table of function values using bc
*/

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#endif

main(int argc, char **argv)
{
    char *expression, *p, *s;
    int i, nx = 0;
    double xmin = -50.0;
    double xmax = 50.0;
    double dx = 1.0;
    double x;
    double atof();

    if (argc < 2) {
	fprintf(stderr, "usage: %s EXPRESSION [ XMIN XMAX DX ]\n", argv[0]);
	exit(1);
    }
    expression = calloc(strlen(argv[1])+2, sizeof(char));
    strcpy(expression, argv[1]);
    for (s = expression; *s; s++)
	if (*s == 'x') *s = '\0';
    if (argc > 2) xmin = atof(argv[2]);
    if (argc > 3) xmax = atof(argv[3]);
    if (argc > 4)   dx = atof(argv[4]);

    for (x = xmin; x <= xmax; x += dx) {
	printf("print %f,\" \";", x);
	for (p = expression; p <= s; ) {
	    fputs(p, stdout);
	    p += strlen(p) + 1;
	    if (p <= s)
		printf("%f", x);
	}
	printf("\n");
    }
}






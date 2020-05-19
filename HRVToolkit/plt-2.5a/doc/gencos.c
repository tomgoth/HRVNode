#include <stdio.h>
#include <math.h>
#include <stdlib.h>

main()
{
    int i;
    double c;

    for (i = -180; i <= 180; i += 5) {
	c = cos(M_PI*i/180.0);
	printf("%d %g\n", i, c*c);
    }
    exit(0);
}


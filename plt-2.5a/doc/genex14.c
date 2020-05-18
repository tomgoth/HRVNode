#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifndef PI
#define PI	3.141592653589793238462643382795029
#endif

main()
{
    int i;

    for (i = 0; i <= 20; i++) {
	printf("%d %.3lf\n",
	       i*5,
	       sin(2*PI*i/7.0)+((double)random()/(double)RAND_MAX-0.5)/10.0);
    }
    exit(0);
}

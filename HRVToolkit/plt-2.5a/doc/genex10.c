#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifndef PI
#define PI	3.141592653589793238462643382795029
#endif

main()
{
    int i;

    for (i = 0; i < 20; i++) {
	printf("%d %.3lf %.3lf %.3lf\n",
	       i,
	       sin(0.4*i*PI) + 1.5,
	       (double)random()/(double)RAND_MAX + 3.8,
	       .1 + 0.15*(double)random()/(double)RAND_MAX);
    }
    exit(0);
}

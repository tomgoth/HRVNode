/*	mem.c		Paul Albrecht		March 1983

	Memory allocation subroutines.

	char *zmem(n,size)	Return pointer to n*size bytes of zeroed memory

	char *azmem(ptr,max,n,size)
			Increase array pointed to by ptr from (*max)*size to
			n*size bytes.  If ptr is zero allocate new array.
			If max is not null, increase *max by n.  Return ptr.

	char *strsave(str)	Return pointer to a saved version of str;
*/


typedef unsigned int	UINT;


char	*zmem( n, size )
UINT	n, size;
{
char	*ptr, *calloc();

	if( n*size < 1 || (ptr=calloc(n,size)) == 0 )
		estop( "zmem(%u,%u) failed to get %u bytes", n, size, n*size  );
	return( ptr );

}


char	*azmem( ptr, max, n, size )
char	*ptr;
UINT	*max, n, size;
{
UINT	oldb, newb;
char	*realloc(), *calloc();
register char *c, *cend;

	oldb = (*max) * size;
	newb = oldb + n*size;
	*max += n;

	if( ptr == 0 || oldb == 0 )
		ptr = newb ? calloc(n,size) : 0;
	else 	ptr = realloc(ptr, newb);

	if( ptr == 0 )
		estop( "azmem(%x,%u,%u,%u) failed to add %u bytes", ptr, *max, n, size, n*size );

	for( c= &ptr[oldb], cend= &ptr[newb];  c < cend;  c++ )
			*c = 0;

	return( ptr );
}

char	*strsave( str )
char	*str;
{
char	*ptr, *malloc();

	ptr = malloc( strlen(str)+1 );
	if( ptr != 0 )
		strcpy( ptr, str );
	return( ptr );
}

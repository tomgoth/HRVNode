# Makefile for PA's `libc' library (for `plt').
#	G. Moody	15 August 1990

CFLAGS = -O
L = libc.a
OL = $L(args.o) $L(data.o) $L(io.o) $L(mem.o)

libc.a:		$(OL)
	ranlib $L

clean:
	rm -f *.o *~

listing:
	enscript -2rk args.h opts.h args.c data.c io.c mem.c Makefile

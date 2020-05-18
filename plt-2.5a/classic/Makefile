# file: Makefile	Paul Albrecht		June 1987
#			Last revised:		24 May 1996 (by G. Moody)
# Copyright (C)	Paul Albrecht, 1988.

# This file is used to compile the programs used by the shell script `plt'.
# Set the site-dependent variables below before using it, edit `plt' and
# `lwcat' as appropriate for your system, then type `make' to compile and
# install `plt' and its associated programs.

# ----------------------------------------------------------------------------
# Site-dependent variables:

# BINDIR is the directory into which the executables and shell scripts are
# to be installed.  You must have write permission in BINDIR to complete the
# installation successfully.
BINDIR = /usr/local/bin

# PSPDIR is the directory in which PostScript prologs are kept.  You will also
# need write permission in PSPDIR.
PSPDIR = /usr/local/lib/ps

# LIBC is PA's libc library.
LIBC = libc/libc.a

# LIBCSRCDIR is the directory which contains sources for PA's libc library.
# (These are needed only if LIBC does not exist already.)
LIBCSRCDIR = libc

# CFLAGS is a set of C compiler options.  These may include:
#  -g			(to save debugging symbols)
#  -O			(to optimize; usually cannot be used together with -g)
#  -I/usr/5include	(needed on Sun systems only, to get System V headers)
#  -f68881		(to use a floating point coprocessor, as on the Sun 3)
# Note: don't use -O with gcc under Linux -- the executable won't handle -w
# options properly if you do.
CFLAGS = -g -L/usr/X11R6/lib

# LDFLAGS is appended to the C compiler command line when an executable file
# is to be produced;  normally it will not need to be changed.
LDFLAGS = $(LIBC) -lm

# TARGETS is a list of the executables to be compiled by default.  It may
# include any of:
#  egaplt	plt for EGA under Microport UNIX System V
#  gplt		plt for GraphOn GO-140 and Tektronix 4010 emulators
#  lwplt	plt for LaserWriter (and other PostScript devices)
#  mplt		plt for Masscomp
#  sunplt	plt for SunView
#  xplt		plt for X11
#  xpltwin	window-creation program needed by xplt
#  makeplthead	generates PostScript bounding box for lwplt
TARGETS = xplt xpltwin lwplt gplt makeplthead

# PRINT is the command to be used by `make listing'.
PRINT = enscript -2r

# It should not be necessary to modify anything below this line.
# ----------------------------------------------------------------------------

OBJ = main.o data.o plot.o axis.o tic.o figure.o window.o \
	text.o optn.o optn2.o optn3.o util.o pterm.o
SRC = main.c data.c plot.c axis.c tic.c figure.c window.c \
	text.c optn.c optn2.c optn3.c util.c iface.c pterm.c
SR2 = xpltwin.c xw.c sun.c go.c lw.c mcd.c unixpl.c makeplthead.c
INC = plt.h plot.h axis.h figs.h text.h optn.h
ADM = Makefile plt lwcat pslw.pro


# `make' or `make install' compiles LIBC and the targets if necessary, then
# installs the `plt' script and the targets in BINDIR.  It edits `lwcat' to
# set the correct path name for `pslw.pro', and installs `lwcat' in BINDIR and
# `pslw.pro' in PSDIR.

install:	$(LIBC) $(TARGETS)
	install $(TARGETS) $(BINDIR)
	cp plt $(BINDIR)
	sed s%PSPDIR%$(PSPDIR)% <lwcat >$(BINDIR)/lwcat
	cp gvcat $(BINDIR)/gvcat
	cp pslw.pro $(PSPDIR)

# `make clean' removes binary and backup files
clean:
	rm -f $(TARGETS) *.o *~ core

xplt:		$(OBJ) iface.o xw.o
	$(CC) $(CFLAGS) -static -o $@ $(OBJ) iface.o xw.o $(LDFLAGS) -lX11

xpltwin:	xpltwin.c
	$(CC) $(CFLAGS) -o $@ xpltwin.c $(LDFLAGS) -lX11

lwplt:		$(OBJ) lw.o
	$(CC) $(CFLAGS) -o $@ $(OBJ) lw.o $(LDFLAGS)

makeplthead:	makeplthead.c
	$(CC) $(CFLAGS) -o $@ makeplthead.c

sunplt:		$(OBJ) iface.o sun.o
	$(CC) $(CFLAGS) -o $@ $(OBJ) iface.o sun.o \
		$(LDFLAGS) -lsuntool -lsunwindow -lpixrect

gplt:		$(OBJ) iface.o go.o
	$(CC) $(CFLAGS) -o $@ $(OBJ) iface.o go.o $(LDFLAGS)

mplt:		$(OBJ) iface.o mcd.o
	$(CC) $(CFLAGS) -o $@ $(OBJ) iface.o mcd.o $(LDFLAGS) -lgp

egaplt:		$(OBJ) iface.o unixpl.o
	$(CC) $(CFLAGS) -o $@ $(OBJ) iface.o unixpl.o $(LDFLAGS) -ltega
	superbit egaplt

# This rule builds LIBC by spawning an inferior `make'.
$(LIBC):
	cd $(LIBCSRCDIR); make

listing:
	$(PRINT) $(INC) $(SRC) $(SR2) plt lwcat pslw.pro Makefile

# Dependencies
main.o:		plt.h plot.h axis.h figs.h text.h optn.h
plot.o:		plt.h plot.h axis.h text.h
data.o: 	plt.h plot.h axis.h text.h
axis.o: 	plt.h axis.h figs.h
tic.o:		plt.h axis.h text.h
figure.o:	plt.h plot.h axis.h figs.h
optn.o:		plt.h optn.h
optn2.o:	plt.h plot.h axis.h figs.h text.h optn.h
optn3.o:	plt.h optn.h
window.o:	plt.h plot.h axis.h figs.h text.h
text.o:		plt.h axis.h text.h
util.o:		plt.h plot.h
iface.o: 	plt.h plot.h text.h
pterm.o: 	plt.h
unixpl.o:	plt.h

xw.o:	xw.c
lw.o:	lw.c plt.h plot.h
sun.o:	sun.c plt.h
go.o:	go.c plt.h
mcd.o:	mcd.c plt.h

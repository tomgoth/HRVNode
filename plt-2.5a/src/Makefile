# file: Makefile	G. Moody		10 March 2001
#			Last revised:		25 March 2009
# This file is used to compile the programs used by `plt'.  See '../INSTALL'.

# ----------------------------------------------------------------------------
# Site-dependent variables:

# The values of these variables are normally set in ../Makefile -- change them
# there if necessary.

# PREFIX is the root of the tree into which everything is ultimately
# installed.
PREFIX=/usr/local

# INSTALL_PREFIX is usually the same as PREFIX, but 'rpm' or 'rpmbuild' resets
# it to the root of a temporary build tree when making a set of RPMs.  We
# need to distinguish PREFIX from INSTALL_PREFIX because the absolute pathname
# (starting with PREFIX) of the installed copy of 'plt.pro' needs to be
# inserted into 'lwcat' when doing a 'make install' (see below).
INSTALL_PREFIX=$(PREFIX)

# BINDIR is the directory into which the executables and shell scripts are
# to be installed.  You must have write permission in BINDIR to complete the
# installation successfully.
BINDIR = $(INSTALL_PREFIX)/bin

# PSPDIR is the directory in which PostScript prologs are kept.  You will also
# need write permission in PSPDIR.
PSPDIR = $(INSTALL_PREFIX)/lib/ps

# XINCDIR is the directory in which the X11 header (*.h) files are kept.  Only
# read permission is required in XINCDIR.
XINCDIR = /usr/include/X11

# XLIBDIR is the directory in which the X11 client libraries are kept.  Only
# read permission is required in XLIBDIR.
XLIBDIR = /usr/lib
# On a dual-architecture x86_64, you may need this instead:
# XLIBDIR = /usr/lib64
# (or alternatively compile in 32-bit mode by adding -m32 to CFLAGS below).

# CFLAGS is a set of C compiler options.  These may include:
#  -g		   (to save debugging symbols)
#  -O		   (to optimize; usually cannot be used together with -g)
#  -DDEBUG	   (to enable special debugging code; see plt.h)
#  -DNO_VSNPRINTF  (if your C library is missing vsnprintf;  but note that
#	 	    the alternative, using vsprintf, is a security problem,
#		    and that it's better to update your C library if you can)
CFLAGS = -O

# ----------------------------------------------------------------------------
# Pathnames of programs used by 'plt' that are not part of the 'plt' package
#
# These programs are not required for minimal functionality of 'plt', but they
# are used to provide additional features.  All are free, open source software.
# The definitions in ../Makefile override these if 'make' is invoked from
# the parent of this directory.

# CONVERT is the full pathname of the 'convert' application (part of
# ImageMagick, http://www.imagemagick.org), needed to make PNG plots.
CONVERT = /usr/bin/convert

# EPSTOPDF is the full pathname of the 'epstopdf' application, available from
# CTAN at http://tug.ctan.org/tex-archive/support/epstopdf/, and included
# in the tetex, TeX Live (http://www.tug.org/texlive/), and MiKTeX
# (http://www.miktex.org/) distributions.  EPSTOPDF is needed to make PDF plots.
EPSTOPDF = /usr/bin/epstopdf

# GV is the full pathname of a viewer for PostScript files,
# such as 'gv' (http://www.gnu.org/software/gv/) or
# 'evince' (http://www.gnome.org/projects/evince/), used by 'lwcat -gv'.
GV = /usr/bin/gv

# LPR is the full pathname of the print spooler used by 'lwcat'.
LPR=/usr/bin/lpr

# PRINT is the command used by `make listing' (not needed by 'plt' itself),
# available at http://www.codento.com/people/mtr/genscript/.
PRINT = enscript -2r

# It should not be necessary to modify anything below this line.
# ----------------------------------------------------------------------------

# SFILES is a list of shell scripts.
SFILES = gvcat lwcat plt pltpng

# XFILES is a list of compiled executable files.
XFILES = lwplt makeplthead xplt xpltwin

# EXEFILES is a list of XFILES with the .exe suffix (for MS-Windows only).
EXEFILES = lwplt.exe makeplthead.exe xplt.exe xpltwin.exe

# INSTFILES is a list of files that may be installed in BINDIR.
INSTFILES = $(SFILES) $(XFILES)

OBJ = main.o data.o plot.o axis.o tick.o figure.o window.o \
	text.o option.o util.o pterm.o
SRC = main.c data.c plot.c axis.c tick.c figure.c window.c \
	text.c option.c util.c pterm.c lw.c makeplthead.c xpltwin.c xw.c 
INC = plt.h rgb.h
ADM = Makefile gvcat lwcat plt plt.pro pltpng

# `make all' (or just `make') compiles the targets but does not install them.
# You must have write permission in this directory, but nothing more is needed.
all:		lwplt makeplthead
	if [ -d $(XINCDIR) ]; then $(MAKE) xplt xpltwin; fi
	@echo "plt has been compiled successfully.  Install it by typing:"
	@echo "    make install"

# `make install' compiles the targets if necessary, then installs the `plt'
# script and the targets in BINDIR.  It edits `lwcat' to set the correct path
# name for `plt.pro', and installs `lwcat' and `gvcat' in BINDIR and
# `plt.pro' in PSPDIR.  You must have write permission in BINDIR and PSPDIR.
install:	all
	-mkdir -p $(BINDIR)
	-mkdir -p $(PSPDIR)
	if [ -d $(XINCDIR) ]; then cp xplt xpltwin $(BINDIR); fi
	cp lwplt makeplthead plt pltpng $(BINDIR)
	sed s%GV%$(GV)% <gvcat >$(BINDIR)/gvcat
	sed s%PSPDIR%$(PREFIX)/lib/ps% <lwcat | \
	 sed s%BINDIR%$(BINDIR)% | sed s%LPR%$(LPR)% | \
	 sed s%EPSTOPDF%$(EPSTOPDF)% >$(BINDIR)/lwcat
	sed s%BINDIR%$(BINDIR)% <plt >$(BINDIR)/plt
	sed s%CONVERT%$(CONVERT)% <pltpng >$(BINDIR)/pltpng
	cd $(BINDIR); chmod -f 755 $(INSTFILES)
	cp plt.pro $(PSPDIR)
	chmod 644 $(PSPDIR)/plt.pro

# 'make uninstall':  remove all installed files
uninstall:
	cd $(BINDIR); rm -f $(INSTFILES) $(EXEFILES)
	rm -f $(PSPDIR)/plt.pro

# PostScript driver for plt
lwplt:		$(OBJ) lw.o
	$(CC) $(CFLAGS) -o $@ $(OBJ) lw.o -lm

# Utility for calculating PostScript bounding boxes
makeplthead:	makeplthead.c
	$(CC) $(CFLAGS) -o $@ makeplthead.c

# X11 driver for plt
xplt:		$(OBJ) xw.o
	$(CC) $(CFLAGS) -o $@ $(OBJ) xw.o -L$(XLIBDIR) -lX11 -lm

# Utility that creates an X11 window in which xplt draws
xpltwin:	plt.h xpltwin.c
	$(CC) $(CFLAGS) -I$(XINCDIR) -o $@ xpltwin.c -L$(XLIBDIR) -lX11

# Rules for compiling *.o files needed by lwplt and xplt.
$OBJ:		plt.h

lw.o:		plt.h rgb.h
	$(CC) $(CFLAGS) -c lw.c

xw.o:		plt.h
	$(CC) $(CFLAGS) -I$(XINCDIR) -c xw.c

# Print a listing of plt sources.
listing:
	$(PRINT) $(INC) $(SRC) $(ADM)

# Remove binary and backup files.
clean:
	rm -f $(XFILES) $(EXEFILES) *.o *~ core

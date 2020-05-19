# file: Makefile	G. Moody       24 February 2002
#			Last revised:	26 April 2005
# This file is used to compile miscellaneous 'plt'-related stuff. See
# '../INSTALL'.

# ----------------------------------------------------------------------------
# Site-dependent variables:

# The values of these variables are normally set in ../Makefile -- change them
# there if necessary.

# PREFIX is the root of the tree into which the plt package is ultimately
# installed.
PREFIX=/usr/local

# INSTALL_PREFIX is usually the same as PREFIX, but 'rpm' or 'rpmbuild' resets
# it to the root of a temporary build tree when making a set of RPMs.
INSTALL_PREFIX=$(PREFIX)

# BINDIR is the directory into which the executables and shell scripts are
# to be installed.  You must have write permission in BINDIR to complete the
# installation successfully.
BINDIR = $(INSTALL_PREFIX)/bin

# It should not be necessary to modify anything below this line.
# ----------------------------------------------------------------------------

# `make all' (or just `make') compiles the targets but does not install them.
# You must have write permission in this directory, but nothing more is needed.
all:		ftable imageplt
	@echo "plt-misc compiled successfully.  Install by typing:"
	@echo "    make install"

# `make install' compiles the targets if necessary, then installs them.
# You must have write permission in BINDIR.
install:	all
	-mkdir -p $(BINDIR)
	cp ftable pltf imageplt $(BINDIR)
	cd $(BINDIR); chmod 755 ftable pltf imageplt

uninstall:
	cd $(BINDIR); rm -f ftable ftable.exe pltf imageplt imageplt.exe

# ftable: given a function of x, ftable makes a bc script for generating a
# table of function values.
ftable:		ftable.c
	$(CC) $(CFLAGS) -o $@ ftable.c

# imageplt: prepare a grey-scale image to be plotted via 'plt 0 1 2 -pc'
imageplt:	imageplt.c
	$(CC) $(CFLAGS) -o $@ imageplt.c

# Remove binary and backup files.
clean:
	rm -f ftable ftable.exe imageplt imageplt.exe *.o *~ core

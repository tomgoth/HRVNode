# file: Makefile	G. Moody	  4 April 2001
#			Last revised:	12 December 2010	plt 2.5a
# Top-level `make' description file for `plt'.  See `INSTALL'.

# Version and release numbers.
PLTVER=2.5a
PLTREL=1

# ----------------------------------------------------------------------------
# Site-dependent variables:

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

# MAN1DIR is the directory in which man pages for applications are kept.
MAN1DIR = $(INSTALL_PREFIX)/share/man/man1

# PSPDIR is the directory in which PostScript prologs are kept.  You will also
# need write permission in PSPDIR.
PSPDIR = $(INSTALL_PREFIX)/lib/ps

# XINCDIR is the directory in which the X11 header (*.h) files are kept.  Only
# read permission is required in XINCDIR.
XINCDIR = /usr/include/X11
# XINCDIR = /usr/X11R6/include  # for Cygwin/MS Windows

# XLIBDIR is the directory in which the X11 client libraries are kept.  Only
# read permission is required in XLIBDIR.
XLIBDIR = /usr/lib
# XLIBDIR = /usr/X11/lib  # for Mac OS X
# XLIBDIR = /usr/X11R6/lib  # for Cygwin/MS Windows

# CFLAGS is a set of C compiler options.  These may include:
#  -g		   (to save debugging symbols)
#  -O		   (to optimize; usually cannot be used together with -g)
#  -DDEBUG	   (to enable special debugging code; see src/plt.h)
#  -DNO_VSNPRINTF  (if your C library is missing vsnprintf;  but note that
#	 	    the alternative, using vsprintf, is a security problem,
#		    and that it's better to update your C library if you can)
CFLAGS = -g -DPLT_VERSION='\"$(PLTVER)\"'

# TARGETS is a list of the executables to be compiled by default.  It may
# include any of:
#  lwplt	plt for LaserWriter (and other PostScript devices)
#  makeplthead	generates PostScript bounding box for lwplt
#  xplt		plt for X11
#  xpltwin	window-creation program needed by xplt
TARGETS = lwplt makeplthead xplt xpltwin

# ----------------------------------------------------------------------------
# Pathnames of programs used by 'plt' that are not part of the 'plt' package
#
# These programs are not required for minimal functionality of 'plt', but they
# are used to provide additional features.  All are free, open source software.
# These definitions override those in src/Makefile if 'make' is invoked from
# this directory.

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

# MAKE is the command used to run dependent 'make' processes in the lower-level
# directories.

MAKE = make PLTVER=$(PLTVER) \
PREFIX=$(PREFIX) INSTALL_PREFIX=$(INSTALL_PREFIX) \
 BINDIR=$(BINDIR) MAN1DIR=$(MAN1DIR) PSPDIR=$(PSPDIR) XINCDIR=$(XINCDIR) \
 XLIBDIR=$(XLIBDIR) CFLAGS="$(CFLAGS)" CONVERT=$(CONVERT) EPSTOPDF=$(EPSTOPDF) \
 GV=$(GV) LPR=$(LPR) PRINT="$(PRINT)"
# It should not be necessary to modify anything below this line.
# ----------------------------------------------------------------------------

# `make all' (or just `make') compiles the targets but does not install them.
# You must have write permission in the `src' directory, but nothing more is
# needed.
all:
	cd src; $(MAKE) all
	cd misc; $(MAKE) all

# `make install' compiles the targets if necessary, then installs the `plt'
# script and the targets in BINDIR.  It edits `lwcat' to set the correct path
# name for `pslw.pro', and installs `lwcat' and `gvcat' in BINDIR and
# `pslw.pro' in PSPDIR.  You must have write permission in BINDIR and PSPDIR.
install:	force-install
	cd src; $(MAKE) install
	cd misc; $(MAKE) install
	cd doc; $(MAKE) install
	@echo "plt has been installed successfully.  Test it by typing:"
	@( if [ -s $(BINDIR)/xplt ]; \
	  then echo "    make xdemo"; \
	       echo "in an X terminal window, or by typing:"; fi )
	@echo "    make psdemo"

# 'make uninstall' removes everything installed by 'make install'.
uninstall:
	cd src; $(MAKE) uninstall
	cd misc; $(MAKE) uninstall
	cd doc; $(MAKE) uninstall
	@echo "plt has been uninstalled successfully.  If any of $(BINDIR),"
	@echo "$(MAN1DIR), or $(PSPDIR) were created by the installation,"
	@echo "they must be removed manually."

# This is a kludge, so that `make install' will work under MS-Windows even
# though MS-Windows can't tell the difference between `INSTALL' and 'install'.
force-install:
	touch force-install

# `make mswplt':  install on MS-Windows (lwplt only, no xplt)
# You *must* use the free Cygwin C compiler and tools -- MSVC and other
# commercial compilers *will not work*.  See the README for details.
mswplt:
	cd src; $(MAKE) mswplt

# PostScript driver for plt
lwplt:
	cd src; $(MAKE) lwplt

# Utility for calculating PostScript bounding boxes
makeplthead:
	cd src; $(MAKE) makeplthead

# X11 driver for plt
xplt:
	cd src; $(MAKE) xplt

# Utility that creates an X11 window in which xplt draws
xpltwin:
	cd src; $(MAKE) xpltwin

# Print a listing of plt sources.
listing:
	cd src; $(MAKE) listing

# Print a copy of the plt Tutorial and Cookbook (requires that lpr
# be able to print a PostScript document;  see doc/Makefile).
hardcopy:
	cd doc; $(MAKE) hardcopy

# Show figures from the plt Tutorial and Cookbook in an X window.
xdemo:	all
	cd doc; $(MAKE) xdemo

# Show figures from the plt Tutorial and Cookbook in a GhostScript window.
psdemo:	all
	cd doc; $(MAKE) psdemo

# Create a distribution file in the parent directory using GNU tar.
tarball:
	touch timestamp
	$(MAKE) clean
	cd ..; tar cfvz plt-$(PLTVER).tar.gz plt-$(PLTVER)

# Create source and binary RPMs
RPMROOT=/usr/src/redhat

rpms:		tarball	
	cp -p ../plt-$(PLTVER).tar.gz $(RPMROOT)/SOURCES
	sed s/PLTVER/$(PLTVER)/ <plt.spec | \
	 sed s/PLTREL/$(PLTREL)/ >$(RPMROOT)/SPECS/plt-$(PLTVER)-$(PLTREL).spec
	if [ -x /usr/bin/rpmbuild ]; \
	 then rpmbuild -ba $(RPMROOT)/SPECS/plt-$(PLTVER)-$(PLTREL).spec; \
	 else rpm -ba $(RPMROOT)/SPECS/plt-$(PLTVER)-$(PLTREL).spec; \
	fi
	mv $(RPMROOT)/RPMS/*/plt*rpm ..
	mv $(RPMROOT)/SRPMS/plt*rpm ..
	rm -f $(RPMROOT)/SPECS/plt-$(PLTVER)-$(PLTREL).spec

# Remove binary and backup files.
clean:
	cd src; $(MAKE) clean
	cd doc; $(MAKE) clean
	cd misc; $(MAKE) clean
	rm -f *~ core force-install timestamp


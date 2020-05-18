# file: Makefile	G. Moody	29 March 2001
#			Last revised:	25 March 2009
# 'make' description file for creating 'plt' documentation

# The primary documentation for plt is the 'plt Tutorial and Cookbook', which
# is included here in LaTeX source format (book.tex), and also (for those who
# don't have LaTeX) in PDF (book.pdf), PostScript (book.ps), and HTML (begin
# reading at ..html/index.html).  The formatted versions can be recreated from
# book.tex if you have the necessary (free) software (see below).  The figures
# contained within the book can also be recreated using 'plt' once you have
# installed it.

# Also included here is a set of Unix-style 'man' pages, meant to be read
# using the 'man' command (e.g., 'man plt').

# ----------------------------------------------------------------------------
# Site-dependent variables:

# The values of these variables are normally set in ../Makefile -- change them
# there if necessary.

# PREFIX is the root of the tree into which the plt package is ultimately
# installed.
PREFIX=/usr

# INSTALL_PREFIX is usually the same as PREFIX, but 'rpm' or 'rpmbuild' resets
# it to the root of a temporary build tree when making a set of RPMs.
INSTALL_PREFIX=$(PREFIX)

# MAN1DIR is the directory in which the plt man pages are kept.
MAN1DIR = $(INSTALL_PREFIX)/share/man/man1

# MANPAGES are the sources for the plt man pages.
MANPAGES = imageplt.1 lwcat.1 plt.1 pltf.1

# GZMANPAGES are the compressed versions of MANPAGES.
GZMANPAGES = imageplt.1.gz lwcat.1.gz plt.1.gz pltf.1.gz

# It should not be necessary to modify anything below this line.
# ----------------------------------------------------------------------------

# PSFIGURES is a list of the figures in the book.
PSFIGURES = figure1.ps simple0.ps.gz simple1.ps simple2.ps simple3.ps         \
	    simple4.ps simple5.ps ecg.ps coords.ps figure8.ps figure2.ps      \
	    figure3.ps henon.ps.gz figure4.ps symbols.ps style-C.ps           \
	    style-e+Z.ps style-e-X.ps style-ec+.ps style-E+0.ps               \
	    style-E-ftriangle.ps style-Ecsquare.ps style-f.ps style-i.ps      \
	    style-l.ps style-m.ps style-n.ps style-N.ps style-o.ps            \
	    style-O.ps style-sO.ps style-Sfdiamond.ps style-t.ps              \
	    figure6.ps figure7.ps figure10.ps figure11.ps figure12.ps         \
            figure13.ps figure9.ps labels.ps flowchart.ps fonts.ps            \
	    linestyles.ps fontgroup1.ps figure14.ps colors.ps figure16.ps     \
	    figure17.ps conf.ps fig.ps function.ps image.ps

# PDFFIGURES are made from PSFIGURES using epstopdf.
PDFFIGURES= figure1.pdf simple0.pdf simple1.pdf simple2.pdf simple3.pdf       \
	    simple4.pdf simple5.pdf ecg.pdf coords.pdf figure8.pdf figure2.pdf\
	    figure3.pdf henon.pdf figure4.pdf symbols.pdf style-C.pdf         \
	    style-e+Z.pdf style-e-X.pdf style-ec+.pdf style-E+0.pdf           \
	    style-E-ftriangle.pdf style-Ecsquare.pdf style-f.pdf style-i.pdf  \
	    style-l.pdf style-m.pdf style-n.pdf style-N.pdf style-o.pdf       \
	    style-O.pdf style-sO.pdf style-Sfdiamond.pdf style-t.pdf          \
	    figure6.pdf figure7.pdf figure10.pdf figure11.pdf figure12.pdf    \
            figure13.pdf figure9.pdf labels.pdf flowchart.pdf fonts.pdf       \
	    linestyles.pdf fontgroup1.pdf figure14.pdf colors.pdf figure16.pdf\
	    figure17.pdf conf.pdf fig.pdf function.pdf image.pdf

.SUFFIXES: .ps .ps.gz .pdf

.ps.pdf:
	epstopdf $<

.ps.gz.pdf:
	gzip -d <$< | epstopdf --filter >$@

# 'make' or 'make book.ps' creates the PostScript version of the book from the
# LaTeX sources.  You won't need to do this unless you change the sources.

book.ps:	book.dvi
	dvips -o book.ps book.dvi

# 'make html' creates an HTML version from the LaTeX sources.
html:		book.dvi $(PSFIGURES)
	test -d ../html || mkdir ../html
	latex2html -dir ../html -local_icons book \
	 -up_url="/physiotools/manuals.shtml" \
	 -up_title="Books about PhysioToolkit"
	./fixhtml		# change background of pages to white
	cp icons/* ../html	# shouldn't be necessary

# 'make book.dvi' creates the (La)TeX device-independent output file,
# with all cross-references and indices.
book.dvi:	book.tex plt.1.tex lwcat.1.tex imageplt.1.tex pltf.1.tex $(PSFIGURES)
	latex book
	latex book
	makeindex book
	latex book	# we need 3 passes to get cross-refs right

# 'make book.pdf' creates a PDF version from the LaTeX sources.
book.pdf:	book.tex plt.1.tex lwcat.1.tex imageplt.1.tex pltf.1.tex $(PDFFIGURES)
	pdflatex book
	pdflatex book
	makeindex book
	pdflatex book	# we need 3 passes to get cross-refs right

# The following rules produce LaTeX source versions of the plt.1, lwcat.1,
# imageplt.1, and pltf.1 man pages using rman (PolyglotMan), and using tail,
# sed, and tr to clean up the mess rman version 3.0.7 makes -- later versions
# of rman may not need all of these tweaks, but they should be harmless.  These
# LaTeX files are incorporated into book.tex (the plt Tutorial and Cookbook) as
# its last appendix.
plt.1.tex:	plt.1
	rman -fLaTeX2e plt.1 | tail +10 | sed "s%{itemize}%{description}%g" | \
	 sed "s%textbf%texttt%g" | tr \\n \_ | sed "s%_}%}%g" | tr \_ \\n | \
	 sed "s% }%}%g" | sed "s%section{%subsection*{%" | \
	 sed "s%\end{document}%%" >plt.1.tex
lwcat.1.tex:	imageplt.1
	rman -fLaTeX2e lwcat.1 | tail +10 | \
	 sed "s%{itemize}%{description}%g" | \
	 sed "s%textbf%texttt%g" | tr \\n \_ | sed "s%_}%}%g" | tr \_ \\n | \
	 sed "s% }%}%g" | sed "s%section{%subsection*{%" | \
	 sed "s%\end{document}%%" >lwcat.1.tex
imageplt.1.tex:	imageplt.1
	rman -fLaTeX2e imageplt.1 | tail +10 | \
	 sed "s%{itemize}%{description}%g" | \
	 sed "s%textbf%texttt%g" | tr \\n \_ | sed "s%_}%}%g" | tr \_ \\n | \
	 sed "s% }%}%g" | sed "s%section{%subsection*{%" | \
	 sed "s%\end{document}%%" >imageplt.1.tex
pltf.1.tex:	pltf.1
	rman -fLaTeX2e pltf.1 | tail +10 | \
	 sed "s%{itemize}%{description}%g" | \
	 sed "s%textbf%texttt%g" | tr \\n \_ | sed "s%_}%}%g" | tr \_ \\n | \
	 sed "s% }%}%g" | sed "s%section{%subsection*{%" | \
	 sed "s%\end{document}%%" >pltf.1.tex

plt.1.ps:	plt.1
	man -t ./plt.1 >plt.1.ps

lwcat.1.ps:	lwcat.1
	man -t ./lwcat.1 >lwcat.1.ps

imageplt.1.ps:	imageplt.1
	man -t ./imageplt.1 >imageplt.1.ps

pltf.1.ps:	pltf.1
	man -t ./pltf.1 >pltf.1.ps

figure1.ps:	example1.data
	plt -T lw example1.data 0 2 | lwcat -eps >figure1.ps

heartrate.data:	
	ihr -r mitdb/115 -a atr -t 5:0 -x >heartrate.data
# `ihr' is part of the WFDB Software Package available from PhysioNet
# (http://www.physionet.org/).

# The next block generates the X11 screen dump used as the first figure in
# chapter 2.  Since screen dumps are large, the file is kept in gzip-
# compressed form, with a separate .bb file containing its bounding box
# so that LaTeX can leave a space of the appropriate size on the page.
# The page dimensions for convert should match the size of the screen dump
# in pixels, including the window manager's frame.  They may need minor
# adjustment if the window manager is not 'enlightenment'.
#
# In order to make this file, you will need xwd (part of X11),
# convert (from the ImageMagick package), and gzip, in addition to
# plt.

simple0.ps.gz: heartrate.data
	xpltwin -g 800x600 # make sure the plt window is the right size
	plt heartrate.data
	@echo Please click on the plt window to create the screen dump.
	xwd -frame >simple0.xwd
	@echo Type 'Q' in the plt window to dismiss it.
	@sleep 2
	convert -page 812x628 simple0.xwd simple0.ps
	rm -f simple0.xwd
	grep BoundingBox simple0.ps | head -1 >simple0.ps.bb
	gzip simple0.ps

simple1.ps:	heartrate.data
	plt -T lw heartrate.data | lwcat -eps >simple1.ps

simple2.ps:	heartrate.data
	plt -T lw heartrate.data -t "Heart rate time series" \
	  -x "Time (seconds)" -y "Heart rate (beats per minute)" \
	 | lwcat -eps >simple2.ps

simple3.ps:	heartrate.data
	plt -T lw heartrate.data -t "Heart rate time series" \
	  -x "Time (seconds)" -y "Heart rate (beats per minute)" \
	  -xa 60 300 15 - 4 -ya 0 80 20 -g grid,sub \
	 | lwcat -eps >simple3.ps

simple4.ps:	heartrate.data
	plt -T lw heartrate.data -t "Heart rate time series" \
	  -x "Time (seconds)" -y "Heart rate (beats per minute)" \
	  -xa 60 300 15 - 4 0 -ya 0 80 20 -g grid,sub \
	  -p 0,1Scircle \
	 | lwcat -eps >simple4.ps

simple5.ps:	heartrate.data
	plt -T lw heartrate.data -t "Heart rate time series" \
	  -x "Time (seconds)" -y "Heart rate (beats per minute)" \
	  -xa 60 300 15 - 4 0 -ya 0 80 20 -g grid,sub \
	  -p "0,1Scircle(P/2,Cblue)" \
	 | lwcat -eps >simple5.ps

ecg.ps:		ecg.dat
	plt -T lw :s2,1024,2049,1 ecg.dat\
	        -cz 8 .00781 -F"p 0,1n(Cred) 0,2n(Cblue)" | lwcat -eps >ecg.ps

coords.ps:	coords.format
	plt -T lw -f coords.format | lwcat -eps >coords.ps

figure8.ps:	example8.format
	plt -T lw -f example8.format | lwcat -eps >figure8.ps

figure2.ps:	example1.data
	plt -T lw example1.data 0 2\
		-x "time in seconds" -y "amplitude in cm" -t "Time vs Amp" | \
		lwcat -eps >figure2.ps

figure3.ps:	example1.data example3.format
	plt -T lw example1.data 0 2 -f example3.format | lwcat -eps >figure3.ps

figure4.ps:	example4.data
	plt -T lw example4.data 0 1 2 -F"\
		fs helvetica longdashed dotted\
		p c" | lwcat -eps >figure4.ps

symbols.ps:	symbols.dat symbols.format
	plt -T lw symbols.dat -f symbols.format | lwcat -eps >symbols.ps

style-C.ps:	styles.data
	plt -T lw styles.data -p "0,1C(G.90)" -t "Plotstyle C" | \
	 lwcat -eps >style-C.ps

style-e+Z.ps:	styles.data
	plt -T lw styles.data -p 0,1,2e+Z -t "Plotstyle e+Z" | \
	 lwcat -eps >style-e+Z.ps

style-e-X.ps:	styles.data
	plt -T lw styles.data -p 0,1,2e-X -t "Plotstyle e-X" | \
	 lwcat -eps >style-e-X.ps

style-ec+.ps:	styles.data
	plt -T lw styles.data -p 0,1,2e:+ -t "Plotstyle e:+" | \
	 lwcat -eps >style-ec+.ps

style-E+0.ps:	styles.data
	plt -T lw styles.data -p 0,1,2E+0 -t "Plotstyle E+0" | \
	 lwcat -eps >style-E+0.ps

style-E-ftriangle.ps:	styles.data
	plt -T lw styles.data -p 0,1,2E-ftriangle \
	    -t "Plotstyle E-ftriangle" | lwcat -eps >style-E-ftriangle.ps

style-Ecsquare.ps:	styles.data
	plt -T lw styles.data -p 0,1,2E:square \
	    -t "Plotstyle E:square" | lwcat -eps >style-Ecsquare.ps

style-f.ps:	styles.data
	plt -T lw styles.data -p "0,1,2f(G.90)" -t "Plotstyle f" | \
	 lwcat -eps >style-f.ps

style-i.ps:	styles.data
	plt -T lw styles.data -p 0,1i -t "Plotstyle i" | \
	 lwcat -eps >style-i.ps

style-l.ps:	styles.data
	plt -T lw styles.data -p 0,1,0,3l -t "Plotstyle l" | \
	 lwcat -eps >style-l.ps

style-m.ps:	styles.data
	plt -T lw styles.data -p 1m -t "Plotstyle m" | \
	 lwcat -eps >style-m.ps

style-n.ps:	styles.data
	plt -T lw styles.data -p 0,1n -t "Plotstyle n" | \
	 lwcat -eps >style-n.ps

style-N.ps:	styles.data
	plt -T lw styles.data -p "0,1N(G.90)" -t "Plotstyle N" | \
	 lwcat -eps >style-N.ps

style-o.ps:	styles.data
	plt -T lw styles.data -p 0,1,2o -t "Plotstyle o" | \
	 lwcat -eps >style-o.ps

style-O.ps:	styles.data
	plt -T lw styles.data -p "0,1,2O(G.90)" -t "Plotstyle O" | \
	 lwcat -eps >style-O.ps

style-sO.ps:	styles.data
	plt -T lw styles.data -p 0,1sO -t "Plotstyle sO" | \
	 lwcat -eps >style-sO.ps

style-Sfdiamond.ps:	styles.data
	plt -T lw styles.data -p 0,1Sfdiamond \
	    -t "Plotstyle Sfdiamond" | lwcat -eps >style-Sfdiamond.ps

style-t.ps:	styles.data
	plt -T lw styles.data -p 0,1,3t -t "Plotstyle t" \
	 -ts "Do Re Mi Fa Sol La Ti" CB | lwcat -eps >style-t.ps

figure6.ps:	example5.data
	plt -T lw example5.data 0 3 0 2 1 -F"p s+ s* m" \
		-x "x axis" -y "y axis" -t "plot of y=x; y=2x and y=3x" | \
		lwcat -eps >figure6.ps

figure7.ps:	example7.data example7.format
	( plt -T lw example7.data 0 1 -f example7.format; \
	  plt -T lw example7.data 0 2 -f example7.axes -o ) | \
	 lwcat -eps >figure7.ps

genex10:	genex10.c
	$(CC) -o genex10 genex10.c -lm

example10.data:	genex10
	./genex10 >example10.data

figure10.ps:	example10.data example10.format
	plt -T lw example10.data -f example10.format | \
	 lwcat -eps >figure10.ps

figure11.ps:	example11.data
	( plt -T lw -wm 0 -t "This is the main title for the plot"; \
	  plt -T lw example11.data 0 1 -wm 1 -t "Window 1" ) | \
	 lwcat -eps >figure11.ps

figure12.ps:	example11.data
	( plt -T lw -wb 0 -t "This is the main title for the plot"; \
	  plt -T lw example11.data 0 1 -wb 1 -t "Window 1"; \
	  plt -T lw example11.data 0 2 -wb 2 -t "Window 2" ) | \
	 lwcat -eps >figure12.ps

figure13.ps:	example11.data
	( plt -T lw -wq 0 -t "This is the main title for the plot"; \
	  plt -T lw example11.data 0 1 -wq 1 -t "Window 1"; \
	  plt -T lw example11.data 0 2 -wq 2 -t "Window 2"; \
	  plt -T lw example11.data 0 3 -wq 3 -t "Window 3"; \
	  plt -T lw example11.data 0 4 -wq 4 -t "Window 4" ) | \
	 lwcat -eps >figure13.ps

figure16.ps:	ldemo.data
	( plt -T lw ldemo.data 2 3 -t"FREQUENCY RESPONSE OF THE FILTER" \
	   -x"FREQUENCY IN HERTZ" -y"THIS IS THE AMPLITUDE" -sf all P16; \
	  plt -T lw ldemo.data 2 3 -t"FREQUENCY RESP." -x"FREQUENCY" \
	   -y"AMPLITUDE"  -W .3 .3 .5 .45 -se -sf all P12; \
	  plt -T lw ldemo.data 2 3 -t"FREQUENCY RESPONSE" -x"FREQUENCY" \
	   -y"AMPLITUDE"  -W .6 .55 .9 .8 -se  -sf all P14 ) | \
	 lwcat -eps >figure16.ps

figure9.ps:	example9.data example9.format
	plt -T lw example9.data -f example9.format | lwcat -eps >figure9.ps

gencos:		gencos.c
	$(CC) -o gencos gencos.c -lm

cos2.data:	gencos
	./gencos >cos2.data

labels.ps:	cos2.data labels.format
	plt -T lw cos2.data 0 1 -f labels.format | lwcat -eps >labels.ps

flowchart.ps:	flowchart.format
	plt -T lw -f flowchart.format | lwcat -eps >flowchart.ps

fonts.ps:	fonts.format
	plt -T lw -f fonts.format | lwcat -eps >fonts.ps

linestyles.ps:	linestyles.format
	plt -T lw -f linestyles.format | lwcat -eps >linestyles.ps

genex14:	genex14.c
	$(CC) -o genex14 genex14.c -lm

example14.data:	genex14
	./genex14 >example14.data

fontgroup1.ps:	example14.data fontgroup.format
	plt -T lw example14.data -f fontgroup.format | \
	 lwcat -eps >fontgroup1.ps

figure14.ps:	example14.data example14.format
	plt -T lw example14.data -f example14.format | \
	 lwcat -eps >figure14.ps

colors.ps:	colors.format
	plt -T lw -f colors.format | lwcat -eps >colors.ps

figure17.ps:	ldemo.data
	( plt -T lw -wq 0 -t"THE TITLE FOR THE ENTIRE GRAPH GOES HERE"; \
	  plt -T lw ldemo.data 2 3 -wqs 1 \
	    -lx -g in -t"LPF: Log Freq & Ticks in"; \
	  plt -T lw ldemo.data 4 5 -wqs 4 \
	    -lx -ly -g both -t"LPF: Log Freq & Ampl"; \
	  plt -T lw ldemo.data 0 1 -wqs 3 \
	    -lx e -g out  -t"Alternate base & Ticks out"; \
	  plt -T lw ldemo.data 6 7 -wqs 2 \
	    -lx -ly - yes -g grid,sym,out -t"Log axes with grid" ) | \
	 lwcat -eps >figure17.ps

conf.ps:	power.data conf1.format conf2.format
	( plt -T lw power.data 0 1 2 3 4 5 -f conf1.format; \
	  plt -T lw power.data 0 6 7 8 9 10 -f conf2.format ) | \
	 lwcat -eps >conf.ps

function.ps:
	PTERM=lw pltf 's(40*x)*s(3*x)' 0 5 .01 | lwcat -eps >function.ps

image.ps:
	imageplt -d 5 3 image.data | plt -T lw 0 1 2 -pc | lwcat -eps >image.ps

henon:		henon.c
	$(CC) -o henon henon.c

henon.ps.gz:	henon.c
	$(CC) -o henon henon.c
	./henon | plt -T lw % -p s. -X -1.5 1.5 -Y -.5 .5 \
	 -t "Henon attractor" | lwcat -eps >henon.ps
	grep BoundingBox henon.ps | head -1 >henon.ps.bb
	gzip henon.ps

# 'make xdemo': show the figures from the book in an X window
xdemo:		henon example10.data cos2.data example14.data
	./xdemo.sh

# 'make psdemo': show the figures from the book in a GhostScript window
psdemo:		henon example10.data cos2.data example14.data
	./psdemo.sh

#.............................................................................
# a few more examples, not included in the book:

cpu.ps:		cpu.data cpu.format cpu.strings
	( plt -T lw -wb 0 -t "CPU Prices (April 2001)"; \
	  plt -T lw -W .15 .6 .9 .9 cpu.data -f cpu.format1; \
	  plt -T lw -W .15 .125 .9 .425 cpu.data -f cpu.format2 ) | \
	 lwcat -eps >cpu.ps

map.ps:		coastlines.data cities.data
	( plt -T lw coastlines.data 0 1 2 -p c -s xyt \
	      -X -13000 19000 -Y -8000 8000; \
	  plt -T lw cities.data 0 1 -p "Scircle(P/4,Cblue)" -setxy -ex \
	      -X -13000 19000 -Y -8000 8000 )| lwcat -landscape -v -eps >map.ps

mary.ps:	mary.strings mary.data
	plt -T lw mary.data -p 0,1,2c -tf mary.strings -X 0 6 -Y 0 6 -t Poem \
	 | lwcat -eps >mary.ps

#.............................................................................

install:
	-mkdir -p $(MAN1DIR)
	cp $(MANPAGES) $(MAN1DIR)
	cd $(MAN1DIR); gzip -f $(MANPAGES); chmod 644 $(GZMANPAGES)

uninstall:
	cd $(MAN1DIR); rm -f $(GZMANPAGES)
clean:
	rm -f *~ *.log core book.aux book.dvi book.idx book.ilg book.ind \
	 book.lof book.log book.toc gencos genex10 genex14 henon *.exe \
	 *.1.aux *.1.dvi henon.ps.gz henon.ps.bb example7.axes \
	 figure10.ps example10.data labels.ps fontgroup1.ps figure14.ps \
	 example14.data cos2.data $(PDFFIGURES)

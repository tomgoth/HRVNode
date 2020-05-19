% plt.pro	PostScript prolog for plt
% Last revised: 25 March 2009 (GBM)

count /numstack exch def
save 100 dict begin /plt exch def

/ED	{exch def} def
/IN	{exch 72 mul def} def

/CLOSE	{count numstack sub {pop} repeat plt end restore} def

/BF	{gsave INIT} def
/EF	{grestore} def

/LS	{} def
/INIT	{0 setlinecap LS /DMTX matrix currentmatrix def
	/XOFF [0 0 0 -.5 -1 -1 -1 -.5 -.5 0 -.5 -1] def
	/YOFF [0 -.37 -1 -1 -1 -.37 0 0 -.37 -.74 -.74 -.74] def
	/XM 0 def /YM 0 def /XYM 0 def} def

/WSUBST	{} def

/WDEF	{/ury ED /urx ED /lly ED /llx ED
	WSUBST /y1 IN /x1 IN /y0 IN /x0 IN /FSCL ED
	gsave matrix setmatrix
	x0 2 add y0 2 add translate 
	x1 x0 sub 4 sub urx llx sub div
	y1 y0 sub 4 sub ury lly sub div scale
	llx neg lly neg translate
	/PMTX matrix currentmatrix def grestore} def

/M	{newpath PMTX transform moveto} def
/N	{PMTX dtransform rlineto} def
/L	{N currentpoint stroke moveto} def
/F	{closepath 0 eq {fill} {FS} ifelse} def
/FS	{gsave fill grestore gsave 0 0 0 setrgbcolor stroke grestore} def 

/XC	0 def
/YC	0 def
/T	{/ang ED /pos ED /YY ED /XX ED
	XX -32767 eq
	 {YY -32767 eq
	  {XC YC moveto}
	  {0 YY M currentpoint XC exch moveto pop}
	 ifelse}
	 {YY -32767 eq
	  {XX 0 M currentpoint pop YC moveto}
	  {XX YY M}
	 ifelse}
	ifelse
	ang rotate
	/str ED str stringwidth pop
	XOFF pos get mul
	YOFF pos get PS mul XYM FSCL mul add
	rmoveto str show 0 .37 PS mul XYM FSCL mul add rmoveto
	/XYM 0 def SDM currentpoint /YC ED /XC ED} def

/GT	{7 0 T} def
/XT	{/XYM XM neg def 3 0 T /XM 0 def} def
/XTR	{/XYM XM def 7 0 T /XM 0 def} def
/YT	{/XYM YM def 7 90 T /YM 0 def} def
/YTR	{/XYM YM neg def 3 90 T /YM 0 def} def

/XN	{/XM PS def 3 0 T} def
/XNR	{/XM PS def 9 0 T} def
/YN	{-1 1 0 YNUM} def
/YNR	{1 0 0 YNUM} def
/YNUM	{6 3 roll M /str ED str stringwidth pop add /len ED
	len mul .2 PS mul add mul PS -.37 mul rmoveto
	str show /YM len .2 PS mul add YM MAX def} def

/SP	{/y ED /x ED /yp ED /yn ED /str ED
	x y M currentpoint
	str false charpath PBOX newpath
	exch dup add x0 x1 add 2 div sub
	exch dup add y0 y1 add 2 div sub moveto
	str false charpath gsave fill EB} def

/SY	{/y ED /x ED /yp ED /yn ED /st ED
	newpath x y M PS 4 div dup scale
	[{CIR} {UTRI} {DI} {SQ} {TRI}] st 5 mod get exec
	closepath SDM gsave 
	[{1 FILL stroke} {fill} {stroke}] st 5 idiv 3 mod get exec
	EB} def

/EB {	grestore PMTX concat PBOX SDM
	y1 y sub yp le {yp y1 EB2} if 
	y0 y sub yn ge {yn y0 EB2} if} def
/EB2	{newpath x exch M 0 exch N x0 x sub 0 N x1 x0 sub 0 N stroke} def

/CIR	{currentpoint newpath 1.1 0 360 arc} def
/TRI	{0 1.2 rmoveto -1.2 -2.4 rlineto 2.4 0 rlineto} def
/DI	{ 0 -1.5 rmoveto 1.3 1.5 rlineto
	-1.3 1.5  rlineto -1.3 -1.5 rlineto} def
/UTRI	{180 rotate TRI 180 rotate} def
/SQ	{1 1 rmoveto -2 0 rlineto 0 -2 rlineto 2 0
	rlineto} def

/LW	{.2 mul FSCL mul setlinewidth} def
/SF	{FSCL mul /PS ED findfont PS scalefont setfont} def
/DL	{PS mul .05 mul} def

/solid		{{}0} def
/dotted		{[1 DL 3 DL] 0} def
/longdashed	{[7 DL] 0} def
/shortdashed	{[4 DL 5 DL] 0} def
/dotdashed	{[1 DL 5 DL 6 DL 5 DL] 0} def

/MAX	{2 copy lt {exch pop} {pop} ifelse} def
/SDM	{DMTX setmatrix} def
/FILL	{gsave setgray fill grestore} def
/PBOX	{pathbbox /y1 ED /x1 ED /y0 ED /x0 ED} def
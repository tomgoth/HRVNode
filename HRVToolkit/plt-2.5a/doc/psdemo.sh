#! /bin/sh
# file: psdemo.sh         G. Moody                2 April 2001
#                         Last revised:         24 February 2002

cat <<EOF

Welcome to the PostScript plt demonstration!

This script runs 'plt' to display the figures included in the plt Tutorial
and Cookbook. This version of the demo generates PostScript figures and
displays them using GhostScript (via lwcat -gv).

In order to run this demonstration successfully, both 'plt' and GhostScript
must have been installed first.  Each example is shown in a GhostScript
(gv or gsview) window.  Exit from GhostScript when you have finished with each
example in order to see the next example (you can do this by typing 'q' into
the gv window, or <alt>f x in the gsview window).

The most recent version of plt is always available from PhysioNet
(http://www.physionet.org/), and the most recent version of GhostScript is
always available from http://ghostscript.com/.

EOF

echo -n "Press Enter to begin the demonstration: "
read A

PTERM=lw
export PTERM

C=2
F=1
echo "Figure $C.$F"
plt example1.data 0 2 | lwcat -gv

echo Figure 2.2 is a screen image, also shown as figure 2.3 in PostScript form.
F=3
plt heartrate.data | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt heartrate.data -t "Heart rate time series" \
 -x "Time (seconds)" -y "Heart rate (beats per minute)" \
 | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt heartrate.data -t "Heart rate time series" \
 -x "Time (seconds)" -y "Heart rate (beats per minute)" \
 -xa 60 300 15 - 4 -ya 0 80 20 -g grid,sub \
 | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt heartrate.data -t "Heart rate time series" \
 -x "Time (seconds)" -y "Heart rate (beats per minute)" \
 -xa 60 300 15 - 4 -ya 0 80 20 -g grid,sub \
 -p 0,1Scircle \
 | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt heartrate.data -t "Heart rate time series" \
 -x "Time (seconds)" -y "Heart rate (beats per minute)" \
 -xa 60 300 15 - 4 -ya 0 80 20 -g grid,sub \
 -p "0,1Scircle(P/2,Cblue)" \
 | lwcat -gv


F=`expr $F + 1`;
echo "Figure $C.$F: "
PTERM=lw pltf 's(40*x)*s(3*x)' 0 5 .01 | lwcat -gv

C=`expr $C + 1`; F=1
echo "Figure $C.$F"
plt :s2,1024,2049,1 ecg.dat -cz 8 .00781 -F"p 0,1n(Cred) 0,2n(Cblue)" | \
 lwcat -gv

C=`expr $C + 1`; F=1
echo "Figure $C.$F"
plt -f coords.format | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt -f example8.format | lwcat -gv

C=`expr $C + 1`; F=1
echo "Figure $C.$F"
plt example1.data 0 2\
    -x "time in seconds" -y "amplitude in cm" -t "Time vs Amp" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt example1.data 0 2 -f example3.format | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
./henon | plt % -p s. -X -1.5 1.5 -Y -.5 .5 -t "Henon Attractor" | lwcat -gv

C=`expr $C + 1`; F=1
echo "Figure $C.$F"
plt example4.data 0 1 2 -F"\
    fs helvetica longdashed dotted\
    p c" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt symbols.dat -f symbols.format | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p "0,1C(G.90)" -t "Plotstyle C" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1,2e+Z -t "Plotstyle e+Z" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1,2e-X -t "Plotstyle e-X" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p "0,1,2e:+" -t "Plotstyle e:+" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1,2E+0 -t "Plotstyle E+0" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p "0,1,2E-ftriangle" -t "Plotstyle E-ftriangle" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p "0,1,2E:square" -t "Plotstyle E:square" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p "0,1,2f(G.90)" -t "Plotstyle f" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1i -t "Plotstyle i" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1,0,3l -t "Plotstyle l" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 1m -t "Plotstyle m" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1n -t "Plotstyle n" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p "0,1N(G.90)" -t "Plotstyle N" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1,2o -t "Plotstyle o" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p "0,1,2O(G.90)" -t "Plotstyle O" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1sO -t "Plotstyle sO" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1Sfdiamond -t "Plotstyle Sfdiamond" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt styles.data -p 0,1,3t -t "Plotstyle t" -ts "Do Re Mi Fa Sol La Ti" CB | \
 lwcat -gv

C=`expr $C + 1`; F=1
echo "Figure $C.$F"
plt example5.data 0 3 0 2 1 -F"p s+ s* m" \
    -x "x axis" -y "y axis" -t "plot of y=x; y=2x and y=3x" | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
( plt example7.data 0 1 -f example7.format
  plt example7.data 0 2 -f example7.axes -o )  | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt example10.data -f example10.format | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
( plt ldemo.data 2 3 -t"FREQUENCY RESPONSE OF THE FILTER" \
    -x"FREQUENCY IN HERTZ" -y"THIS IS THE AMPLITUDE" -sf all P16
  plt ldemo.data 2 3 -t"FREQUENCY RESP." -x"FREQUENCY" \
    -y"AMPLITUDE"  -W .3 .3 .5 .45 -se -sf all P12
  plt ldemo.data 2 3 -t"FREQUENCY RESPONSE" -x"FREQUENCY" \
    -y"AMPLITUDE"  -W .6 .55 .9 .8 -se  -sf all P14 ) | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
( plt -wm 0 -t "This is the main title for the plot"
  plt example11.data 0 1 -wm 1 -t "Window 1" ) | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
( plt -wb 0 -t "This is the main title for the plot"
  plt example11.data 0 1 -wb 1 -t "Window 1"
  plt example11.data 0 2 -wb 2 -t "Window 2" ) | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
( plt -wq 0 -t "This is the main title for the plot"
  plt example11.data 0 1 -wq 1 -t "Window 1"
  plt example11.data 0 2 -wq 2 -t "Window 2"
  plt example11.data 0 3 -wq 3 -t "Window 3"
  plt example11.data 0 4 -wq 4 -t "Window 4" )  | lwcat -gv

C=`expr $C + 1`; F=1
echo "Figure $C.$F"
plt example9.data -f example9.format | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt cos2.data 0 1 -f labels.format | lwcat -gv

C=`expr $C + 1`; F=1
echo "Figure $C.$F"
plt -f flowchart.format | lwcat -gv

C=`expr $C + 1`; F=1
C=`expr $C + 1`; F=1
echo "Figure $C.$F"
plt -f fonts.format | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt -f linestyles.format | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt example14.data -f fontgroup.format | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt example14.data -f example14.format | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
plt -f colors.format | lwcat -gv

C=`expr $C + 1`; F=1
echo "Figure $C.$F"
( plt -wq 0 -t"THE TITLE FOR THE ENTIRE GRAPH GOES HERE"
  plt ldemo.data 2 3 -wqs 1 -lx -g in -t"LPF: Log Freq & Ticks in"
  plt ldemo.data 4 5 -wqs 4 -lx -ly -g both -t"LPF: Log Freq & Ampl"
  plt ldemo.data 0 1 -wqs 3 -lx e -g out  -t"Alternate base & Ticks out"
  plt ldemo.data 6 7 -wqs 2 -lx -ly - yes -g grid,sym,out \
    -t"Log axes with grid" ) | lwcat -gv

F=`expr $F + 1`
echo "Figure $C.$F"
( plt power.data 0 1 2 3 4 5 -f conf1.format
  plt power.data 0 6 7 8 9 10 -f conf2.format ) | lwcat -gv

cat <<EOF
This concludes the demonstration of plt.
EOF
exit 0



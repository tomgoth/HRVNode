#! /bin/sh
# file: xdemo.sh          G. Moody                2 April 2001
#                         Last revised:         16 November 2002
# This script runs 'plt' to display the figures included in the plt Tutorial
# and Cookbook.

if [ x$DISPLAY = x ]
  then
    case `uname` in
      Darwin) if [ ! -d /usr/X11R6 ]
        then
          echo "This script requires a running X server, such as XDarwin"
	  echo "(freely available from http://fink.sourceforge.net/)."
        fi
        echo "Start XDarwin before running this script." ;;
      CYGWIN) if [ ! -d /usr/X11R6 ]
        then
         echo "This script requires a running X server, such as Cygwin/XFree86"
	 echo "(freely available from http://www.cygwin.com/)."
        fi
        echo "Start Cygwin/XFree86 before running this script." ;;
      *) echo "Start your X server before running this script." ;;
    esac
    exit
fi

echo "Welcome to the plt on-screen demonstration!"

C=2
F=1
plt example1.data 0 2

cat <<EOF

You should now see figure 2.1 from the plt Tutorial and Cookbook, in a window
on your screen (look for the window named "Plt Window").  If the window has not
been created, plt may not have been installed properly on this system; in this
case, please read Appendix D of the plt Tutorial and Cookbook to see how to
install plt.  The most recent version of plt is always available from PhysioNet
(http://www.physionet.org/).

EOF

echo Figure 2.2 is a screen image, also shown as figure 2.3 in PostScript form.
F=3
echo -n "Press Enter to view Figure $C.$F: "
read A
plt heartrate.data

F=`expr $F + 1`;
echo -n "Press Enter to view Figure $C.$F: "
read A
plt heartrate.data -t "Heart rate time series" \
 -x "Time (seconds)" -y "Heart rate (beats per minute)"

F=`expr $F + 1`;
echo -n "Press Enter to view Figure $C.$F: "
read A
plt heartrate.data -t "Heart rate time series" \
 -x "Time (seconds)" -y "Heart rate (beats per minute)" \
 -xa 60 300 15 - 4 -ya 0 80 20 -g grid,sub 

F=`expr $F + 1`;
echo -n "Press Enter to view Figure $C.$F: "
read A
plt heartrate.data -t "Heart rate time series" \
 -x "Time (seconds)" -y "Heart rate (beats per minute)" \
 -xa 60 300 15 - 4 -ya 0 80 20 -g grid,sub \
 -p 0,1Scircle

F=`expr $F + 1`;
echo -n "Press Enter to view Figure $C.$F: "
read A
plt heartrate.data -t "Heart rate time series" \
 -x "Time (seconds)" -y "Heart rate (beats per minute)" \
 -xa 60 300 15 - 4 -ya 0 80 20 -g grid,sub \
 -p "0,1Scircle(P/2,Cblue)"

F=`expr $F + 1`;
echo -n "Press Enter to view Figure $C.$F: "
read A
pltf 's(40*x)*s(3*x)' 0 5 .01

C=`expr $C + 1`; F=1
echo -n "Press Enter to view Figure $C.$F: "
read A
plt :s2,1024,2049,1 ecg.dat -cz 8 .00781 -F"p 0,1n(Cred) 0,2n(Cblue)"

C=`expr $C + 1`; F=1
echo -n "Press Enter to view Figure $C.$F: "
read A
plt -f coords.format

F=`expr $F + 1`
cat <<EOF
Font scaling on the X display does not exactly match PostScript font scaling,
so the next example may appear odd.  See this example in the plt Tutorial and
Cookbook.
EOF

echo -n "Press Enter to view Figure $C.$F: "
read A
plt -f example8.format

C=`expr $C + 1`; F=1
echo -n "Press Enter to view Figure $C.$F: "
read A
plt example1.data 0 2\
    -x "time in seconds" -y "amplitude in cm" -t "Time vs Amp"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt example1.data 0 2 -f example3.format

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F, the animated Hénon attractor plot:"
read A
./henon | plt % -p s. -X -1.5 1.5 -Y -.5 .5 -t "Hénon Attractor"

C=`expr $C + 1`; F=1
echo -n "Press Enter to view Figure $C.$F: "
read A
plt example4.data 0 1 2 -F"\
    fs helvetica longdashed dotted\
    p c"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt symbols.dat -f symbols.format

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p "0,1C(G.70)" -t "Plotstyle C"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1,2e+Z -t "Plotstyle e+Z"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1,2e-X -t "Plotstyle e-X"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p "0,1,2e:+" -t "Plotstyle e:+"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1,2E+0 -t "Plotstyle E+0"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p "0,1,2E-ftriangle" -t "Plotstyle E-ftriangle"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p "0,1,2E:square" -t "Plotstyle E:square"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p "0,1,2f(G.70)" -t "Plotstyle f"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1i -t "Plotstyle i"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1,0,3l -t "Plotstyle l"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 1m -t "Plotstyle m"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1n -t "Plotstyle n"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p "0,1N(G.70)" -t "Plotstyle N"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1,2o -t "Plotstyle o"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p "0,1,2O(G.70)" -t "Plotstyle O"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1sO -t "Plotstyle sO"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1Sfdiamond -t "Plotstyle Sfdiamond"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt styles.data -p 0,1,3t -t "Plotstyle t" -ts "Do Re Mi Fa Sol La Ti" CB

C=`expr $C + 1`; F=1
echo -n "Press Enter to view Figure $C.$F: "
read A
plt example5.data 0 3 0 2 1 -F"p s+ s* m" \
    -x "x axis" -y "y axis" -t "plot of y=x; y=2x and y=3x"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt example7.data 0 1 -f example7.format
plt example7.data 0 2 -f example7.axes -o

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt example10.data -f example10.format

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt ldemo.data 2 3 -t"FREQUENCY RESPONSE OF THE FILTER" \
    -x"FREQUENCY IN HERTZ" -y"THIS IS THE AMPLITUDE" -sf all P16
plt ldemo.data 2 3 -t"FREQUENCY RESP." -x"FREQUENCY" \
    -y"AMPLITUDE"  -W .3 .3 .5 .45 -se -sf all P12
plt ldemo.data 2 3 -t"FREQUENCY RESPONSE" -x"FREQUENCY" \
    -y"AMPLITUDE"  -W .6 .55 .9 .8 -se  -sf all P14

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt -wm 0 -t "This is the main title for the plot"
plt example11.data 0 1 -wm 1 -t "Window 1"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt -wb 0 -t "This is the main title for the plot"
plt example11.data 0 1 -wb 1 -t "Window 1"
plt example11.data 0 2 -wb 2 -t "Window 2"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt -wq 0 -t "This is the main title for the plot"
plt example11.data 0 1 -wq 1 -t "Window 1"
plt example11.data 0 2 -wq 2 -t "Window 2"
plt example11.data 0 3 -wq 3 -t "Window 3"
plt example11.data 0 4 -wq 4 -t "Window 4"

C=`expr $C + 1`; F=1
echo -n "Press Enter to view Figure $C.$F: "
read A
plt example9.data -f example9.format

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt cos2.data 0 1 -f labels.format

C=`expr $C + 1`; F=1
echo -n "Press Enter to view Figure $C.$F: "
read A
plt -f flowchart.format

C=`expr $C + 1`; F=1
C=`expr $C + 1`; F=1
echo -n "Press Enter to view Figure $C.$F: "
read A
plt -f fonts.format

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt -f linestyles.format

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt example14.data -f fontgroup.format

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt example14.data -f example14.format

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt -f colors.format

C=`expr $C + 1`; F=1
echo -n "Press Enter to view Figure $C.$F: "
read A
plt -wq 0 -t"THE TITLE FOR THE ENTIRE GRAPH GOES HERE"
plt ldemo.data 2 3 -wqs 1 -lx -g in -t"LPF: Log Freq & Ticks in"
plt ldemo.data 4 5 -wqs 4 -lx -ly -g both -t"LPF: Log Freq & Ampl"
plt ldemo.data 0 1 -wqs 3 -lx e -g out  -t"Alternate base & Ticks out"
plt ldemo.data 6 7 -wqs 2 -lx -ly - yes -g grid,sym,out -t"Log axes with grid"

F=`expr $F + 1`
echo -n "Press Enter to view Figure $C.$F: "
read A
plt power.data 0 1 2 3 4 5 -f conf1.format
plt power.data 0 6 7 8 9 10 -f conf2.format

cat <<EOF
This concludes the demonstration of plt.
Press Esc in the plt window to close it.
EOF
exit 0


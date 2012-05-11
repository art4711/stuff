set autoscale
set xtic auto
set ytic auto
set ylabel "time per event (ns)"
set yrange [0:]
set xlabel "elements"
#set logscale x
plot "test.out" using 1:2 title "insert" with linespoints, \
     "test.out" using 1:3 title "update" with linespoints, \
     "test.out" using 1:4 title "remove" with linespoints

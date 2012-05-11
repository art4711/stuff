set autoscale
set xtic auto
set ytic auto
set ylabel "time per event (ns)"
set yrange [0:]
set xlabel "elements"
#set logscale x
plot "test.out" using 1:2 title "insert" with lines, \
     "test.out" using 1:3 title "update" with lines, \
     "test.out" using 1:4 title "remove" with lines, \
     "reference.out" using 1:2 title "ref insert" with lines, \
     "reference.out" using 1:3 title "ref update" with lines, \
     "reference.out" using 1:4 title "ref remove" with lines

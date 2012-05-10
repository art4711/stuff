set autoscale
set xtic auto
set ytic auto
set ylabel "time per event (ns)"
set yrange [0:]
set xlabel "timeouts"
#set logscale x
plot "test.out" using 1:4 title "add" with lines, \
     "test.out" using 1:5 title "del" with lines, \
     "test.out" using 1:6 title "fire" with lines, \
     "test.out.old" using 1:4 title "oadd" with lines, \
     "test.out.old" using 1:5 title "odel" with lines, \
     "test.out.old" using 1:6 title "ofire" with lines

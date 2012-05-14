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
     "test-avl.out" using 1:4 title "avl add" with lines, \
     "test-avl.out" using 1:5 title "avl del" with lines, \
     "test-avl.out" using 1:6 title "avl fire" with lines, \
     "test-heap.out" using 1:4 title "heap add" with lines, \
     "test-heap.out" using 1:5 title "heap del" with lines, \
     "test-heap.out" using 1:6 title "heap fire" with lines

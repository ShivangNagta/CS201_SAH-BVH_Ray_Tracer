set terminal png size 800,600
set output 'benchmark_results.png'
set title 'Ray Tracing Performance: BVH vs No BVH'
set xlabel 'Number of Spheres'
set ylabel 'Time (seconds)'
set xtics ('1K' 1000, '10K' 10000)
set grid xtics ytics
set xtics scale 1
set ytics scale 1
set xtics rotate by -45
set key top left
set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 1.5
set style line 2 lc rgb '#dd181f' lt 1 lw 2 pt 7 ps 1.5
plot 'benchmark_data.txt' using 1:2 with linespoints ls 1 title 'No BVH', \
     'benchmark_data.txt' using 1:3 with linespoints ls 2 title 'With BVH'

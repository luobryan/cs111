#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2b_1.png ... 
#	lab2b_2.png ...  
#	lab2b_3.png
#	lab2b_4.png
#   lab2b_5.png
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "List-1: Throughput (ops/sec) vs. Number of Threads"
set xlabel "Number of Threads"
set logscale x 2
set ylabel "Throughput (ops/sec)"
set logscale y 10
set output 'lab2b_1.png'

# grep out  
plot \
     "< grep -E 'list-none-s,[0-9]+,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'spin lock' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-m,[0-9]+,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'mutex lock' with linespoints lc rgb 'green'


set title "List-2: Wait-for-lock time and avg. time per operation vs. number of threads for Mutex lock"
set xlabel "Threads"
set logscale x 2
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep -E 'list-none-m,[0-9]+,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'wait-for-lock time' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-m,[0-9]+,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'avg. time per operation' with linespoints lc rgb 'green'
     
set title "List-3: Unprotected Threads and Iterations that run without failure (yield=id, 4 lists)"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep -E 'list-id-none,[0-9]+,[0-9]+,4,' lab2b_list.csv" using ($2):($3) \
	title 'no protection' with points lc rgb 'green', \
     "< grep -E 'list-id-m,[0-9]+,[0-9]+,4,' lab2b_list.csv" using ($2):($3) \
	title 'mutex protection' with points lc rgb 'red', \
     "< grep -E 'list-id-s,[0-9]+,[0-9]+,4,' lab2b_list.csv" using ($2):($3) \
	title 'spin-lock protection' with points lc rgb 'blue'

set title "List-4: Aggregated throughput (ops/sec) vs. number of threads (mutex sync)"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (ops/sec)"
set logscale y 10
set output 'lab2b_4.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep -E 'list-none-m,[0-9]+,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title '1 list' with linespoints lc rgb 'green', \
     "< grep -E 'list-none-m,[0-9]+,1000,4,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title '4 lists' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-m,[0-9]+,1000,8,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title '8 lists' with linespoints lc rgb 'violet', \
    "< grep -E 'list-none-m,[0-9]+,1000,16,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title '16 lists' with linespoints lc rgb 'blue'

set title "List-5: Aggregated throughput (ops/sec) vs. number of threads (spin-lock sync)"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (ops/sec)"
set logscale y 10
set output 'lab2b_5.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep -E 'list-none-s,[0-9]+,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title '1 list' with linespoints lc rgb 'green', \
     "< grep -E 'list-none-s,[0-9]+,1000,4,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title '4 lists' with linespoints lc rgb 'red', \
     "< grep -E 'list-none-s,[0-9]+,1000,8,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title '8 lists' with linespoints lc rgb 'violet', \
    "< grep -E 'list-none-s,[0-9]+,1000,16,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title '16 lists' with linespoints lc rgb 'blue'


    



     


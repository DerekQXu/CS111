#! /usr/bin/gnuplot
# NAME: Derek Xu
# EMAIL: derekqxu@g.ucla.edu
# ID: 704751588

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "Scalability-1: Throughput of Synchronized Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operation/sec)"
set logscale y 10
set output 'lab2b_1.png'
plot \
     "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin_Lock' with linespoints lc rgb 'green'

set title "Scalability-2: Per-operation Times for Mutex Protected List Operations"
set xlabel "Threads"
set logscale x 2
set ylabel "mean time/operation (ns)"
set logscale y 10
set output 'lab2b_2.png'
plot \
     "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'Wait for Lock' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'Average Time per Operation' with linespoints lc rgb 'green'

set title "Scalability-3: Correct Synchronization of Partitioned Lists"
set xlabel "Threads"
set logscale x 10
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
     "< grep 'list-id-none' lab2b_list.csv" using ($2):($3) \
	title 'Unprotected' with points lc rgb 'red', \
     "< grep 'list-id-m' lab2b_list.csv" using ($2):($3) \
	title 'Mutex' with points lc rgb 'green', \
     "< grep 'list-id-s' lab2b_list.csv" using ($2):($3) \
	title 'Spin_Lock' with points lc rgb 'blue'

set title "Scalability-4: Throughput of Mutex Synchronized list"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operation/sec)"
set logscale y 10 
set output 'lab2b_4.png'
plot \
     "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 List' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,.*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 List' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,.*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 List' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,.*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 List' with linespoints lc rgb 'orange'

set title "Scalability-5: Throughput of Linked List Synchronized list"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operation/sec)"
set logscale y 10 
set output 'lab2b_5.png'
plot \
     "< grep 'list-none-s,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 List' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,.*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 List' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,.*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 List' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,.*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 List' with linespoints lc rgb 'orange'

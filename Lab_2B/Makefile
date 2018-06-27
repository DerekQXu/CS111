# NAME: Derek Xu
# EMAIL: derekqxu@g.ucla.edu
# ID: 704751588

CC = gcc
CFLAGS = -Wall -Wextra -pthread -g -lprofiler

build:
	$(CC) $(CFLAGS) lab2_list.c SortedList.c -o lab2_list

clean:
	rm -rf lab2_list *.tar.gz

tests: clean build
	touch lab2b_list.csv
	rm lab2b_list.csv
	touch lab2b_list.csv
	#Mutex synchronized list
	./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv
	#Spin-lock synchronized list
	./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv
	#Correctness (no sync): 
	-./lab2_list --threads=1 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	#Correctness (sync+yields): 
	./lab2_list --threads=1 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	#Correctness (sync+no yields): 
	./lab2_list --threads=1 --lists=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --lists=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --lists=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --lists=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --lists=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --lists=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --lists=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --lists=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --lists=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --lists=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --lists=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --lists=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --lists=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --lists=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --lists=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --lists=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --lists=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --lists=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --lists=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --lists=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --lists=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --lists=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --lists=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --lists=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --lists=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --lists=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --lists=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --lists=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --lists=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --lists=16 --iterations=1000 --sync=s >> lab2b_list.csv

profile:
	touch profile.out
	rm profile.out
	touch profile.out
	rm -f ./raw.gperf
	LD_PRELOAD=/u/ee/ugrad/qiang/lib/libprofiler.so.0
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf >> profile.out
	pprof --list=list_func ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

graphs: tests
	./lab2b_list.gp

dist: graphs profile
	tar -czvf lab2b-704751588.tar.gz SortedList.h SortedList.c lab2_list.c \
	Makefile README lab2b_list.csv profile.out lab2b_list.gp \
	lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png

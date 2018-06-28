# NAME: Derek Xu
# EMAIL: derekqxu@g.ucla.edu
# ID: 704751588

CC = gcc
CFLAGS = -Wall -Wextra -lmraa -lm -pthread

build:
	$(CC) $(CFLAGS) -o lab4b lab4b.c

clean:
	rm -f lab4b *.tar.gz lab4b *.txt

dist:
	tar -czvf lab4b-704751588.tar.gz lab4b.c auto_check.sh Makefile README

check: clean build
	@ ./auto_check.sh

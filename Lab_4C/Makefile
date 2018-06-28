# NAME: Derek Xu
# EMAIL: derekqxu@g.ucla.edu
# ID: 704751588

CC = gcc
CFLAGS = -Wall -Wextra -lmraa -lm -pthread

build:
	$(CC) $(CFLAGS) -o lab4c_tcp lab4c_tcp.c
	$(CC) $(CFLAGS) -lssl -lcrypto -o lab4c_tls lab4c_tls.c

clean:
	rm -f lab4c_tcp lab4c_tls *.tar.gz

dist:
	tar -czvf lab4c-704751588.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README

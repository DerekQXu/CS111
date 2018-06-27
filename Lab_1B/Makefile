# NAME: Derek Xu
# EMAIL: derekqxu@g.ucla.edu
# ID: 704751588

CC = gcc
CFLAGS = -Wall -Wextra -lz

default:
	$(CC) $(CFLAGS) lab1b-server.c -o lab1b-server
	$(CC) $(CFLAGS) lab1b-client.c -o lab1b-client

client:
	$(CC) $(CFLAGS) lab1b-client.c -o lab1b-client

server:
	$(CC) $(CFLAGS) lab1b-server.c -o lab1b-server

clean:
	rm -f lab1b-server lab1b-client *.tar.gz

dist:
	tar -czvf lab1b-704751588.tar.gz lab1b-client.c lab1b-server.c Makefile README

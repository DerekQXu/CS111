# NAME: Derek Xu
# EMAIL: derekqxu@g.ucla.edu
# ID: 704751588

CC = gcc
CFLAGS = -g -Wall -Wextra

INPUT = hello world!
OUTPUT = foobar
ERROR = 0

default:
	$(CC) $(CFLAGS) lab0.c -o lab0

clean:
	rm -f lab0 *.tar.gz *.txt

dist:
	tar -czvf lab0-704751588.tar.gz lab0.c Makefile backtrace.png breakpoint.png README

check: setup data_comparison segfault catch unreadable unwrittable

setup:
	@ $(CC) $(CFLAGS) lab0.c -o lab0

data_comparison_setup:
	@ echo "$(INPUT)" > input.txt
	@ echo "$(OUTPUT)" > output.txt
	@ ./lab0 --input=input.txt --output=output.txt

data_comparison: data_comparison_setup
	@ if [ $$? -eq 0 ]; then \
		echo return value on success: SUCCESS; \
	else \
		echo return value on success: FAILURE; \
	fi
	@ $(eval NEW_OUTPUT=$(shell cat output.txt))
	@ if [ "$(INPUT)" = "$(NEW_OUTPUT)" ]; then \
		echo file comparison on success: SUCCESS; \
	else \
		echo file comparison on success: FAILURE; \
	fi

segfault_setup:
	-@ ./lab0 --segfault || echo $$? > output.txt

segfault: segfault_setup
	@ $(eval EXIT_CODE=$(shell cat output.txt))
	@ if [ 139 = $(EXIT_CODE) ]; then \
		echo segmentation fault without handler: SUCCESS; \
	else \
		echo segmentation fault without handler: FAILURE; \
	fi

catch_setup:
	-@ ./lab0 --segfault --catch || echo $$? > output.txt

catch: catch_setup
	@ $(eval EXIT_CODE=$(shell cat output.txt))
	@ if [ 4 = $(EXIT_CODE) ]; then \
		echo segmentation fault with handler: SUCCESS; \
	else \
		echo segmentation fault with handler: FAILURE; \
	fi

unreadable_setup:
	@ echo "$(INPUT)" > input.txt
	@ echo "$(OUTPUT)" > output.txt
	@ chmod 000 input.txt
	-@ ./lab0 --input=input.txt --output=output.txt || echo $$? > output.txt

unreadable: unreadable_setup
	@ $(eval EXIT_CODE=$(shell cat output.txt))
	@ if [ 2 -eq $(EXIT_CODE) ]; then \
		echo unreadable file handling: SUCCESS; \
	else \
		echo unreadable file handling: FAILURE; \
	fi
	@ rm -f input.txt

unwrittable_setup:
	@ echo "$(INPUT)" > input.txt
	@ echo "$(OUTPUT)" > output.txt
	@ chmod 000 output.txt
	-@ ./lab0 --input=input.txt --output=output.txt || echo $$? > input.txt

unwrittable: unwrittable_setup
	@ $(eval EXIT_CODE=$(shell cat input.txt))
	@ if [ 3 -eq $(EXIT_CODE) ]; then \
		echo unwrittable file handling: SUCCESS; \
	else \
		echo unwrittable file handling: FAILURE; \
	fi
	@ rm -f output.txt

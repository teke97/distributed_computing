CC=gcc

CFLAFS=-std=c99 -Wall -pedantic

all: lab1

lab1: 
	$(CC) $(CFLAGS) src/lab1.c -o bin/lab1

clean:
	rm -rf bin/*.o bin/lab1
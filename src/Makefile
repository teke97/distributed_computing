CC=clang-3.5

CFLAFS=-std=c99 -Wall -pedantic
BIN_DIR=../bin/

all: lab1

lab1: lab1.c pipes.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)lab1 lab1.c pipes.c

clean:
	rm -rf $(BIN_DIR)*.o $(BIN_DIR)lab1

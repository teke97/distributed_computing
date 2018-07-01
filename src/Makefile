CC=gcc

CFLAFS=-std=c99 -Wall -pedantic
BIN_DIR=../bin/

all: pa23

pa23: pa23.c pipes.c
	$(CC) $(CFLAGS) pa23.c pipes.c ./lib64/libruntime.so -o $(BIN_DIR)pa23 

clean:
	rm -rf $(BIN_DIR)*.o $(BIN_DIR)pa23

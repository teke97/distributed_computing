CC=clang-3.5

CFLAFS=-std=c99 -Wall -pedantic
BIN_DIR=../bin/

all: pa23

pa23: pa23.c pipes.c
	$(CC) $(CFLAGS) pa23.c pipes.c bank_robbery.c queue.c -o $(BIN_DIR)pa23 -L. -lruntime

clean:
	rm -rf $(BIN_DIR)*.o $(BIN_DIR)pa23

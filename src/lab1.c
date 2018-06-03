#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 
#include "lib/pa1_starter_code/common.h"
#include "lib/pa1_starter_code/ipc.h"  
#include "lib/pa1_starter_code/pa1.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <malloc.h>

void usage(){
	printf("Usage: lab1 -P X,\n\twhere X is a number of process (0<X<=10)\n");
}

int send(void* self, local_id dst, const Message * msg){
	printf("%i", *((int*)self + dst));
}

int child_work(int* fd, int id, int process_number){
	printf(log_started_fmt, id, getpid(), getppid());
	send(fd, 2, "heh");
	printf(log_done_fmt, id);
}


int parent_work(){}

int main(int argc, char *argv[]) {
	int process_number;
	int* pid;
	int* fd;
	if (argc != 3){
		usage();
		return 1;
	}
	if (strcmp("-P", argv[1]) != 0) {
		usage();
		return 2;
	}

	process_number = atoi(argv[2]);

	if (process_number<=0 || process_number > 10){
		usage();
		return 3;
	}

	fd = (int*) malloc ((process_number + 1) ^ 2 * sizeof(int));
	for (int j = 0; j < process_number; j++)
		for (int i = 0; i < process_number; i++){
			if(pipe(fd + i * 2 + j * process_number) != 0)
				return 4;
		}

	pid = (int*) calloc(process_number, sizeof(int)); 

	for (int i = 0; i < process_number; i++){
		pid[i] = fork();
		if( pid[i] == 0) {
			return child_work(fd, i, process_number);
		}
	}
	parent_work();
	while ( wait(NULL) > 0);
}

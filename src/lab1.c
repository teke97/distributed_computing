#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 
#include "lib/pa1_starter_code/common.h"
#include "lib/pa1_starter_code/ipc.h"  
#include "lib/pa1_starter_code/pa1.h"
#include <sys/types.h>
#include <sys/wait.h>


void usage(){
	printf("Usage: lab1 -P X,\n\twhere X is a number of process (0<X<=10)\n");
}



int main(int argc, char *argv[]) {
	int process_number;
	int* pid;
	if (argc != 3){
		usage();
		return 1;
	}
	if (strcmp("-P", argv[1]) != 0) {
		usage();
		return 2;
	}

	process_number = atoi(argv[2]);
	pid = (int*) calloc(process_number, sizeof(int)); 

	if (process_number<=0 || process_number > 10){
		usage();
		return 3;
	}

	for (int i = 0; i < process_number; i++){
		pid[i] = fork();
		if( pid[i] == 0) {
			printf(log_started_fmt, i, getpid(), getppid());
			printf(log_done_fmt, i);
			return 0;	
		}
	}
	
	while ( wait(NULL) > 0);
}

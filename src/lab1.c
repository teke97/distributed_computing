#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 
#include "common.h"
#include "ipc.h"  
#include "pa1.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <malloc.h>
#include "pipes.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void usage(){
	printf("Usage: lab1 -P X,\n\twhere X is a number of process (0<X<=10)\n");
}

int first_stage(IO context){
	char buf[MAX_PAYLOAD_LEN];
	int status;
	sprintf(buf, log_started_fmt, context.id, getpid(), getppid());
	if (status = write(context.events, buf, strlen(buf)) < 0){
		return status;
	}
	return 0;
}

int third_stage(IO context){
	char buf[MAX_PAYLOAD_LEN];
	int status;
	sprintf(buf, log_done_fmt, context.id);
	if (status = write(context.events, buf, strlen(buf)) < 0){
		return status;
	}
	return 0;
}

int child_work(IO context){
	int status;
	
	if (status = first_stage(context) < 0)
		return status;

	if (status = third_stage(context) < 0)
		return status;
	return 0;
}


int parent_work(IO context){
	while ( wait(NULL) > 0);
	return 0;
}

int main(int argc, char *argv[]) {
	int process_number;
	int pid;
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
	

	IO context = init_pipelines(process_number);

	context.proc_num = process_number;

	context.events = open(events_log, O_WRONLY | O_TRUNC);
	context.pipes = open(pipes_log, O_WRONLY | O_TRUNC);
	
	if (context.events < 0 || context.pipes < 0)
		return 5;
	print_pipes(context);
	for (local_id i = 1; i <= process_number; i++){
		pid = fork();
		if (pid < 0) 
			return 6;
		if( pid == 0) {
			context.id = i;
			return child_work(context);
		}
		pid = 0;
	}
	context.id = 0;
	return parent_work(context);
}

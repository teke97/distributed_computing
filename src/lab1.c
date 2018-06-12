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
#include <sys/stat.h>
#include <fcntl.h>

void usage(){
	printf("Usage: lab1 -P X,\n\twhere X is a number of process (0<X<=10)\n");
}


int child_work(IO* pipelines){
	char buf[MAX_PAYLOAD_LEN];
	Message* msg = malloc( sizeof(Message) * pipelines-> process_number);
	close_self(pipelines);
//first
	sprintf( buf, log_started_fmt, pipelines->local_id, getpid(), getppid());
	write(pipelines-> events, buf, strlen(buf));
	send_multicast(pipelines, build_msg("Process started\n", STARTED));
	receive_any(pipelines, msg);
	sprintf( buf, log_received_all_started_fmt, pipelines->local_id);
	write(pipelines-> events, buf, strlen(buf));
	send_multicast(pipelines, build_msg("Process done\n", DONE));
	msg = malloc( sizeof(Message) * pipelines-> process_number);
	receive_any(pipelines, msg);
	sprintf(buf, log_done_fmt, pipelines->local_id);
	write(pipelines-> events, buf, strlen(buf));
	close_all(pipelines);
	return 0;
}


int parent_work(IO* pipelines){
	char buf[MAX_PAYLOAD_LEN];
	close_self(pipelines);
	Message* msg = (Message*) malloc(sizeof(Message) * pipelines-> process_number);
	sprintf( buf, log_started_fmt, pipelines->local_id, getpid(), getppid());
	write(pipelines-> events, buf, strlen(buf));

	receive_any(pipelines, msg);
	sprintf( buf, log_received_all_started_fmt, pipelines->local_id);
	write(pipelines-> events, buf, strlen(buf));
	msg = (Message*) malloc(sizeof(Message) * pipelines-> process_number);
	receive_any(pipelines, msg);
	while ( wait(NULL) > 0);
	sprintf(buf, log_done_fmt, pipelines->local_id);
	write(pipelines-> events, buf, strlen(buf));
	close_all(pipelines);
	return 0;
}

int main(int argc, char *argv[]) {
	int process_number;
	int* pid;
	int* fd;
	if (argc != 3){
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
	IO* pipelines = init_pipelines(process_number);
	
	//print_(pipelines);
	
	pipelines -> events = open(events_log, O_WRONLY | O_TRUNC );
	pipelines -> pipes = open(pipes_log, O_WRONLY | O_TRUNC );

	pipelines -> process_number = process_number;
	
	pid = (int*) calloc(process_number, sizeof(int)); 

	for (local_id i = 1; i <= process_number; i++){
		pid[i] = fork();
		if( pid[i] == 0) {
			pipelines -> local_id = i;
			return child_work(pipelines);
		}
	}
	pipelines -> local_id = 0;
	return parent_work(pipelines);
}

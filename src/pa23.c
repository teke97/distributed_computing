#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 
#include "common.h"
#include "ipc.h"  
#include "pa2345.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <malloc.h>
#include "pipes.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "banking.h"

void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount)
{
    // student, please implement me
}

void usage(){
	printf("Usage: pa23 -p X N1 N2 [N3 ...],\n\twhere X is a number of process (0<X<=10) and N starting balance\n");
}

int close_n_needed(IO context){
	for (local_id i = 0 ; i <= context.proc_num; i++){
		for (local_id j = 0 ; j <= context.proc_num; j++){
			if ( i == j)
				continue;
			if ( i == context.id){
				close(context.pipelines[i][j][1]);
			}
			else {
				if ( j != context.id){
					close(context.pipelines[i][j][0]);
					close(context.pipelines[i][j][1]);
				}
				else {	
					close(context.pipelines[i][j][0]);
				}
			}
		}
	} 
	return 0;
}

int first_stage(IO context){
	char buf[MAX_PAYLOAD_LEN];
	int status;
	sprintf(buf, log_started_fmt, get_physical_time() ,context.id, getpid(), getppid(), context.balance[context.id]);
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	
	send_multicast(&context, build_msg(buf, STARTED));
	
	Message msg;

	for (local_id i = 1; i <= context.proc_num; i++){
		if (context.id == i)
			continue;
		receive_any(&context, &msg);
	}
	
	sprintf(buf, log_received_all_started_fmt,get_physical_time(), context.id);
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	
	return 0;
}


int second_stage(IO context){
	Message msg;
	local_id from = 0;
	/*while(1){
		from = receive_any(&context, &msg);
		
		if (from == 0 && msg.s_header.s_type == STOP){
			printf("%i\n",context.id);
			return 0;
		}

	}*/
	printf("%i\n",receive_blk(&context, from, &msg));
	//printf("%s", msg.s_payload);
	//if (msg.s_header.s_type == STOP)
		//printf("%i\n",context.id);
	return 0;

}

int third_stage(IO context){
	char buf[MAX_PAYLOAD_LEN];
	int status;

	sprintf(buf ,log_done_fmt, get_physical_time(), context.id, context.balance[context.id]);

	send_multicast(&context, build_msg(buf , DONE));
	
	Message msg;

	for (local_id i = 1; i <= context.proc_num; i++){
		if (context.id == i)
			continue;
		receive_any(&context, &msg);
	}
	
	sprintf(buf, log_received_all_done_fmt, get_physical_time(),context.id);
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}

	sprintf(buf, log_done_fmt, get_physical_time(), context.id, context.balance[context.id]);
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	return 0;
}

int child_work(IO context){
	int status;
	close_n_needed(context);
	if ((status = first_stage(context)) < 0)
		return status;
	
	if ((status = second_stage(context)) < 0)
		return status;

	if ((status = third_stage(context)) < 0)
		return status;
	return 0;
}


int parent_work(IO context){
	close_n_needed(context);
	Message msg;
	for (local_id i = 0; i <= context.proc_num; i++){
		if (context.id == i)
			continue;
		receive_any(&context, &msg);
	}

	send_multicast(&context, build_msg("dude", STOP));

	for (local_id i = 0; i <= context.proc_num; i++){
		if (context.id == i)
			continue;
		receive_any(&context, &msg);
	}
	while ( wait(NULL) > 0);
	return 0;
}

int main(int argc, char *argv[]) {
	int process_number;
	int pid;
	if (argc < 4){
		usage();
		return 1;
	}
	if (strcmp("-p", argv[1]) != 0) {
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

	if ( argc != process_number + 3) {
		usage();
		return 8;
	}

	for(local_id i = 1; i <= process_number; i++)
		context.balance[i] = atoi(argv[i + 2]);

	context.events = open(events_log, O_WRONLY | O_APPEND | O_CREAT);
	context.pipes = open(pipes_log, O_WRONLY | O_APPEND | O_CREAT);
	
	if (context.events < 0 || context.pipes < 0)
		return 5;
	//print_pipes(context);
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

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

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount){
	return;
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





void transfer_out(IO* cxt, Message msg){
	char buf[MAX_PAYLOAD_LEN];
	TransferOrder to = *((TransferOrder*) msg.s_payload);
	cxt -> balance -= to.s_amount;
	timestamp_t time = max_t(msg.s_header.s_local_time, cxt->time);
	time = get_lamport_time(cxt);
	cxt -> balance_history.s_history_len = time;
	cxt -> balance_history.s_history[time].s_time = time;
	cxt -> balance_history.s_history[time].s_balance = cxt -> balance;
	cxt -> balance_history.s_history[time].s_balance_pending_in = to.s_amount;

	
	msg.s_header.s_local_time = time;	
	sprintf(buf, log_transfer_out_fmt, cxt -> time, to.s_src, to.s_amount, to.s_dst);

	send(cxt, to.s_dst, &msg );
	write(cxt -> events, buf, strlen(buf));
}


void transfer_in(IO* cxt, Message msg){
	char buf[MAX_PAYLOAD_LEN];
	TransferOrder to = *((TransferOrder*) msg.s_payload);
        cxt -> balance += to.s_amount;
	timestamp_t time = max_t(msg.s_header.s_local_time, cxt->time);
	cxt -> balance_history.s_history_len = time;
	cxt -> balance_history.s_history[time].s_time = time;
	cxt -> balance_history.s_history[time].s_balance = cxt -> balance;
	cxt -> time = time;
	msg.s_header.s_local_time = cxt -> time;
	send(cxt, 0, build_msg(cxt,"", ACK));
	sprintf(buf, log_transfer_in_fmt, cxt -> time, to.s_dst, to.s_amount, to.s_src);
	write(cxt -> events, buf, strlen(buf));
}

int first_stage_child(IO* cxt){
	IO context = *cxt;
	char buf[MAX_PAYLOAD_LEN];
	int status;
	Message msg;
	// Build msg
	sprintf(buf, log_started_fmt, cxt -> time ,context.id, getpid(), getppid(), context.balance);
	// Log start msg in events.log 
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	// Send started msg other process
	send_multicast(cxt , build_msg(cxt,buf, STARTED));
	
	// Receive started msg from other process
	for (local_id i = 1; i <= context.proc_num; i++){
		if (context.id == i)
			continue;
		receive_blk(cxt, i ,&msg);
	}
	// Log recived started msg from other process
	sprintf(buf, log_received_all_started_fmt,cxt -> time, context.id);
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	
	return 0;
}


int is_min(IO* cxt, timestamp_t cache[10]){
	for ( size_t i = 0; i < cxt -> proc_num; i++){
		if ( i + 1 == cxt -> id)
			continue;
		printf("i: %lu t: %hu\n", i, cache[i]);
		if ( cache[cxt -> id - 1] > cache[i])
			return 0;
	}
	return 1;
}

int more_then(IO* cxt, local_id id){
	for (Node* n = cxt -> q -> head;; n = n -> next){
		if ( n -> id == id)
			return 1;
		if ( n -> id == cxt -> id)
			return 0;
	}
}

int request_cs(const void* self){
	IO* cxt = (IO*) self;
	Message msg;

	send_child(cxt, build_msg(cxt, "", CS_REQUEST));
	insert(cxt -> q, create_node(cxt-> id, cxt -> time));

	for (local_id i = 1; i <= cxt -> proc_num; i++){
		memset(&msg, 0, sizeof(Message));
		if (i == cxt -> id)
			continue;
////////	if (cxt -> id == 2)
////////		printf("%hhu\n", i);
		local_id from = receive_any(cxt, &msg);
		switch(msg.s_header.s_type){
			case CS_REPLY:
			////////	if (cxt -> id == 2)
			////////		printf("reply\n");
					break;
			case CS_REQUEST:
			////////	if (cxt -> id == 2)
			////////		printf("request\n");				
					insert( cxt -> q, create_node(from, msg.s_header.s_local_time));
					if ( cxt ->  q -> head -> id != cxt -> id){
						send(cxt, cxt -> q -> head -> id, build_msg(cxt, "", CS_REPLY));
						release(cxt -> q);
					}
					i--;
					break;
		}
		
	}
////////if (cxt -> q -> head -> id == cxt -> id){
////////	return 0;
////////}
////////while(1){
////////	memset(&msg, 0, sizeof(Message));
////////	local_id from = receive_any(cxt, &msg);
////////        switch(msg.s_header.s_type){
////////                case CS_REQUEST:
////////                //              if (cxt -> id == 2)
////////                //                      printf("request\n");
////////                                insert( cxt -> q, create_node(from, msg.s_header.s_local_time));
////////                                send(cxt, from, build_msg(cxt, "", CS_REPLY));
////////                                break;
////////                case CS_RELEASE:
////////                //              if (cxt -> id == 2)
////////                //                      printf("release\n");
////////                                release(cxt -> q);
////////				if (cxt -> q -> head -> id == cxt -> id){
////////					return 0;
////////				}
////////                                break;
////////        }

////////}
	release(cxt -> q);
	return 0;
}
int release_cs(const void* self){
	IO* cxt = (IO*) self;
	//send_child(cxt, build_msg(cxt, "", CS_RELEASE));
	for (; cxt -> q -> head != NULL;){
		send(cxt, cxt -> q -> head -> id, build_msg(cxt, "", CS_REPLY));
		release(cxt -> q);
	}
////////if (cxt -> id == 1)
////////print_Q(cxt -> q);
	return 0;
}

int __wait(IO* cxt){
	Message msg;
	if ( cxt -> q -> head == NULL && cxt -> id == cxt -> proc_num)
		return 0;
	while(1){
		memset(&msg, 0, sizeof(Message));
		local_id from = receive_any(cxt, &msg);
                switch(msg.s_header.s_type){
                        case CS_REQUEST:
                        //              if (cxt -> id == 2)
                        //                      printf("request\n");
                                        send(cxt, from, build_msg(cxt, "", CS_REPLY));
                                        break;
			case DONE:
					return 0;
					break;
                }

	}
}

int second_stage_child(IO* cxt){
	char buf[MAX_PAYLOAD_LEN];
	
	for (int i = 1; i <=  5 * cxt -> id; i++){
		sprintf(buf, log_loop_operation_fmt, cxt -> id, i, cxt -> id * 5); 
		if (cxt -> mutex == 1)
		request_cs(cxt);
		print(buf);
		if (cxt -> mutex == 1)
		release_cs(cxt);
	}
	__wait(cxt);
	return 0;
}


int third_stage_child(IO* cxt){
	IO context = *cxt;
	char buf[MAX_PAYLOAD_LEN];
	int status;
	Message msg;

	// Build msg
	sprintf(buf ,log_done_fmt, cxt -> time, context.id, context.balance);
	// Send stop done other process
	send_multicast(cxt, build_msg(cxt,buf , DONE));
	// Receive done msg from other process
	for (local_id i = 1; i < context.proc_num; i++){
		if (context.id == i)
			continue;
		//receive_any(&context, &msg);
		receive_blk(cxt, i ,&msg);
	}
	// Build receive all done msg
	sprintf(buf, log_received_all_done_fmt, cxt -> time,context.id);
	// Log receive all done msg
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	// Build done msg
	sprintf(buf, log_done_fmt, cxt -> time, context.id, context.balance);
	// Log done msg
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	return 0;
}

int child_work(IO context){
	close_n_needed(context);
	context.q = init_Q();
	first_stage_child(&context);
	second_stage_child(&context);
	third_stage_child(&context);

	return 0;
}

int first_stage_parent(IO* cxt){
	char buf[MAX_PAYLOAD_LEN];
	int status;
	Message msg;

	// Receive started msg from other process
	for (local_id i = 1; i <= cxt->proc_num; i++){
		if (cxt->id == i)
			continue;
		receive_any(cxt, &msg);
	}
	// Log recived started msg from other process
	sprintf(buf, log_received_all_started_fmt,cxt->time, cxt->id);
	if ((status = write(cxt->events, buf, strlen(buf))) < 0){
		return status;
	}
	
	return 0;
}

int second_stage_parent(IO* cxt){
	return 0;
}
int third_stage_parent(IO* cxt){
	char buf[MAX_PAYLOAD_LEN];
	int status;
	Message msg;
	// Receive done msg from other process
	for (local_id i = 1; i <= cxt->proc_num; i++){
		if (cxt->id == i)
			continue;
		receive_any(cxt, &msg);
	}
	// Build receive all done msg
	sprintf(buf, log_received_all_done_fmt, cxt->time,cxt->id);
	// Log receive all done msg
	if ((status = write(cxt->events, buf, strlen(buf))) < 0){
		return status;
	}
	return 0;
}

int parent_work(IO context){
	int wstatus;

	close_n_needed(context);

	first_stage_parent(&context);
	second_stage_parent(&context);
	third_stage_parent(&context);

	while ( wait(&wstatus) > 0){
////////	if (WIFEXITED(wstatus)) {
////////               printf("exited, status=%d\n", WEXITSTATUS(wstatus));
////////           } else if (WIFSIGNALED(wstatus)) {
////////               printf("killed by signal %d\n", WTERMSIG(wstatus));
////////           } else if (WIFSTOPPED(wstatus)) {
////////               printf("stopped by signal %d\n", WSTOPSIG(wstatus));
////////           } else if (WIFCONTINUED(wstatus)) {
////////               printf("continued\n");
////////           }
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int process_number;
	int pid;
	int offset = 0;
	if (argc < 2){
		usage();
		return 1;
	}
	if (strcmp("--mutexl", argv[1]) == 0){
		offset = 1;
	}
	else if(strcmp("-p", argv[1]) != 0){
		usage();
		return 2;
	}

	process_number = atoi(argv[2 + offset]);

	if (process_number<=1 || process_number > 10){
		usage();
		return 3;
	}
	

	IO context = init_pipelines(process_number);

	context.proc_num = process_number;

	context.events = open(events_log, O_WRONLY | O_APPEND | O_CREAT, 0666);
	context.pipes = open(pipes_log, O_WRONLY | O_APPEND | O_CREAT, 0666);
	
	if (context.events < 0 || context.pipes < 0)
		return 5;
	//print_pipes(context);
	for (local_id i = 1; i <= process_number; i++){
		pid = fork();
		if (pid < 0) 
			return 6;
		if( pid == 0) {
			context.id = i;
			context.time = 0;
			context.mutex = offset;
			context.m = 0;
			return child_work(context);
		}
		pid = 0;
	}
	context.id = 0;
	context.time = 0;
	context.mutex = offset;
	return parent_work(context);
}

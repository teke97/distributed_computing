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
	IO context = *((IO*)parent_data);
	TransferOrder transfer_order;
	Message msg;	

	transfer_order.s_src = src;
	transfer_order.s_dst = dst;
	transfer_order.s_amount = amount;
	send(parent_data, src, build_msg((char*) &transfer_order, TRANSFER));
	receive_blk(parent_data, dst, &msg);
	printf("Transaction from %hhu to %hhu with %hu$$$ is completed.\nType:%hu\n", src, dst, amount,msg.s_header.s_type);
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


//void set_history(IO* cxt,timestamp_t time, balance_t old_balance , balance_t new_balance){
//	for (timestamp_t i = 0; i < time; i++){
//		if (ctx -> balance_history.[i].s_time < time){
//			ctx -> balance_history.[i].
//		}
//	}
//}
void print_balance_s(BalanceState bs){
	printf("Time: %hu\tBalance: %hu\n", bs.s_time, bs.s_balance );
}

void print_balance_h(BalanceHistory bh){
	printf("Id: %hhu\nLen: %hhu\n", bh.s_id, bh.s_history_len);
	for (size_t i = 0; i <= bh.s_history_len + 1; i++){
		print_balance_s(bh.s_history[i]);
	}
}
void fix_h(BalanceHistory* bh){
        for (size_t i = 1; i <= bh -> s_history_len; i++){
		if (bh -> s_history[i].s_time != i){
			bh -> s_history[i].s_time = i;
			bh -> s_history[i].s_balance = bh -> s_history[i-1].s_balance;
		}
        }
}


void transfer_out(IO* cxt, Message msg){
	char buf[MAX_PAYLOAD_LEN];
	TransferOrder to = *((TransferOrder*) msg.s_payload);
	sprintf(buf, log_transfer_out_fmt, get_physical_time(), to.s_src, to.s_amount, to.s_dst);
	cxt -> balance -= to.s_amount;
	timestamp_t time = get_physical_time();
	cxt -> balance_history.s_history_len = time;
	cxt -> balance_history.s_history[time].s_time = time;
	cxt -> balance_history.s_history[time].s_balance = cxt -> balance;
	send(cxt, to.s_dst, &msg );
	write(cxt -> events, buf, strlen(buf));
}

void transfer_in(IO* cxt, Message msg){
	char buf[MAX_PAYLOAD_LEN];
			TransferOrder to = *((TransferOrder*) msg.s_payload);
			sprintf(buf, log_transfer_in_fmt, get_physical_time(), to.s_dst, to.s_amount, to.s_src);
                        cxt -> balance += to.s_amount;
			timestamp_t time = get_physical_time();
			cxt -> balance_history.s_history_len = time;
			cxt -> balance_history.s_history[time].s_time = time;
			cxt -> balance_history.s_history[time].s_balance = cxt -> balance;
			send(cxt, 0, build_msg("", ACK));
			write(cxt -> events, buf, strlen(buf));
}

int first_stage_child(IO* cxt){
	IO context = *cxt;
	char buf[MAX_PAYLOAD_LEN];
	int status;
	Message msg;
	// Build msg
	sprintf(buf, log_started_fmt, get_physical_time() ,context.id, getpid(), getppid(), context.balance);
	// Log start msg in events.log 
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	// Send started msg other process
	send_multicast(&context, build_msg(buf, STARTED));
	
	// Receive started msg from other process
	for (local_id i = 1; i <= context.proc_num; i++){
		if (context.id == i)
			continue;
		receive_blk(&context,i ,&msg);
		if ( msg.s_header.s_type == TRANSFER){
			transfer_in(cxt, msg);
			i--;
		}
	}
	// Log recived started msg from other process
	sprintf(buf, log_received_all_started_fmt,get_physical_time(), context.id);
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	
	return 0;
}

int second_stage_child(IO* cxt){
	IO context = *cxt;
	Message msg;
	local_id from;
        while(1){
        	from = receive_any(&context, &msg);
		
		if (from == 0){
			if(msg.s_header.s_type == TRANSFER){
				transfer_out(cxt, msg);
			}
			if (msg.s_header.s_type == STOP){
				if (cxt -> id == 1){
				print_balance_h( cxt -> balance_history);
				fix_h(&(cxt -> balance_history));
				print_balance_h( cxt -> balance_history);
				}
		 		return 0;
        		}
		} else {
				transfer_in(cxt, msg);	
		}
        	
        }

}

int third_stage_child(IO* cxt){
	IO context = *cxt;
	char buf[MAX_PAYLOAD_LEN];
	int status;
	Message msg;

	// Build msg
	sprintf(buf ,log_done_fmt, get_physical_time(), context.id, context.balance);
	// Send stop done other process
	send_multicast(cxt, build_msg(buf , DONE));
	// Receive done msg from other process
	for (local_id i = 1; i <= context.proc_num; i++){
		if (context.id == i)
			continue;
		//receive_any(&context, &msg);
		receive_blk(cxt, i ,&msg);
		if ( msg.s_header.s_type == TRANSFER){
			transfer_in(cxt, msg);
			i--;
		}
	}
	// Build receive all done msg
	sprintf(buf, log_received_all_done_fmt, get_physical_time(),context.id);
	// Log receive all done msg
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	// Build done msg
	sprintf(buf, log_done_fmt, get_physical_time(), context.id, context.balance);
	// Log done msg
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	return 0;
}

int child_work(IO context){
        Message msg;
	close_n_needed(context);
	
	first_stage_child(&context);
	
	second_stage_child(&context);

	third_stage_child(&context);

	return 0;
}

int first_stage_parent(IO context){
	char buf[MAX_PAYLOAD_LEN];
	int status;
	Message msg;

	// Receive started msg from other process
	for (local_id i = 1; i <= context.proc_num; i++){
		if (context.id == i)
			continue;
		receive_any(&context, &msg);
	}
	// Log recived started msg from other process
	sprintf(buf, log_received_all_started_fmt,get_physical_time(), context.id);
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	
	return 0;
}

int second_stage_parent(IO context){
	Message msg;
	
	bank_robbery(&context, context.proc_num);

	transfer(&context, 3, 1, 4);
	
	send_multicast(&context, build_msg("" , STOP));

	return 0;

}

int third_stage_parent(IO context){
	char buf[MAX_PAYLOAD_LEN];
	int status;
	Message msg;

	// Receive done msg from other process
	for (local_id i = 1; i <= context.proc_num; i++){
		if (context.id == i)
			continue;
		receive_any(&context, &msg);
	}
	// Build receive all done msg
	sprintf(buf, log_received_all_done_fmt, get_physical_time(),context.id);
	// Log receive all done msg
	if ((status = write(context.events, buf, strlen(buf))) < 0){
		return status;
	}
	
	return 0;
}

int parent_work(IO context){
	char buf[MAX_PAYLOAD_LEN];
	close_n_needed(context);

	first_stage_parent(context);
	
	second_stage_parent(context);
	
	third_stage_parent(context);

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

	//for(local_id i = 1; i <= process_number; i++)
		//context.balance[i] = atoi(argv[i + 2]);

	context.events = open(events_log, O_WRONLY | O_APPEND | O_CREAT, 0666);
	context.pipes = open(pipes_log, O_WRONLY | O_APPEND | O_CREAT, 0666);
	
	if (context.events < 0 || context.pipes < 0)
		return 5;
//	print_pipes(context);
	for (local_id i = 1; i <= process_number; i++){
		pid = fork();
		if (pid < 0) 
			return 6;
		if( pid == 0) {
			BalanceState bs;
			bs.s_balance = atoi(argv[i + 2]);
			context.id = i;
			context.balance = atoi(argv[i + 2]);
			context.balance_history.s_id = i;
			context.balance_history.s_history[0] = bs;
			return child_work(context);
		}
		pid = 0;
	}
	context.id = 0;
	return parent_work(context);
}
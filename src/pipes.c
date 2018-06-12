#include "pipes.h"

IO* init_pipelines(int process_number){
	IO* pipelines = (IO*) malloc(sizeof(IO));
	pipelines -> process_number = process_number;
	pipelines -> input = (int***) malloc (sizeof(int**) * (process_number + 1));
	for ( int i = 0; i < process_number + 1; i++){
		*(pipelines -> input + i) = malloc (sizeof(int*) * (process_number + 1));
		for ( int j = 0; j < process_number + 1; j++){
			if ( i == j || j == 0)
				continue;
			pipe(*(*(pipelines -> input + i) + j) = (int*) malloc(sizeof(int) * 2));
		}
	}
	return pipelines;
}

int send(void * self, local_id dst, const Message * msg){
	char buf[MAX_PAYLOAD_LEN];
	IO pl = *((IO*) self);
	int count =  write( *(*(*(pl.input + dst) + pl.local_id) + 1), msg, sizeof(MessageHeader) + msg -> s_header.s_payload_len);
	sprintf( buf, "Process  %i send message to %i\n", pl.local_id, dst);
	write(pl.pipes, buf, strlen(buf));
	return count;
}

int receive(void * self, local_id from, Message * msg){
	char buf[MAX_PAYLOAD_LEN];
	IO pl = *((IO*) self);
	sprintf(buf, "Process  %i wanted message from %i\n", pl.local_id, from);
	write(pl.pipes, buf, strlen(buf));
	int count = read( *(*(*(pl.input + pl.local_id) + from) + 0), msg, sizeof(Message));
	sprintf(buf, "Process  %i recive message from %i: %s", pl.local_id, from, msg->s_payload);
	write(pl.pipes, buf, strlen(buf));
	return count;
}

int send_multicast(void * self, const Message * msg){
	IO pl = *((IO*) self);
	for(local_id i = 0; i <= pl.process_number; i++) {
		if( i == pl.local_id)
			continue;
		int exit_code;
		if (exit_code = send(self, i, msg) < 0)
			return exit_code;
	}
	return 0;
}

int receive_any(void * self, Message * msg){
	IO pl = *((IO*) self);
	for(local_id i = 1; i <= pl.process_number; i++) {
		if( i == pl.local_id)
			continue;
		int exit_code;
		if (exit_code = receive(self, i, msg + i - 1) < 0){
			printf("xui");
			return exit_code;
		}
	}
	return 0;
}

Message* build_msg(const char* payload, MessageType type){
	MessageHeader msgh;
	msgh.s_magic = MESSAGE_MAGIC;
	msgh.s_payload_len = strlen(payload);
	msgh.s_type = type;
	msgh.s_local_time = time(NULL);
	Message* msg = (Message*) malloc (sizeof(Message));
	msg -> s_header = msgh;
	strcpy(msg -> s_payload, payload);
	return msg;
}
int close_self(IO* pipelines){
	for(local_id i = 1; i <= pipelines -> process_number; i++){
		if ( i == pipelines -> local_id )
			continue;
		if(close(*(*(*(pipelines -> input +  pipelines -> local_id) + i) + 1)) < 0){
			printf("xui\n");
			return 5;
		}
	}
	for(local_id i = 0; i <= pipelines -> process_number; i++){
		if ( i == pipelines -> local_id || pipelines -> local_id == 0)
			continue;
		if(close(*(*(*(pipelines -> input + i)  + pipelines -> local_id) + 0)) < 0){
			printf("xui\n");
			return 5;
		}
	}
}
void print_(IO* pipelines) {
	IO pl = *(pipelines);
	for (local_id i = 0; i <= pl.process_number; i++){
		for (local_id j = 0; j <= pl.process_number; j++){
			if (j == 0 || i == j) {
				printf("{0 0}");
				continue;
			}
			printf("{%i %i}", *(*(*(pl.input + i) + j)), *(*(*(pl.input + i) + j) + 1));
		}
		printf("\n");
	}
}
int close_all(IO* pipelines){
	IO pl = *(pipelines);
	for (local_id i = 0; i <= pl.process_number; i++){
		for (local_id j = 0; j <= pl.process_number; j++){
			if (j == 0 || i == j) 
				continue;
			if(close(*(*(*(pl.input + i) + j))) < 0){
				printf("xui input\n");
				return 6;
			}
			if(close(*(*(*(pl.input + i) + j) + 1)) < 0){
				printf("xui output\n");
				return 6;
			}
		}
	}
	if(close(pl.events) < 0){
		printf("xui event\n");
		return 6;
	}
	if(close(pl.pipes) < 0){
		printf("xui pipe\n");
		return 6;
	}
}

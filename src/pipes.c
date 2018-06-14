#include "pipes.h"

IO init_pipelines(int proc_num){
	IO context;
	for (local_id i = 0; i <= proc_num; i++){
		for (local_id j = 0; j <= proc_num; j++){
			if ( i == j || j == 0)
				continue;
			if (pipe(context.pipelines[i][j]) != 0)
				_exit(4);
		}
	}
	return context;
}

void print_pipes(IO context){
	for (local_id i = 0; i <= context.proc_num; i++){
		for (local_id j = 0; j <= context.proc_num; j++){
			if ( i == j || j == 0){
				printf("{0;0}");
				continue;
			}
			printf("{%i;%i}",context.pipelines[i][j][0],context.pipelines[i][j][1]);
				
		}
	printf("\n");
	}
}
int send(void * self, local_id dst, const Message * msg){
	IO context = *((IO*) self);
	int status;
	char buf[MAX_PAYLOAD_LEN];
	status = write(context.pipelines[dst][context.id][1], msg, sizeof(MessageHeader) + msg-> s_header.s_payload_len);
	sprintf(buf, log_send, context.id, dst, msg->s_header.s_type);
	if (status = write(context.pipes, buf, strlen(buf)) < 0){
		return status;
	}
	return status;
	
}

int receive(void * self, local_id from, Message * msg){
	IO context = *((IO*) self);
	int status;
	char buf[MAX_PAYLOAD_LEN];
	MessageHeader msgh;
	status = read(context.pipelines[context.id][from][0], &msgh, sizeof(MessageHeader));
	msg->s_header = msgh;
	if (msgh.s_payload_len != 0)
		status = read(context.pipelines[context.id][from][0], msg->s_payload, msgh.s_payload_len);
	
	sprintf(buf, log_rec, context.id, from, msg->s_header.s_type);
	if (status = write(context.pipes, buf, strlen(buf)) < 0){
		return status;
	}
	return status;
}
int receive_any(void * self, Message * msg){
	IO context = *((IO*) self);
	int status;
	while(1){
		
	}
}
int send_multicast(void * self, const Message * msg){
	IO context = *((IO*) self);
	int status;
	for (local_id i = 0; i <= context.proc_num; i++){
		if ( i == context.id)
			continue;
		status = send(self, i, msg);
	}
	return status;
}

Message* build_msg(const char* payload, MessageType type){
	MessageHeader msgh;
	msgh.s_magic = MESSAGE_MAGIC;
	msgh.s_payload_len = strlen(payload);
	msgh.s_type = type;
	msgh.s_local_time = time(NULL);
	Message* msg = (Message*) malloc (sizeof(Message));
	msg -> s_header = msgh;
	if (msgh.s_payload_len != 0)
		strcpy(msg -> s_payload, payload);
	return msg;
}

#include "pipes.h"

timestamp_t max_t(timestamp_t first, timestamp_t second){
	return first > second ? first : second;
}

IO init_pipelines(int proc_num){
	IO context;
	for (local_id i = 0; i <= proc_num; i++){
		for (local_id j = 0; j <= proc_num; j++){
			if ( i == j )//|| j == 0)
				continue;
			if (pipe(context.pipelines[i][j]) != 0)
				_exit(4);
			fcntl(context.pipelines[i][j][0], F_SETFL, O_NONBLOCK); //| O_RDWR);
			fcntl(context.pipelines[i][j][1], F_SETFL, O_NONBLOCK); //| O_RDWR);
		}
	}
	return context;
}

void print_pipes(IO context){
	for (local_id i = 0; i <= context.proc_num; i++){
		for (local_id j = 0; j <= context.proc_num; j++){
			if ( i == j ){
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
	sprintf(buf, log_send,((IO*) self) -> time , context.id, dst, msg->s_header.s_type);
	if ((status = write(context.pipes, buf, strlen(buf))) < 0){
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
	if (status == -1)
		return status;
	msg->s_header = msgh;
	if (msgh.s_payload_len > 0)
		status = read(context.pipelines[context.id][from][0], msg->s_payload, msgh.s_payload_len);
	timestamp_t time = get_lamport_time((IO*)self);
	((IO*) self) -> time = time > msg -> s_header.s_local_time + 1 ? time : msg -> s_header.s_local_time + 1;
	
////////if (context.id == 1)
////////	printf("%d\n", ((IO*) self) -> time);
	
	sprintf(buf, log_rec, ((IO*) self) -> time ,context.id, from, msg->s_header.s_type);
	if ((status = write(context.pipes, buf, strlen(buf))) < 0){
		return status;
	}
	msg->s_payload[msgh.s_payload_len] = '\0';
	
	//if (context.id == 1)
	//print_msg(*msg);
	
	return 0;
}
int receive_blk(void * self, local_id from, Message * msg){
	int status;
	int n = 0;
	while (1){
		n++;
		status = receive(self, from, msg);
		if (status == 0)
			return n;
	}
}
int receive_any(void * self, Message * msg){
	IO context = *((IO*) self);
	int status;
	int i;
	while(1){
		for ( i = 0; i <= context.proc_num; i++) {
			if ( i == context.id)
				continue;
			status = receive(self, i, msg);
			if (status == 0)
				return i;	
		}
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
int send_child(void * self, const Message * msg){
	IO context = *((IO*) self);
	int status;
	for (local_id i = 1; i <= context.proc_num; i++){
		if ( i == context.id)
			continue;
		status = send(self, i, msg);
	}
	return status;
}  

Message* build_msg(IO* cxt ,const char* payload, MessageType type){
	MessageHeader msgh;
	msgh.s_magic = MESSAGE_MAGIC;
	msgh.s_payload_len = strlen(payload);
	msgh.s_type = type;
	msgh.s_local_time = get_lamport_time(cxt);
	Message* msg = (Message*) malloc (sizeof(Message));
	msg -> s_header = msgh;
	if (msgh.s_payload_len != 0)
		strcpy(msg -> s_payload, payload);
	return msg;
}
	
Message* build_msg_h(IO* cxt, MessageType type){
	BalanceHistory bh = cxt -> balance_history;
	MessageHeader msgh;
	msgh.s_magic = MESSAGE_MAGIC;
	msgh.s_payload_len = sizeof(BalanceHistory) - sizeof(BalanceState)*(MAX_T - bh.s_history_len);
	//printf("%lu   %lu\n",sizeof(uint8_t)*2 + (bh.s_history_len + 1)*sizeof(BalanceState),sizeof(BalanceHistory) - sizeof(BalanceState)*(MAX_T - bh.s_history_len));
	msgh.s_type = type;
	msgh.s_local_time = get_lamport_time(cxt);
	Message* msg = (Message*) malloc (sizeof(Message));
	msg -> s_header = msgh;
	memcpy(msg -> s_payload, &bh , msgh.s_payload_len);
	return msg;
}
	
Message* build_transfer(IO* cxt, TransferOrder transfer_order){
	MessageHeader msgh;
	msgh.s_magic = MESSAGE_MAGIC;
	msgh.s_payload_len = sizeof(TransferOrder);
	msgh.s_type = TRANSFER;
	msgh.s_local_time = get_lamport_time(cxt);
	Message* msg = (Message*) malloc (sizeof(Message));
	msg -> s_header = msgh;
	memcpy(msg -> s_payload, &transfer_order ,sizeof(TransferOrder));
	return msg;
}

void print_msg(Message msg){
	if (msg.s_header.s_payload_len != 0)
	printf("Magic: %hu\nPayload_len: %hu\nType: %hi\nTime: %hi\nMsg: %s\n",msg.s_header.s_magic, msg.s_header.s_payload_len, msg.s_header.s_type,msg.s_header.s_local_time ,msg.s_payload);
	else
	printf("Magic: %hu\nPayload_len: %hu\nType: %hi\nTime: %hi\nMsg: %s\n\n",msg.s_header.s_magic, msg.s_header.s_payload_len, msg.s_header.s_type,msg.s_header.s_local_time ,msg.s_payload);
	return;
}


timestamp_t get_lamport_time(IO* cxt){
	cxt -> time++;
	return cxt -> time;
}



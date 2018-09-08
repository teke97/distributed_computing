#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include "common.h"
#include "ipc.h"  
#include "pa2345.h"
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include "banking.h"
#include "queue.h"



typedef struct {
	int pipelines[11][11][2];
	balance_t balance;
	int proc_num;
	int events;
	int pipes;
	local_id id;
	BalanceHistory balance_history;
	timestamp_t time;
	Queue* q;
	int mutex;
	int m;
} IO;

timestamp_t max_t(timestamp_t first, timestamp_t second);
IO init_pipelines(int proc_num);
void print_pipes(IO context);
Message* build_msg(IO* cxt, const char* payload, MessageType type);
Message* build_msg_h(IO* cxt, MessageType type);
int receive_blk(void * self, local_id from, Message * msg);
void print_msg(Message msg);
void print_balance_h(BalanceHistory bh);
Message* build_transfer(IO* cxt, TransferOrder transfer_order);
int send_child(void * self, const Message * msg);

static const char * const log_send = 
	"%d: Process %i send message to %i with type: %i\n";
static const char * const log_rec = 
	"%d: Process %i receive message from %i with type: %i\n";

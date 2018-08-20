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



typedef struct {
	int pipelines[11][11][2];
	//int balance[11];
	balance_t balance;
	int proc_num;
	int events;
	int pipes;
	local_id id;
	BalanceHistory balance_history;
} IO;

IO init_pipelines(int proc_num);
void print_pipes(IO context);
Message* build_msg(const char* payload, MessageType type);
int receive_blk(void * self, local_id from, Message * msg);
void print_msg(Message msg);

static const char * const log_send = 
	"Process %i send message to %i with type: %i\n";
static const char * const log_rec = 
	"Process %i receive message from %i with type: %i\n";

#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include "common.h"
#include "ipc.h"  
#include "pa1.h"
#include <string.h>
#include <time.h>
#include <fcntl.h>



typedef struct {
	int pipelines[11][11][2];
	int proc_num;
	int events;
	int pipes;
	local_id id;
} IO;

IO init_pipelines(int proc_num);
void print_pipes(IO context);
Message* build_msg(const char* payload, MessageType type);

static const char * const log_send = 
	"Process %i send message to %i with type: %i\n";
static const char * const log_rec = 
	"Process %i receive message from %i with type: %i\n";

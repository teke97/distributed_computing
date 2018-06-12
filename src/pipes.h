#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include "common.h"
#include "ipc.h"  
#include "pa1.h"



typedef struct {
	int pipelines[11][11][2];
	int proc_num;
	int events;
	int pipes;
	local_id id;
} IO;

IO init_pipelines(int proc_num);
void print_pipes(IO context);

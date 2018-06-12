#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include "ipc.h"
#include <string.h>
#include <time.h>
#include <fcntl.h>

typedef struct {
	int*** input;
	int process_number;
	local_id local_id;
	int events;
	int pipes;
} IO;

IO* init_pipelines(int process_number);
Message* build_msg(const char* payload, MessageType type);
int close_self(IO* pipelines);
void print_(IO* pipelines);
int close_all(IO* pipelines);


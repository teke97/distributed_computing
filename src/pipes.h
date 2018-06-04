#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
	int* input;
	int* output;
} IO;

IO* init_pipelines(int process_number);

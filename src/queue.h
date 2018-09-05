#include "ipc.h"


typedef struct Node_t {
	struct Node_t* prev;
	struct Node_t* next;
	local_id id;
	timestamp_t time;
} Node;

typedef struct {
	Node* head;
	Node* tail;
	size_t size;
} Queue;

Node* create_node(local_id id, timestamp_t time);

void add(Queue* queue, Node* node);

size_t release(Queue* queue);

Queue* init_Q();

void print_Q(Queue* q);

void insert(Queue* q, Node* n);
	

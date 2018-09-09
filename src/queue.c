#include "queue.h"
#include <malloc.h>

Node* create_node(local_id id, timestamp_t time){
	Node* node = (Node*) malloc(sizeof(Node));
	node -> time = time;
	node -> id = id;
	node -> next = NULL;
	node -> prev = NULL;
	return node;
}

Queue* init_Q(){
	Queue* q = (Queue*) malloc(sizeof(Queue));
	q -> head = NULL;
	q -> tail = NULL;
	q -> size = 0;
	return q;
}

void insert_head(Queue* q, Node* n){
	n -> next = q -> head;
	q -> head -> prev = n;
	q -> head = n;
	q -> size ++;
}
void insert_tail(Queue* q, Node* n){
	q -> tail -> next = n;
	n -> prev = q -> tail;
	q -> tail = n;
	q -> size ++;
}
void insert_between(Queue* q, Node* a, Node* b, Node* c){
	b -> prev = a; 
	b -> next = c;
	a -> next = b;
	c -> prev = b;
	q -> size ++;
	return;
}



void insert(Queue* q, Node* n){
	if (q -> head == NULL || q -> tail == NULL){
                q -> head = n;
                q -> tail = n;
                q -> size = 1;
                return;
        }
	Node* cur;
	for (cur = q ->tail; cur != NULL && cur -> time > n -> time; cur = cur -> prev);
	if ( cur != NULL && cur -> time == n -> time)
		for ( ; cur != NULL && cur -> id > n -> id && cur -> time == n -> time; cur = cur -> prev);
	
	if (cur == NULL){
		insert_head(q, n);
		return;
	}
	if (cur == q -> tail){
		insert_tail(q, n);
		return;
	}
	insert_between(q, cur, n, cur -> next);
	return;	
}

size_t release(Queue* q){
	if (q -> size == 1){
		q -> head = NULL;
		q -> tail = NULL;
		q -> size = 0;
		return q -> size;
	}
	q -> head -> next -> prev = NULL;
	q -> head = q -> head -> next;
	q -> size --;
	return q -> size;
}

void add(Queue* queue, Node* node){
	if (queue -> head == NULL || queue -> tail == NULL){
		queue -> head = node;
		queue -> tail = node;
		queue -> size = 1;
		return;
	}
	
	if (queue -> tail -> time > node -> time){
		if (node -> id == 1 && node -> time == 3)
		printf("id: %hhu time: %hu \n", node-> id ,node->time );
		Node* cur;
		for (cur = queue -> tail -> prev; cur != NULL &&  cur -> time > node -> time ; cur = cur -> prev){
//			printf("{Id: %hhu; Time: %hu}\n\t=>\n", cur -> id, cur -> time );
		}
		if (cur == NULL){
			if (node -> id == 1 && node -> time == 3)
			printf("null\n");
			node -> next = queue -> head;
			queue -> head -> prev = node;
			queue -> head = node;
			queue -> size ++;
			return;
		}
		if (cur -> time == node -> time && cur -> id > node -> id){
			if (node -> id == 1 && node -> time == 3)
			printf("equal\n");
			cur -> prev -> next = node;
        	        node -> prev = cur -> prev;
               		node -> next = cur;
                	cur -> prev = node; 
                	queue -> size++; 
               		return;
		}

		if (node -> id == 1 && node -> time == 3)
		printf("not equal\n");

		node -> next = cur -> next;
		node -> next -> prev = node;
		cur -> next = node;
		node -> prev = cur;
		queue -> size++;
		return;
	}
	if (queue -> tail -> time == node -> time && queue -> tail -> id > node -> id){
		if (node -> id == 1 && node -> time == 3)
		printf("werr 23r2  id: %hhu time: %hu \n", node-> id ,node->time );
		queue -> tail -> prev -> next = node;
		node -> prev = queue -> tail -> prev;
		node -> next = queue -> tail;
		queue -> tail -> prev = node;
		queue -> size++;
		return;
	}
	queue -> tail -> next = node;
	node -> prev = queue -> tail;
	queue -> tail = node;
	queue -> size++;
	return;
}

void print_Q(Queue* q){
	Node* cur = q -> head;
	printf("Queue size: %lu\n", q -> size);
        for (size_t i = 0; i < q -> size; i++){
        	printf("{Id: %hhu; Time: %hu}\n", cur -> id, cur -> time );
		cur = cur -> next;
		if (i != q -> size - 1)
			printf("\t=>\n");
        }
}

//int main(){
//      Queue* q1 = init_Q();
//      insert(q1, create_node(2, 2));
//      print_Q(q1);
//      printf("%d\n", release(q1));
//      print_Q(q1);
//}

#ifndef _TICKET_H_
#define _TICKET_H_

#define TICKET_SEND 1
#define TICKET_FINISH 2
#include <pthread.h>
typedef struct ticket
{
	int type;
	int server_id;
	int hop_count;

}ticket;

typedef struct node
{
	struct node *next;
	ticket *value;
}node;

typedef struct queue
{
	node *first;
	node *last;
	int size;
}queue;

ticket *create_ticket_default();
ticket *create_ticket(int type,int server_id,int hop_count);
void print_ticket(ticket *m);

node *create_node_default();
node *create_node(ticket *t);
queue *create_queue();
void enqueue(pthread_mutex_t *condition_mutex,queue *q,node *n);
node *dequeue(pthread_mutex_t *condition_mutex,queue *q);
void print_queue(queue *q);
int is_queue_empty(queue *q);

#endif

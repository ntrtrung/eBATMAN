#ifndef _PACKET_QUEUE_H_
#define _PACKET_QUEUE_H_


#include <pthread.h>
#define MAX_QUEUE 1000
typedef struct ticket
{
	int type;
	char packet[2000];
	int length;
}ticket;


typedef struct queue
{
	ticket *QUEUE;;
	int first;
	int last;
	int size;
}queue;

queue *create_queue();
int enqueue(queue *q,char *packet,int pac_length,int pac_type);
char  *dequeue(queue *q, int *type, int *length);
int is_queue_empty(queue *q);

#endif

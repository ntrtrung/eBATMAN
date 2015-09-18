#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "packet_queue.h"
#include "global.h"


queue *create_queue()
{
	printf("\n queue creation");
	queue *q;
	q = (queue*)malloc(sizeof(queue));
	if(q == NULL)
	{
		printf("\n Cannot malloc for queue - create_queue in packet_queue.c");
		exit(0);
	}
	memset(q,0,sizeof(queue));
	q->QUEUE = (ticket*)malloc(sizeof(ticket)*MAX_QUEUE);
	memset(q->QUEUE,0,sizeof(ticket)*MAX_QUEUE);
	q->size = 0;
	q->first = -1;
	q->last = -1;
	return q;
}

int enqueue(queue *q,char *packet,int pac_length,int pac_type)
{
	
	if(q->size >= MAX_QUEUE)
	{
		printf("\n queue is full in packet_queue.c");
		return -1;
	}
	if(q->first == -1)
	{
		q->first = 0;
		q->QUEUE[q->first].length = pac_length;
		//q->QUEUE[q->first].type = pac_type;
		memcpy(q->QUEUE[q->first].packet,packet,pac_length);
		q->last = (q->first + 1)%MAX_QUEUE;
		q->size ++;
		//printf("\n enqueue in first with first %d and last:%d",q->first,q->last);
		return 1;		
	}
	if(q->QUEUE[q->last].length == 0)
	{
		q->QUEUE[q->last].length = pac_length;
		//q->QUEUE[q->last].type = pac_type;
		memcpy(q->QUEUE[q->last].packet,packet,pac_length);
		q->last = (q->last + 1) %MAX_QUEUE;
		q->size ++;
		//printf("\n enqueue in last with first %d and last:%d",q->first,q->last);
		return 1;
	}
	else
	  printf("\n queue does not have any empty space packet_queue.c");
	return -1;
}
char  *dequeue(queue *q, int *type, int *length)
{
	
	int last;
	if(q->size <= 0)
		return NULL;
	
	if(q->QUEUE[q->first].length > 0)
	{
		*length = q->QUEUE[q->first].length;
		q->QUEUE[q->first].length = 0;
		q->size --;
		last = q->first;
		q->first = (last + 1)%MAX_QUEUE;
		//printf("\n dequeue with first %d and last:%d",q->first,q->last);
		return (char*)q->QUEUE[last].packet;
	}
	else
		printf("n Error in dequeue , first node is empty");
	return NULL;
		
}

int is_queue_empty(queue *q)
{
	if(q->size > 0)
		return 1;
	return 0;
}



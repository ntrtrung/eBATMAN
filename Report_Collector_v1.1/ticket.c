#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ticket.h"

void print_queue(queue *q)
{
	if(q == NULL)
		return;
	printf("\nqueue size :%d\n",q->size);
	node *t;
	t = q->first->next;
	while(t!=NULL)
	{
		print_ticket(t->value);
		t=t->next;
	}
}
ticket *create_ticket_default()
{
	ticket *m;
	m =(ticket*)malloc(sizeof(ticket));
	m->type = TICKET_SEND;
	m->server_id =0;
	m->hop_count =0;
	return m;
}
ticket *create_ticket(int type,int server_id,int hop_count)
{
	ticket *m;
	m =(ticket*)malloc(sizeof(ticket));
	m->type = type;
	m->server_id = server_id;
	m->hop_count = hop_count;
	return m;
}

void print_ticket(ticket *m)
{
	if(m == NULL)
		return;
	printf("ticket type :%d - server id :%d - hop count:%d \n",m->type,m->server_id,m->hop_count);
}

node *create_node_default()
{
	node *m ;
	m= (node*)malloc (sizeof(node));
	m->next = NULL;
	m->value = create_ticket_default();
	return m;
}

node *create_node(ticket *t)
{
	node *m ;
	m= (node*)malloc (sizeof(node));
	m->next = NULL;
	m->value = create_ticket(t->type,t->server_id,t->hop_count);
	return m;
}

queue *create_queue()
{
	queue *m;
	m=(queue*)malloc(sizeof(queue));
	m->first = create_node_default();
	m->last = m->first;
	m->size=0;
}

void enqueue(pthread_mutex_t *condition_mutex,queue *q,node *n)
{
    pthread_mutex_lock( condition_mutex );	
	q->last->next = create_node(create_ticket(n->value->type,n->value->server_id,n->value->hop_count));
	q->last= q->last->next;
	q->last->next = NULL;
	q->size ++;
       pthread_mutex_unlock(condition_mutex );
}
node * dequeue(pthread_mutex_t *condition_mutex,queue *q)
{
	pthread_mutex_lock( condition_mutex );	
	if(q->size  <= 0)
		return NULL;
	if(q == NULL) 
	{
		printf("\nqueue is invalid");
		return NULL;
	}
	ticket *value = q->first->next->value;
	node *temp = create_node(value);
       if (q->size ==1 )
	{
		free(q->first->next);
		q->first->next=NULL;
		q->last=q->first;
		q->size --;
		pthread_mutex_unlock(condition_mutex );
		return temp;
	}
	
	node *del;
	del = q->first->next;
	q->first->next = q->first->next->next;
	free(del);
	q->size --;
	pthread_mutex_unlock(condition_mutex );

	return temp;      
}
int is_queue_empty(queue *q)
{
	if(q->size==0)
		return 1;
	return 0;
}


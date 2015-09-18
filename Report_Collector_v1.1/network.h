#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "ticket.h"
#include <netinet/in.h>
#include <pthread.h>

#define TRUE             1
#define FALSE            0

int receive_ticket(ticket *t,int sock_fd);
int SendTicketToServer(pthread_mutex_t *mutex,ticket *data,int *sockfd);
int ConnectToServer(char *ip_address,int port);
void accept_clients(int listen_sd,struct fd_set *master_set,int max_sd,int *end_server);
int init_server(char *ip_address,int port);
char *get_ip_client(struct sockaddr_in *client);

#endif

	



#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include "ticket.h"
#include "network.h"

pthread_mutex_t send_ticket_mutex = PTHREAD_MUTEX_INITIALIZER;

int main()
{
	char *ip_server = "10.0.0.1";
	int port = 9950;

	int server_sd;
	server_sd =  ConnectToServer(ip_server,port);
	ticket *t = create_ticket(1,1,1);
	print_ticket(t);
	if(SendTicketToServer(&send_ticket_mutex,t,&server_sd)<0)
	{
		printf("\ncannot send ticket");
	}	
	
	close(server_sd);
}

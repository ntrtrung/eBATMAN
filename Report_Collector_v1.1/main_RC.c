#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <net/if.h>
#include "ticket.h"
#include "id_ip.h"
#include "network.h"
#include "upload_data.h"
#include "read_config.h"


#define SERVER_PORT  9950
#define TRUE             1
#define FALSE            0
#define interface_name  "fxp0"

pthread_mutex_t is_loading_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t forwarding_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t uploading_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond_for_upload  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t condition_mutex_for_upload = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond_for_forward  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t condition_mutex_for_forward = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t send_ticket_mutex = PTHREAD_MUTEX_INITIALIZER;


int is_uploading;
queue *uploading_queue;
queue *forwarding_queue;
file_list *ST;

int is_shutdown;
int client_sd;
int server_sd;
char *setting_file;
id_ip *ID_IP;
int connected_client_id;
int server_id;
int is_connected;
char * directory;
char *ip_server;
char *list_data_server;
char *config_file="/root/Read_Config/rc_config.xml";
rc_configuration rc;


// this function reads ip adress of em0 card in the server
int get_id_server()
{
	 int fd;
	 struct ifreq ifr;
	 fd = socket(AF_INET, SOCK_DGRAM, 0);
	 ifr.ifr_addr.sa_family = AF_INET;
	 strncpy(ifr.ifr_name,rc.rc_interface, IFNAMSIZ-1);
	 ioctl(fd, SIOCGIFADDR, &ifr);
	 close(fd);
	 
	 ip_server = NULL;
	 ip_server = (char*)inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
	 printf("\nIP of server: %s",ip_server);

	 id_ip *temp;
	 temp = ID_IP;
	 while(temp!= NULL)
	 {
		if ( strcmp(temp->ip_address,ip_server) == 0)
		  return temp->id;
		   temp= temp->next;
	 }
	 printf("\n Cannot find id of this server, please check your setting file");
	 exit(0);
	 return -1;
}


void init_global_variable()
{
	printf("\nInit global variable");
	uploading_queue = create_queue();
	forwarding_queue = create_queue();
	
	is_uploading = 0;
	is_shutdown=0;
	connected_client_id = -1;
       is_connected = 0;
	 if(read_config_file(&rc,config_file)==-1)
	{
		printf("\nRead configureation file error");
		exit(0);
	}
	directory = rc.log_file_path;
	list_data_server = rc.data_server_list;
	setting_file = rc.setting_file;
}

// catch signal broken pipe when server is collapsed.
void signal_brokenpipe_client(int sig)
{
	switch(sig) {
	case SIGPIPE:
		printf("\nConnect is broken");
		is_connected = 0;
		break;
	default:
		break;
	}
}

// server receives a ticket. It will put this ticket into forwarding queue, if the ticket has hop count bigger than 1, or server is uploading. 
// the ticket will put into uploading queue, if server is not under uploading.

void ticket_forwarding(ticket *m)
{
    pthread_mutex_lock(&is_loading_mutex);
	if(is_uploading == 1)
	{
		pthread_mutex_unlock(&is_loading_mutex);	
		printf("\nUnder uploading...now");		
		printf("\nServer is uploading, pass ticket");
		// in the case if client cannot send the ticket to server,ticket will be enqueued.
		node *n = create_node(m);
		enqueue(&forwarding_queue_mutex,forwarding_queue,n);                   
		pthread_cond_signal( &condition_cond_for_forward );
		free(n);
		return;
	}
	else
	   pthread_mutex_unlock(&is_loading_mutex);	
	   
	node *n = create_node(m);
	enqueue(&uploading_queue_mutex,uploading_queue,n);
	pthread_cond_signal( &condition_cond_for_upload );
	print_queue(uploading_queue);
	if(m->hop_count > 1)
	{
		n->value->hop_count--;
		printf("\nHop count is bigger than 1, pass ticket");	
		enqueue(&forwarding_queue_mutex,forwarding_queue,n);  
		pthread_cond_signal( &condition_cond_for_forward );
	}
	free(n);
}

// receive ticket from server.
void receive_data(int i,int *close_conn)
{
  
   int last_ticket =-1;
   int rc,len;
    do
      {
		  ticket *buffer = create_ticket_default();
          rc = recv(i, buffer, sizeof(ticket), 0);
          if (rc < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  recv() failed");
                        *close_conn = TRUE;
                     }
                     break;
                  }

          if (rc == 0)
                  {
                     printf("  Connection closed\n");
                     *close_conn = TRUE;
                     break;
                  }

	   print_ticket(buffer);
          len = rc;
       printf("  %d bytes received\n", len);
	   // if this is the first node of cirle , then all tickets will be dropped when they roll back.
	   if(buffer->type <= last_ticket && server_id == 0)
	   {
			continue;
	   }
	   last_ticket = buffer->type;
	   ticket_forwarding(buffer);
	   free(buffer);
      } while (TRUE);

}

//close a connection
void close_connection(int socket_sd,struct fd_set *master_set,int *max_sd)
{
      close(socket_sd);
      FD_CLR(socket_sd, master_set);
      if (socket_sd == *max_sd)
      {
           while (FD_ISSET(*max_sd,master_set) == FALSE)
                   *max_sd -= 1;
      }
}

//close all connections when server shutdowns.
void close_all_connection(struct fd_set *master_set,int *max_sd,int exception)
{
	printf("\n Redirect client");
	int i;
	for(i=0;i < *max_sd;i++)
	{
		if(i != exception){
			close_connection(i,master_set,max_sd);
		}
	}
	connected_client_id = exception;
}

int reconnect_to_neighbor(int id,id_ip *IP)
{
    if(is_connected >= 1)
	   return -1;

       int client_sd;
	printf("\nReconnect to neighbor");
	int id_neighbor ;
	id_neighbor = id;
	while(1)
	{
		id_neighbor = look_up_next_id(id_neighbor,IP);
		if(id_neighbor == id)
		{
		    is_connected = 0;
			return -1;
		}
		char *ip_next = look_up_ip(IP,id_neighbor);
		if(ip_next == NULL)
		{
			printf("\n Cannot look up a neighbor");
			exit(1);
		}
		printf("\nconnect to next id:%d with ip:%s",id_neighbor,ip_next);
         client_sd = ConnectToServer(ip_next,SERVER_PORT);
        if(client_sd <= 0 )
		{
			printf("\nCannot connect to server:%s\n",ip_next);
			sleep(1);
			continue;
		}
		break;	
	}
    is_connected = 1;
	if(id_neighbor < id)
		is_connected = 2;
   return client_sd;
}

//ticket forwader look at the ticket forwarding queue, then it connects to next server and send a ticket in the queue.
// in the case of sending failure, the ticket will be reenqueued.

void *ticket_forwarder_thread()
{
	printf("\nTicket Forwarding Thread is running");

	while(1)
	{
		if(is_shutdown == 1 )
			break;
	
		pthread_mutex_lock(&condition_mutex_for_forward);
		while(is_queue_empty(forwarding_queue)==1)
		{
			if(is_connected >= 1)
			{
				is_connected = 0;
				close(client_sd);
			}
			pthread_cond_wait(&condition_cond_for_forward, &condition_mutex_for_forward);
		}
		pthread_mutex_lock(&condition_mutex_for_forward);
	   	if(is_connected == 0)
		{
				client_sd = reconnect_to_neighbor(server_id,ID_IP);
				if(client_sd == -1)
					continue;
		}
	    
		
		node *n = create_node_default();    
		n=dequeue(&forwarding_queue_mutex,forwarding_queue);
        if( n == NULL)
		{
				continue;
        }             
		
		//if next neighbor's id is lower than node's id , ticket will be discarded
		if(is_connected == 2)
		{
			free(n);
			continue;
		}
		ticket *t = n->value;
		print_ticket(t);
		if(SendTicketToServer(&send_ticket_mutex,t,&client_sd) <0 )	
		{
			//if ticket cannot be sent , it will be reenqueued .
			enqueue(&uploading_queue_mutex,uploading_queue,n);                   
		}
		free(n);
	}	

}

// server receives connections from nieghbors.
void *connection_manager_thread()
{
   printf("\nconnection manager thread is running\n");
   int    i, rc;
   int    listen_sd, max_sd, new_sd;
   int    desc_ready, end_server = FALSE;
   int    close_conn;
   ticket   *buffer = create_ticket_default();
   struct timeval       timeout;
   struct fd_set        master_set, working_set;

   listen_sd = server_sd;
   FD_ZERO(&master_set);
   max_sd = listen_sd;
   FD_SET(listen_sd, &master_set);

   timeout.tv_sec  = 3 * 60;
   timeout.tv_usec = 0;
   struct sockaddr_in client_sock;
   do
   {
      memcpy(&working_set, &master_set, sizeof(master_set));

      printf("\nWaiting on select()...\n");
      rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);

      if (rc < 0)
      {
         perror("  select() failed");
         break;
      }

      if (rc == 0)
      {
         printf("  select() timed out.  Repeat again.\n");
         continue;
      }
      
      desc_ready = rc;
      for (i=0; i <= max_sd  &&  desc_ready > 0; ++i)
      {
         if (FD_ISSET(i, &working_set))
         {
            desc_ready -= 1;

            if (i == listen_sd)
            {
               printf("  Listening socket is readable\n");
               accept_client(listen_sd,&end_server,&max_sd,&master_set,&client_sock);
				int temp_id;
				temp_id = look_up_id(ID_IP,get_ip_client(&client_sock));
				printf("\nconnnecting_client_id :%d",temp_id);		 
		
			}
            else
            {
               printf("  Descriptor %d is readable\n", i);
               close_conn = FALSE;
               receive_data(i,&close_conn);

               if (close_conn)
               {
                 close_connection(i,&master_set,&max_sd);
               }
            } 
         } 
      } 

   } while (end_server == FALSE);

   for (i=0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }

}

void Upload_Data(id_ip *data_server_list,int data_server_id, char *server_id_string)
{
	char *data_server_ip;
	data_server_ip = look_up_ip(data_server_list,data_server_id);
	if(data_server_ip == NULL)
	{
		printf("\n ERROR : Cannot find ip of data server with ip:%d",data_server_id);
		return;
	}
	if( upload_data(ST,data_server_ip,directory,server_id_string) == -1)
		printf("Upload data error");
	else
        printf("\nData have been uploaded successfuly \n");
}

void *data_uploading_thread()
{
	printf("\nData Uploading Thread is running");
	//----------------READ DATA SERVER LIST FILE----------------//
	printf("\nRead data server list:");
	int numberofnode;
	id_ip *data_server_list;
	data_server_list =  read_setting_file(list_data_server,&numberofnode);
	if(data_server_list == NULL)
	{
		printf("\nCannot read database server list file");
		exit(1);
	}
	//-------------------upload file to server----------------------//
	while(1)
	{
		if(is_shutdown == 1 )
			break;
		pthread_mutex_lock(&condition_mutex_for_upload);
		while(is_queue_empty(uploading_queue)==1)
		{
			pthread_cond_wait(&condition_cond_for_upload, &condition_mutex_for_upload);
		}
		pthread_mutex_lock(&condition_mutex_for_upload);
	
		pthread_mutex_lock(&is_loading_mutex);
		is_uploading =1;
		pthread_mutex_unlock(&is_loading_mutex);

		node *n = create_node_default();
              
		n=dequeue(&uploading_queue_mutex,uploading_queue);
        if( n == NULL)
		{
				printf("\nQueue is empty\n");
				pthread_mutex_lock(&is_loading_mutex);
				is_uploading = 0;
				pthread_mutex_unlock(&is_loading_mutex);
				continue;
        }             
		ticket *t = n->value;
		print_ticket(t);
		Upload_Data(data_server_list,t->server_id,itoa(server_id));
		t->hop_count =1;
		printf("\nUpload finish, send ticket to neighbor\n");		
		print_ticket(t);
		//pass ticket to Ticket Forwarder
		enqueue(&forwarding_queue_mutex,forwarding_queue,n);                   
		pthread_cond_signal( &condition_cond_for_forward );
		pthread_mutex_lock(&is_loading_mutex);
		is_uploading = 0;
		pthread_mutex_unlock(&is_loading_mutex);
		free(n);
	}

}


void signal_handler_main(int sig)
{
	switch(sig) 
	{
		case SIGHUP:
			printf("\nhangup signal catched\n");
			break;
		case SIGTERM:
		case SIGKILL:
		case SIGINT:
			printf("\nterminate signal catched\n");
			is_shutdown =1;
			sleep(1);
			exit(0);
			break;
	}
}
int init_program()
{
	init_global_variable();
    
// read setting file to get id and ip mapping list.
	int numberofnode;
	ID_IP =  read_setting_file(setting_file,&numberofnode);
	if(ID_IP == NULL)
	{
		printf("\nCannot read setting file");
		exit(1);
	}

// init server
       server_id = get_id_server();
	char *ip_server = look_up_ip(ID_IP,server_id);
	printf("\nInit server :%s",ip_server);
	server_sd = init_server(ip_server,SERVER_PORT);  

//connect to neighbor
//	client_sd = reconnect_to_neighbor(server_id,ID_IP);


	ST = init_list_of_file(directory);
	return 1;
}

main (int argc, char *argv[])
{
  
  init_program();  
  
  signal(SIGTERM,signal_handler_main);
  signal(SIGKILL,signal_handler_main);
  signal(SIGINT,signal_handler_main);
  signal(SIGPIPE,signal_brokenpipe_client);
  pthread_t t_server, t_upload,t_forward;
  pthread_create(&t_upload, NULL, &data_uploading_thread, NULL);
  pthread_create(&t_forward, NULL, &ticket_forwarder_thread, NULL);
  pthread_create(&t_server, NULL, &connection_manager_thread, NULL);
  pthread_join(t_upload, NULL);
  pthread_join(t_forward, NULL);
  pthread_join(t_server, NULL);
 

  exit(0);
}

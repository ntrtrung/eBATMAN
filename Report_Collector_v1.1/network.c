#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pthread.h>

#include "ticket.h"
#include "network.h"


int receive_ticket(ticket *t,int sock_fd)
{
	if(sock_fd ==0)
	   return -1;
	return recv(sock_fd,t,sizeof(ticket),0);	
}

int SendTicketToServer(pthread_mutex_t *mutex,ticket *data,int *sockfd)
{
	
	int len;
	if(data == NULL || *sockfd < 1)
	{	
		fprintf(stderr,"Cannot send data to server");
		return -1;
	}
	pthread_mutex_lock(mutex);
	len = send(*sockfd, data,sizeof(ticket), 0);
	pthread_mutex_unlock(mutex);
	if(len == -1)
	{
		perror("  send() failed");
		if (errno == ENOTCONN || errno == EPIPE)
              {
                    close(*sockfd);
		      *sockfd = -1;
		      return -1;		                    
              }
              
	}
	if(len != sizeof(ticket))
   	 {
		perror("client - send() error");
		//close(sockfd);
		return -1;
   	 }
   	 else
		printf("client - send() is OK.\n");
	return len;
}


int ConnectToServer(char *ip_address,int port)
{
  int res, valopt; 
  struct sockaddr_in addr; 
  long arg; 
  fd_set myset; 
  struct timeval tv; 
  socklen_t lon; 
  int soc;

  int optval;
  socklen_t optlen = sizeof(optval);


  // Create socket 
  soc = socket(AF_INET, SOCK_STREAM, 0); 
  
  /* Set the option active */
   optval = 1;
   optlen = sizeof(optval);
   if(setsockopt(soc, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
      perror("setsockopt()");
      close(soc);
      exit(EXIT_FAILURE);
   }

  // Set non-blocking 
  arg = fcntl(soc, F_GETFL, NULL); 
  arg |= O_NONBLOCK; 
  fcntl(soc, F_SETFL, arg); 

  // Trying to connect with timeout 
  addr.sin_family = AF_INET; 
  addr.sin_port = htons(port); 
  addr.sin_addr.s_addr = inet_addr(ip_address); 
  res = connect(soc, (struct sockaddr *)&addr, sizeof(addr)); 

  if (res < 0) { 
     if (errno == EINPROGRESS) { 
        tv.tv_sec = 5; 
        tv.tv_usec = 0; 
        FD_ZERO(&myset); 
        FD_SET(soc, &myset); 
        if (select(soc+1, NULL, &myset, NULL, &tv) > 0) { 
           lon = sizeof(int); 
           getsockopt(soc, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon); 
           if (valopt) { 
              fprintf(stderr, "Error in connection() %d - %s\n", valopt, strerror(valopt)); 
              return -1; 
           } 
        } 
        else { 
           fprintf(stderr, "Timeout or error() %d - %s\n", valopt, strerror(valopt)); 
           return -1; 
        } 
     } 
     else { 
        fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
        return -1; 
     } 
  } 
  // Set to blocking mode again... 
  arg = fcntl(soc, F_GETFL, NULL); 
  arg &= (~O_NONBLOCK); 
  fcntl(soc, F_SETFL, arg); 
  return soc;
}
void print_connect_client(struct sockaddr_in *client)
{

       char *ip = (char *)malloc(20);
	ip = (char*)inet_ntoa(client->sin_addr);
	printf("\nThere is a client who connects to server :%s",ip);
}
char *get_ip_client(struct sockaddr_in *client)
{
	return (char*)inet_ntoa(client->sin_addr);
}
void accept_client(int listen_sd,int *end_server,int *max_sd, struct fd_set *master_set,struct sockaddr_in *client_sock)
{
	int new_sd;
       struct sockaddr_in sa;
       socklen_t len;
	int i;
	do
               {
                  //new_sd = accept(listen_sd,&client,&clientlen);
		    new_sd  = accept(listen_sd,(struct sockaddr*)client_sock,&len);
                  if (new_sd < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  accept() failed");
                        *end_server = TRUE;
                     }
                     break;
                  }

                  printf("  New incoming connection - %d\n", new_sd);
                  FD_SET(new_sd,master_set);
                  if (new_sd > *max_sd)
                     *max_sd = new_sd;
                  print_connect_client(client_sock);

               } while (new_sd != -1);

}


//non block server
int init_server(char *ip_address,int port)
{
   int listen_sd;
   int rc,on=1;
   struct sockaddr_in   addr;

   listen_sd = socket(AF_INET, SOCK_STREAM, 0);
   if (listen_sd < 0)
   {
      perror("socket() failed");
      exit(-1);
   }

   rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on));
   if (rc < 0)
   {
      perror("setsockopt() failed");
      close(listen_sd);
      exit(-1);
   }

   rc = ioctl(listen_sd, FIONBIO, (char *)&on);
   if (rc < 0)
   {
      perror("ioctl() failed");
      close(listen_sd);
      exit(-1);
   }

   memset(&addr, 0, sizeof(addr));
   addr.sin_family      = AF_INET;
   addr.sin_addr.s_addr = inet_addr(ip_address); 
   addr.sin_port        = htons(port);
   rc = bind(listen_sd,(struct sockaddr *)&addr, sizeof(addr));
   if (rc < 0)
   {
      perror("bind() failed");
      close(listen_sd);
      exit(-1);
   }

   rc = listen(listen_sd,3);
   if (rc < 0)
   {
      perror("listen() failed");
      close(listen_sd);
      exit(-1);
   }
   return listen_sd;
	
}

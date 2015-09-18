#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#define DO_ACTION_SERVER_PORT  9911

int ConnectToServer(int port)
{
	int len, rc;
    int sockfd;
    struct sockaddr_in addr;
	/* Create an AF_INET stream socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
		perror("client - socket() error");
		return -1;
    }
    else
		printf("client - socket() is OK.\n");

    /* Initialize the socket address structure */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    memset(&(addr.sin_zero), 0, 8);
    /* Connect to the server */
    rc = connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if(rc < 0)
    {
		perror("client - connect() error");
		close(sockfd);
		return -1;
    }
    else
    {
		printf("client - connect() is OK.\n");
		printf("connect() completed successfully.\n");
		printf("Connection with loopback using port %d established!\n", port);

    }
	return sockfd;
}

int SendDataToServer(float *data,int sockfd)
{
	int len;
	if(*data == -1 || sockfd < 1)
	{	
		printf("\nCannot send data to server");
		return -1;
	}
	len = send(sockfd, data,sizeof(float), 0);
	if(len != sizeof(float))
    {
		perror("client - send() error");
		//close(sockfd);
		return -1;
    }
    else
		printf("client - send() is OK.\n");
	return len;
}
int main()
{
  int server_fd_do_action;
  int server_do_action_connected;
  server_do_action_connected = 0;
  server_fd_do_action = ConnectToServer(DO_ACTION_SERVER_PORT);
  if(server_fd_do_action >0)
	server_do_action_connected = 1;
  else
  {
	printf("\nCannot connecto to DO_ACTION SERVER\n");
	return 0;
  }

  float this_time =0;
  while(this_time <40)
  {
  	if(SendDataToServer(&this_time,server_fd_do_action) == -1)
	{
		printf("\nsend error");
		break;
	}
	this_time+= 0.5;
	sleep(1);
  }



	return 0;
}

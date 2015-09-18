#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>

#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/if_ether.h> 
#include <net/ethernet.h>
#include <net/if.h> 
#include <net/if_dl.h> 
#include <net/if_types.h>


#include "network.h"

void die(char *s)
{
	perror(s);
	exit(1);
}
int init_udp_server(char *ip, int server_port)
{ 
	int s;
       struct sockaddr_in si_me;
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
          die("socket");
		  exit(1);
    }
    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(server_port);
	if (inet_aton(ip,&(si_me.sin_addr))==0) 
	{
          fprintf(stderr, "inet_aton() failed\n");
      	  exit(1);
    }
    
    if (bind(s,(struct sockaddr *)&si_me, sizeof(si_me))==-1)
     {
          die("bind");
		  exit(1);
    }
	return s;
}

int init_udp_client()
{
	int s;
	const int on=1;
       if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
          die("socket");
		  return -1;
	}
	if ( setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) != 0 )
		 die("set broadcast failed");

	return s;
}

int send_packet_to(int socket,char *data,int data_size,long server_ip,int server_port)
{
	struct sockaddr_in si_other;
	if(socket <= 0)
	{
		die("socket die");
		return -1;
	}
	 int slen=sizeof(struct sockaddr_in);
	 memset((char *)&si_other, 0, slen);
	 si_other.sin_family = AF_INET;
     si_other.sin_port = htons(server_port);
     si_other.sin_addr.s_addr = server_ip;	 
	return sendto(socket, data, data_size, 0,(struct sockaddr *)&si_other,slen);	
}
//non-block listen
int receive_packet_from(int sd,char *packet,struct in_addr *server)
{
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);	
	int number_byte;
	number_byte = recvfrom(sd,packet,MAX_PACKET_SIZE,0, (struct sockaddr*)&addr, &len);
	if(number_byte > 0 )
	{
			return number_byte;
	}
	else
			perror("recvfrom");
	return -1;

}

/* Run with a number of incoming connection as argument */
int InitTCPServer(int port)
{
	int rc;
	int listen_sd;
	struct sockaddr_in addr; 
    
	/* Create an AF_INET stream socket to receive */

    /* incoming connections on */
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_sd < 0)
    {
		perror("Iserver - socket() error");
		return -1;
    }
    else
		printf("Iserver - socket() is OK\n");
    

    printf("Binding the socket...\n");
    /* Bind the socket */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    if(rc < 0)
    {
		perror("Iserver - bind() error");
		close(listen_sd);
		return -1;
    }
    else
		printf("Iserver - bind() is OK\n");

    /* Set the listen backlog */
    rc = listen(listen_sd, 5);
    if(rc < 0)
    {
		perror("Iserver - listen() error");
		close(listen_sd);
		return -1;
    }
    else
	printf("\nIserver - listen() is OK\n");
    /* Inform the user that the server is ready */
    printf("\nThe Iserver is ready!\n");
	return listen_sd;
}
int AcceptTCPConnection(int listen_sd)
{
	int accept_sd;
     /* Wait for an incoming connection */
	printf(" waiting on accept()\n");
	accept_sd = accept(listen_sd, NULL, NULL);
	if(accept_sd < 0)
	{
		perror("Iserver - accept() error");
		close(listen_sd);
		return -1;
	}
	else
		printf("\naccept() is OK and completed successfully!\n");
	return accept_sd;
}

int init_tun(char* tap_name,char *ip, char *netmask)
{
	int opened;
	int fd;
	char tunname[50];
	sprintf (tunname,"/dev/%s",tap_name);
	if ((fd = open (tunname, O_RDWR)) > 0)
	{
			printf("\nopen tap device %s",tap_name);
			ioctl(fd, TUNSETNOCSUM, 1); 
			opened = 1;
	}
	if (opened == 0)
	{
		printf("\nCannot allocate TUN/TAP dev dynamically");
		exit(0);
	}
	char command[50];
	int tun_mtu = 1500;
	char *IFCONFIG_PATH ="/sbin/ifconfig";
	sprintf (command,"%s %s %s netmask %s mtu %d up",
				  IFCONFIG_PATH,
				  tap_name,
				  ip,
				  netmask,
				  tun_mtu
				  );
	printf("command :%s",command);
	system(command);

	return fd;
}


int get_ip_of_interface(char *dev)
{
        int fd;
	 struct ifreq ifr;
	 fd = socket(AF_INET, SOCK_DGRAM, 0);
	 ifr.ifr_addr.sa_family = AF_INET;
	 strncpy(ifr.ifr_name,dev, IFNAMSIZ-1);
	 ioctl(fd, SIOCGIFADDR, &ifr);
	 close(fd); 
	 return((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
}



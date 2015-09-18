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
char mac_broad[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
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
	int s;
       struct sockaddr_in si_me;
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
          die("socket");
		  exit(1);
    }
    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	   
    if (bind(s,(struct sockaddr *)&si_me, sizeof(si_me))==-1)
     {
          die("bind");
		  exit(1);
    }
	return s;
}
int AcceptTCPConnection(int listen_sd)
{
	int accept_sd;
     /* Wait for an incoming connection */
	printf(" waiting on accept()\n");
	accept_sd = accept(listen_sd, NULL, NULL);
	if(accept_sd < 0)
	{
		perror("TCP server - accept() error");
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
	int tun_mtu = 1450;
	char *IFCONFIG_PATH ="/sbin/ifconfig";
	sprintf (command,"\n%s %s %s netmask %s mtu %d up",
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
int get_broadcast_ip_of_interface(char *dev)
{
        int fd;
	 struct ifreq ifr;
	 fd = socket(AF_INET, SOCK_DGRAM, 0);
	 ifr.ifr_addr.sa_family = AF_INET;
	 strncpy(ifr.ifr_name,dev, IFNAMSIZ-1);
	 ioctl(fd, SIOCGIFBRDADDR, &ifr);
	 close(fd); 
	 return((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr.s_addr;
}

int is_ip_packet(char *packet)
{
	if(packet == NULL)
		return -1;
	struct ether_header *eptr; 
       u_short ether_type;
	eptr = (struct ether_header *) packet;
       ether_type = ntohs(eptr->ether_type);
	if (ether_type == ETHERTYPE_IP)
       {
		return 1;
	}
	return 0;
}

unsigned long get_ip_dest(char *packet)
{
	u_char *payload;
	payload= (u_char*)(packet + sizeof(struct ether_header));
	struct my_ip *ip_header;
	ip_header = (struct my_ip*)payload;
	return ip_header->ip_dst.s_addr;
}
u_char *get_mac_dest(char *packet)
{
	struct ether_header* header = (struct ether_header *)packet;
	return header->ether_dhost;
}
int is_mac_equal(char *mac1, char *mac2)
{
	//check broadcast address
	if( memcmp(mac1,mac_broad,6) == 0)
	   return 1;
	//check 2 mac address is equal
	if(mac1[5] != mac2[5])
		return 0;
	if(mac1[4] != mac2[4])
		return 0;
	if(mac1[3] != mac2[3])
		return 0;
	if(mac1[2] != mac2[2])
		return 0;
	if(mac1[1] != mac2[1])
		return 0;
	if(mac1[0] != mac2[0])
		return 0;
	return 1;	
}
char *get_mac_of(char *device)
{
	struct ifconf ifc; 
	struct ifreq *ifr; 
	struct sockaddr_dl *sdl; 
	char buf[4096], *p; 
	int s; 

	s = socket(AF_INET, SOCK_DGRAM, 0); 
	if (s < 0) 
		err(-1, "socket"); 
	ifc.ifc_len = sizeof buf; 
	ifc.ifc_buf = buf; 
	if (ioctl(s, SIOCGIFCONF, &ifc) < 0) 
		err(-1, "ioctl"); 
	close(s); 
	for (p = buf; p < buf + ifc.ifc_len; ) 
	{
		 ifr = (struct ifreq *)p; 
		 if (ifr->ifr_addr.sa_family == AF_LINK && strcmp(ifr->ifr_name,device)==0 ) 
		 {
			 sdl = (struct sockaddr_dl *)&ifr->ifr_addr; 
			 if (sdl->sdl_type == IFT_ETHER) 
			{ 
				return LLADDR(sdl); 	
			} 
		}
		p += _SIZEOF_ADDR_IFREQ(*ifr); 
	}
	return NULL;
}


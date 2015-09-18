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



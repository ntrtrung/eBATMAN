#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "routing.h"
#include "id_ip.h"

void parse_ker_rt_to_realtime_rt(kernel_routing_table *RT,int *realtime_rt,int max_realtime_rt,id_ip *ID_IP)
{
	int i;
	char *destination = (char*) malloc(sizeof(char)*20);	
	char *gate_way = (char*) malloc(sizeof(char)*20);
	
	memset(realtime_rt,-1,max_realtime_rt);
	int id_des;
	int id_gateway;
	struct in_addr dest;
	struct in_addr gateway;

	for(i =0;i<RT->number_entry;i++)
	{
		dest.s_addr = RT->entry[i].destination;
		destination = (char*)inet_ntoa(dest);
		id_des = look_up_id(ID_IP,destination);
		//printf("\ndes:%s with id %d",destination,id_des);
		gateway.s_addr = RT->entry[i].gateway;
		gate_way = (char*)inet_ntoa(gateway);
		id_gateway = look_up_id(ID_IP,gate_way);	
		//printf("\tgateway:%s with id %d",gate_way,id_gateway);
		if(id_des == -1)
			continue;
		if(id_gateway == -1)
		{
			realtime_rt[id_des] = -2;
		}
		if(id_des > -1 && id_gateway > -1)
		{
			realtime_rt[id_des] = id_gateway;
		}
		
	}
}

void reset_kernel_routing_table(kernel_routing_table *RT)
{
	memset(RT,0,sizeof(kernel_routing_table));
}
void reset_realtime_routing_table(int *RT, int number_of_node)
{
	memset(RT,-1,sizeof(int)*number_of_node);
}
int compare_realtime_routing_table(int *realtime_rt1,int *realtime_rt2,int max_realtime_rt)
{
	return memcmp(realtime_rt1,realtime_rt2,max_realtime_rt);
}
void copy_realtime_routing_table(int *realtime_rtd,int *realtime_rts,int max_realtime_rt)
{
	memcpy(realtime_rtd,realtime_rts,max_realtime_rt);
}

void print_realtime_routing(int *real,int number_of_node)
{
	printf("\n realtime routing table\n");
	int i;
	for(i=0;i<number_of_node;i++)
		if(real[i] >=0)
			printf("\n destination %d with gateway:%d",i,real[i]);		
}
void delete_routing_entry(char **neighbor_table,int table_index,char *interface,unsigned long *ip_address_table_one,int number_of_node)
{
       char cmd[50]; 
	int	flags;
	int mib[6];
	size_t needed;
	char *host, *lim, *buf, *next;
	struct rt_msghdr *rtm;
	struct sockaddr_in *sin;
	struct sockaddr_dl *sdl;
	extern int h_errno;
	struct hostent *hp;

	u_long addr =0;
	
	mib[0] = CTL_NET;
	mib[1] = AF_ROUTE;
	mib[2] = 0;
	mib[3] = AF_INET;
	mib[4] = NET_RT_FLAGS;
	mib[5] = RTF_LLINFO;
	if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0)
		err(1, "route-sysctl-estimate");
	if ((buf = malloc(needed)) == NULL)
		err(1, "malloc");
	if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0)
		err(1, "actual retrieval of routing table");
	lim = buf + needed;
	int i=0;

	for (next = buf; next < lim; next += rtm->rtm_msglen)
       {
		rtm = (struct rt_msghdr *)next;
		sin = (struct sockaddr_in *)(rtm + 1);
		sdl = (struct sockaddr_dl *)(sin + 1);
		if (addr) {
			if (addr != sin->sin_addr.s_addr)
				continue;	
		} 
        char if_name[IF_NAMESIZE];
        if(if_indextoname(rtm->rtm_index,if_name)==NULL)
			    continue;
		if(strcmp(if_name,interface) == 0 )
		{
			for(i =0;i < number_of_node; i++)
			{
				if(sin->sin_addr.s_addr == ip_address_table_one[i] && neighbor_table[table_index][i] == 1)
				{
					sprintf(cmd,"route delete %s", (char*)inet_ntoa(sin->sin_addr));
					printf("\ndelete route %s",cmd);
				    system(cmd);
				}
			}
		}
	}
}


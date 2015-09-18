#ifndef _ROUTING_H_
#define _ROUTING_H_

#include <netinet/in.h>
#include "id_ip.h"

#define MAX_NEIGHBOR 50

typedef struct rt_entry
{
	unsigned long destination;
	unsigned long gateway;
}rt_entry;

typedef struct kernel_routing_table
{
	rt_entry entry[MAX_NEIGHBOR];
	int number_entry;		
}kernel_routing_table;

void get_routing_table(char *interface,kernel_routing_table *RT);
void parse_ker_rt_to_realtime_rt(kernel_routing_table *RT,int *realtime_rt,int max_realtime_rt, id_ip *ID_IP);
void reset_kernel_routing_table(kernel_routing_table *RT);
void reset_realtime_routing_table(int *RT, int number_of_node);
int compare_realtime_routing_table(int *realtime_rt1,int *realtime_rt2,int max_realtime_rt);
void copy_realtime_routing_table(int *realtime_rtd,int *realtime_rts,int max_realtime_rt);
void print_realtime_routing(int *real,int number_of_node);
void delete_routing_entry(char **neighbor_table,int table_index,char *interface,unsigned long *ip_address_table_one,int number_of_node);
#endif

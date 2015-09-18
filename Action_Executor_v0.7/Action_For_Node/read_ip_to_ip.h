#ifndef _READ_IP_TO_IP_H_
#define _READ_IP_TO_IP_H_


#include <netinet/in.h>
typedef struct ip_to_ip
{	
	struct ip_to_ip *next;
	unsigned long ip_address_one;
	unsigned long ip_address_two;

}ip_to_ip;


ip_to_ip* create_ip_to_ip(char *address1, char *address2);
ip_to_ip * read_ip_to_ip_file(char *path, int *p_size);
void print_ip_to_ip(ip_to_ip *p);
unsigned long look_up_ip_two(ip_to_ip *p,struct in_addr *ip1);
unsigned long look_up_ip_one(ip_to_ip *p,struct in_addr *ip2);
void print_ip_to_ip(ip_to_ip *p);

#endif

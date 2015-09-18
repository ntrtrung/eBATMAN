#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "read_ip_to_ip.h"

void print_ip_to_ip_node(ip_to_ip *t)
{
	if(t == NULL)
		return;
	struct in_addr t1;
	struct in_addr t2;
	t1.s_addr = t->ip_address_one;
	printf("\n ip1:%s",inet_ntoa(t1));
	t2.s_addr = t->ip_address_two;
	printf("\n ip2:%s",inet_ntoa(t2));

}

ip_to_ip* create_ip_to_ip(char *address1, char *address2)
{
	ip_to_ip *temp;
	temp = (ip_to_ip*)malloc(sizeof(ip_to_ip));
	//printf("\naddress1 :%s - address2:%s",address1,address2);
       struct in_addr t;

	if (inet_aton(address1,&t)==0) 
	{
          fprintf(stderr, "inet_aton() failed\n");
      	  return NULL;
       }
	temp->ip_address_one = t.s_addr;
	t.s_addr = 0;
	if (inet_aton(address2,&t)==0) 
	{
          fprintf(stderr, "inet_aton() failed\n");
      	  return NULL;
       }
	temp->ip_address_two = t.s_addr;
	if(temp->ip_address_one == temp->ip_address_two)
		printf("error");
	//print_ip_to_ip_node(temp);
	temp->next= NULL;
	return temp;
}

ip_to_ip * read_ip_to_ip_file(char *path, int *p_size)
{
  static char buf[1024];
  int line_nr=0;
  FILE *fd;
  *p_size =0;
  int node_id;
  char *ip1;
  ip1 = (char*)malloc(sizeof(char)*16);
  char *ip2;
  ip2 = (char*)malloc(sizeof(char)*16);
  ip_to_ip *first;
  ip_to_ip *temp=NULL;
  first=temp;
  int count =0;

  if((fd = fopen(path, "r")) == NULL)
    {
       printf("\nCannot open settings file '%s'", path);
      return NULL;
    }

  while(fgets(buf,1024, fd) != NULL)
    {
	  line_nr++;
	  int scaned_items;
	  scaned_items=sscanf(buf, "%s %s", ip1, ip2);
	  //printf("\n ip1: %s  - ip2:%s",ip1,ip2);
	  if(scaned_items<2)
         {
		  printf("\nSkipped invalid line #%d in settings file '%s'",line_nr, path);
			  continue;
	  }
	  else
	  {
		if(scaned_items == 2)
	       {
		    count ++;
			ip_to_ip *p;
			p = create_ip_to_ip(ip1,ip2);
			if(p == NULL)
			{
				printf("\nerror in reading ip to ip");
				fclose(fd);
				return NULL;
			}
			p->next= NULL;
			if(temp==NULL)
			{
			 	temp=p;
				first=temp;
			}
			else 
			{
			   temp->next = p;
			   temp=temp->next;
			}
			
		}
	  }
    }
  fclose(fd);
  *p_size=count;
  return first;
}
/*
void print_ip_to_ip(ip_to_ip *p)
{
	ip_to_ip *t =p;
	if(t == NULL) return;
	while(t != NULL)
	{
		printf("\n ip1:%s - ip2:%s",inet_ntoa(t->ip_address_one),inet_ntoa(t->ip_address_two));
		t= t->next;
	}
}
*/
unsigned long look_up_ip_two(ip_to_ip *p,struct in_addr *ip1)
{
	ip_to_ip *temp;
	temp = p;
	if( p == NULL ) return 0;
	while(temp!=NULL)
	{
		if(temp->ip_address_one == ip1->s_addr) return temp->ip_address_two;
		else
			temp=temp->next;
	}
	return 0;
}
unsigned long look_up_ip_one(ip_to_ip *p,struct in_addr *ip2)
{
	ip_to_ip *temp;
	temp = p;
	if( p == NULL ) return 0;
	while(temp!=NULL)
	{
		if(temp->ip_address_two == ip2->s_addr) return temp->ip_address_one;
		else
			temp=temp->next;
	}
	return 0;
}


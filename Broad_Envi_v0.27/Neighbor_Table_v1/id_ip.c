#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include "id_ip.h"

//-------------------function for reading setting file-----------------------------////////
id_ip * create_id_ip (int id, char *address)
{
	id_ip *temp;
	temp = (id_ip*)malloc(sizeof(id_ip));
	temp->id=id;
	temp->ip_address = (char*)malloc(strlen(address)+1);
	memset(temp->ip_address,0,strlen(address)+1);
	memcpy(temp->ip_address,address,strlen(address));
	temp->next= NULL;
	return temp;
}

id_ip * read_setting_file(char *path, int *p_size)
{
  static char buf[1024];
  int line_nr=0;
  FILE *fd;
  *p_size =0;
  int node_id;
  char *node_ip;
  node_ip = (char*)malloc(sizeof(char)*16);
  id_ip *first;
  id_ip *temp=NULL;
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
	  scaned_items=sscanf(buf, "%d %16s", &node_id, node_ip);
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
			id_ip *p;
			p = (id_ip*)malloc(sizeof(id_ip));
			p->id=node_id;
			p->ip_address = (char*)malloc(strlen(node_ip)+1);
			memset(p->ip_address,0,strlen(node_ip)+1);
			memcpy(p->ip_address,node_ip,strlen(node_ip));
			p->next= NULL;
                     print_id_ip(p);
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
void print_id_ip(id_ip *p)
{
	id_ip *t =p;
	if(t == NULL) return;
	while(t != NULL)
	{
		printf("\n id:%d with ip :%s",t->id,t->ip_address);
		t= t->next;
	}
}
char *look_up_ip(id_ip *p,int node_id)
{
	id_ip *temp;
	temp = p;
	if( p == NULL ) return NULL;
	while(temp!=NULL)
	{
		if(temp->id == node_id) return temp->ip_address;
		else
			temp=temp->next;
	}
	return NULL;
}

int look_up_id(id_ip *p,char *ip)
{
	if(ip == NULL)
		return -1;
	printf("\nlook up id of ip:%s",ip);
       id_ip *temp;
	temp = p;
	if( p == NULL ) return -1;
	while(temp!=NULL)
	{
		printf("\nid:%d - ip:%s",temp->id,temp->ip_address);
		if(strcmp(ip,temp->ip_address)== 0) 
			return temp->id;
		else
			temp=temp->next;
	}
	return -1;	
}
int look_up_next_id(int id,id_ip *p)
{
	id_ip *temp;
	temp = p;
	if( p == NULL ) return -1;
	while(temp!=NULL)
	{
		if(temp->id == id)
		{
			if(temp->next == NULL)
				return p->id;
			return temp->next->id;
		}
		else
			temp=temp->next;
	}
	return -1;
}


#ifndef _ID_IP_H_
#define _ID_IP_H_

typedef struct id_ip
{	
	struct id_ip *next;
	int id;
	char *ip_address;

}id_ip;

int look_up_next_id(int id,id_ip *p);
id_ip * create_id_ip (int id, char *address);
id_ip * read_setting_file(char *path, int *p_size);
char *look_up_ip(id_ip *p,int node_id);
void print_id_ip(id_ip *p);
int look_up_id(id_ip *p,char *ip);
#endif

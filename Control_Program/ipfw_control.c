#include <stdio.h>
#include <stdlib.h>
#include "ipfw_control.h"

void add_rule(int rule_num,char *source, int port, char *interface)
{
	 char cmd[50]; 
	 sprintf(cmd,"ipfw -q add %d allow udp from %s to any %d in via %s",rule_num,source,port,interface);
	 system(cmd);
}

void delete_rule(int rule_num)
{
	char cmd[50]; 
	sprintf(cmd,"ipfw -q delete %d",rule_num);
	system(cmd);
}	

void add_global_rule(char * interface,int port)
{
     char cmd[50]; 
	 sprintf(cmd,"ipfw -q add %d deny udp from any to any %d in via %s",GLOBAL_RULE,port,interface);
	 system(cmd);
}

void delete_global_rule()
{
	char cmd[50]; 
	sprintf(cmd,"ipfw -q delete %d",GLOBAL_RULE);
	system(cmd);
}
void clear_all_rules()
{
	char cmd[50]; 
	sprintf(cmd,"ipfw -q flush");
	system(cmd);
	sprintf(cmd,"ipfw -q add 65000 allow ip from any to any");
	system(cmd);
}

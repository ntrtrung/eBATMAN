#include<stdio.h>
#include<stdlib.h>
#include"action_record.h"
#include "create_action_binary.h"
#include "read_config.h"
#include "read_ip_to_ip.h"
#include "id_ip.h"
#include "network.h"
#include "config_files.h"

config_files cf;

int get_node_id()
{ 
	be_configuration  be;
      ip_to_ip *ip_to_ip_table;
     id_ip *id_ip_table;
	 
	if(read_config_file(&be,cf.be_config)== -1)
	{
		printf("\nCannot load be_config.xml");
		exit(0);
	}
	int count;
	ip_to_ip_table = read_ip_to_ip_file(cf.ip_to_ip,&count);
	if(ip_to_ip_table == NULL)
	{
		printf("\nCannot load ip_to_ip");
		exit(0);
	}
	//print_ip_to_ip(ip_to_ip_table);

	id_ip_table = read_setting_file(cf.setting_file,&count);
	if(id_ip_table == NULL)
	{
		printf("\nCannot read setting file");
		exit(0);
	}
	printf("\nnumber of node in setting file:%d\n",count);	
           
	struct in_addr ip_normal_server;
	ip_normal_server.s_addr = get_ip_of_interface(be.control_interface);
       printf("\nControl interface:%s",(char*)inet_ntoa(ip_normal_server));
   
	struct in_addr ip_experiment_inferface ;
	ip_experiment_inferface.s_addr = look_up_ip_one(ip_to_ip_table,&ip_normal_server);
       printf("\nExperiment interface:%s",(char*)inet_ntoa(ip_experiment_inferface));

	//get node id 
	int node_id;
	node_id = look_up_id(id_ip_table,(char*)inet_ntoa(ip_experiment_inferface));
	if(node_id == -1)
	{
		printf("\nCannot find node id with interface:%s",(char*)inet_ntoa(ip_normal_server));
		exit(0);
	}
       printf("\n Node ID::%d\n",node_id);
	return node_id;

}
int main(int argc, char **argv)
{

    if(read_config_files(&cf)== -1)
	{
		printf("\nError in reading config files");
		exit(0);
	}

	char *filename = "binary.out";
	char *xml_file =cf.action;
	if(create_binary_from_xml_file(xml_file,filename) == -1 )
	{
		printf("\nError in create action binary file");
		exit(0);
	}
	int count;
	ActionRecord *ac = NULL;
	ac = ReadOutputFile(filename,&count);
	
	if(ac == NULL)
	{
		fputs ("\ncannot reading file\n",stderr); 
		return -1;
	}   
	int id	= get_node_id();
	char *action_record_file = cf.action_record;
	int i;
	int numberofrecord =0;
	for( i=0; i< count; i++)
	{
		if( ac[i].source == id)
		{
			numberofrecord++;
			WriteToFile(action_record_file,&ac[i]);
			PrintAction(&ac[i]);	
		}
		
	}
	if(numberofrecord > 0 )
		WriteCountToFile(action_record_file,&numberofrecord);
	else
		printf("\nNo record is written to binary file\n");
	free(ac);
	return 0;
}




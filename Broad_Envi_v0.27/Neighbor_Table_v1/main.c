#include <stdio.h>
#include <stdlib.h>
#include "read_config.h"
#include "read_ip_to_ip.h"
#include "io.h"
#include "id_ip.h"
#include "network.h"
#include "neighbor_table.h"
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
		printf("\nCannot load ip_to_ip.txt");
		exit(0);
	}
	print_ip_to_ip(ip_to_ip_table);

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
       printf("\n Node ID::%d",node_id);
	return node_id;

}

int main()
{
	int node_id ;
	binary_header_class bhc;
    binary_time_record_class btrc;
	
	if(read_config_files(&cf)==-1)
	{
		printf("\nCannot load config_xml.xml");
		exit(0);
	}
	
	FILE *f;
	char *filename = cf.scenario_file_bin;
	f = fopen(filename,"rb");
	if(f == NULL)
		return 0;
	io_read_binary_header_from_file(&bhc,f);
	io_binary_print_header(&bhc);
	
       node_id = get_node_id();
	
	char **record_table;
	record_table=(char**)malloc(sizeof(char*)*bhc.time_record_number);
	if (record_table == NULL)
	{
		printf("\nCannot alloc for record table");
		exit(0);
	}
	int i;
	for(i =0;i<bhc.time_record_number;i++)
	{
		record_table[i] = (char*)malloc(sizeof(char)*bhc.node_number+1);
		memset(record_table[i],0,bhc.node_number+1);
	}
	int count =0;
	int j;
	int k;

	for(i =0;i<bhc.time_record_number;i++)
	{
		io_read_binary_time_record_from_file(&btrc,f);
		//printf("\ni:%d with record number %d and time:%f\n",i,btrc.record_number,btrc.time);
		binary_record_class * binary_records;
		binary_records = (binary_record_class *)malloc(sizeof(binary_record_class)* btrc.record_number);
		if(binary_records == NULL)
		{
			printf("\nNot enough memory");
			exit(0);
		}
		io_read_binary_records_from_file(binary_records,btrc.record_number,f);
		if(btrc.record_number <=0 )
			continue;
		//io_binary_print_record(binary_records);

		

		for(k=0;k<btrc.record_number;k++)
		{
			if( binary_records[k].from_node == node_id)
			{
				//printf("\nbinary_records[k].loss_rate:%f",binary_records[k].loss_rate);
				if(binary_records[k].loss_rate < 0.8)
					record_table[i][binary_records[k].to_node] = 1;
				else
					record_table[i][binary_records[k].to_node] = -1;
				continue;
			}
			if( binary_records[k].to_node == node_id)
			{								
				//printf("\nbinary_records[k].loss_rate:%f",binary_records[k].loss_rate);
				if(binary_records[k].loss_rate < 0.8)
					record_table[i][binary_records[k].from_node] = 1;
				else
					record_table[i][binary_records[k].from_node] = -1;
		
			}

		}
		free(binary_records);		
		
	}
	//refine_neighbors_record1(records,bhc.time_record_number);
	//printf("\nrefine neighbor table");
	for(i =1;i<bhc.time_record_number;i++)
		for(j=0;j<bhc.node_number;j++)
		{
			if(record_table[i][j] == 0)
			{
				record_table[i][j] = record_table[i-1][j];
			}
		}
	//sum all neighbors in a point of time
	int temp =0;
	for(i =1;i<bhc.time_record_number;i++)
	{
		for(j=0;j<bhc.node_number;j++)
		{
			if(record_table[i][j] == 1)
				temp++;
		}
		record_table[i][j] = temp;
		temp =0;
	}
	//printf("\n refine finish");
	char *file_bin = cf.neighbor_table_bin;
	write_neighbor_time_table_to_file(file_bin,record_table,bhc.time_record_number,bhc.node_number + 1,node_id);
	close(f);
	printf("\n write successfully");
	int number_of_time_record;
	int number_of_node;
	//char** tables= load_neighbor_record_table_file(cf.neighbor_table_bin,&number_of_time_record,&number_of_node,node_id);
	//if(tables == NULL)
	//{
		//printf("\nCannot load neighbor table file");
		//return 0;
	//}	
	
	/*for(i =0;i<number_of_time_record;i++)
	{

		for(j=0;j<number_of_node;j++)
		{
			printf("%d ",tables[i][j]);
		}
		printf("\n");
	}*/

	return 0;
}

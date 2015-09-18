#ifndef _NEIGHBOR_TABLE_
#define _NEIGHBOR_TABLE_

#include "io.h"

typedef struct 
{
	int node_id;
	int number_time_record;
	int number_node;
}header_neighbor_table_file;

char** load_neighbor_record_table_file(char *filename,int *number_of_time_record,int *number_of_node,int node_id);
void write_neighbor_time_table_to_file(char *filename,char** record_table,int number_of_time_record,int number_of_node,int node_id);
void print_header_neighbor_table_file(header_neighbor_table_file *header);


#endif

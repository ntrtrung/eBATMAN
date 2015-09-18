#include<stdio.h>
#include "neighbor_table.h"

int main(char **argv,int argc)
{

	int node_id = 0;
	char *filename = "bin_neighbor_table.bin";
	int number_of_time_record;
	int number_of_node;
	char** tables= load_neighbor_record_table_file(filename,&number_of_time_record,&number_of_node,node_id);
	if(tables == NULL)
	{
		printf("\nCannot load neighbor table file");
		return 0;
	}	
	int i,j;
	for(i =0;i<number_of_time_record;i++)
	{
		printf("\nline %d:",i);
		for(j=0;j<number_of_node;j++)
		{
			printf(" %d ",tables[i][j]);
		}
	}
	
	return 0;
}

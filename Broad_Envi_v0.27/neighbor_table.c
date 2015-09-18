#include <stdio.h>
#include "neighbor_table.h"
#include "io.h"


void print_header_neighbor_table_file(header_neighbor_table_file *header)
{
	printf("\nnode id:%d",header->node_id);
	printf("\nnumber_time_record:%d",header->number_time_record);
	printf("\nnumber_node:%d",header->number_node);
}

void write_neighbor_time_table_to_file(char *filename,char** record_table,int number_of_time_record,int number_of_node,int node_id)
{
	printf("\n Begin to write");
	FILE *f;
	f = fopen(filename,"wb");
	if(f == NULL)
	{
		printf("\nCannot writing record time table to file");
		exit(0);
	}
	if(filename == NULL)
		return;
	if(record_table == NULL)
		return;
	header_neighbor_table_file header;
	header.node_id = node_id;
	header.number_time_record = number_of_time_record;
	header.number_node = number_of_node;

	if (fwrite(&header, sizeof(header_neighbor_table_file),1, f) != 1)
   	 {
	      printf ("Error writing header record to file");
      		perror ("fwrite");
      		exit(0);
	}
	int i,j;
	printf("\nnumber_of_time_record:%d",number_of_time_record);
	for(i =1;i<number_of_time_record;i++)
	{
		for(j=0;j<number_of_node;j++)
		{
			if (fwrite(&record_table[i][j],sizeof(char),1, f) < 0 )
			{
	       	   printf ("Error record table to file");
      			   perror ("fwrite");
    		   	   fclose(f);
      			   exit(0);
			}
			//printf("%d ",record_table[i][j]); 
		}
		//printf(" i:%d\n",i);
	}
	fclose(f);
}

char** load_neighbor_record_table_file(char *filename,int *number_of_time_record,int *number_of_node,int node_id)
{
	FILE *f;
	f = fopen(filename,"rb");
	if(f == NULL)
	{
		printf("\nCannot reading record time table to file");
		exit(0);
	}

	header_neighbor_table_file header;
	if (fread (&header,sizeof(header_neighbor_table_file),1, f) != 1)
   	 {
	      printf ("\nError reading header of file");
      		perror ("fread");
     		return NULL;
	}
	
	//print_header_neighbor_table_file(&header);
	if(node_id != header.node_id)
	{
		printf("\nError reading: wrong file with node id:%d",header.node_id);
		exit(0);
	}
	char **record_table;
	record_table=(char**)malloc(sizeof(char*)*header.number_time_record);
	int i,j;
	for(i =0;i<header.number_time_record;i++)
	{
		record_table[i] = (char*)malloc(sizeof(char)*header.number_node);
		memset(record_table[i],0,header.number_node);
	}
	for(i =0;i<header.number_time_record;i++)
	{
		for(j=0;j<header.number_node;j++)
		{
			if (fread(&record_table[i][j],sizeof(char),1, f) < 0 )
			{
	       	   printf ("Error record table to file");
      			   perror ("fwrite");
    		   	   fclose(f);
      			   exit(0);
			}
			//printf("%d ",record_table[i][j]); 
		}

	}
       *number_of_time_record = header.number_time_record;
	*number_of_node = header.number_node;
	fclose(f);
	return record_table;
}

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include "direct_ma.h"


void print_debug(char *s)
{
	printf("\n");
	puts(s);
	printf("\n");
}

file_node *create_file_node(char *filename,char *path,int checked)
{
	file_node *m;
	m =(file_node*)malloc(sizeof(file_node));
	m->next = NULL;
	m->is_checked = checked;
	m->path = (char*)malloc(strlen(path) +1);
	memset(m->path,0,strlen(m->path));
	memcpy(m->path,path,strlen(path));
	m->path[strlen(path)] = '\0';
	m->filename = (char*)malloc(strlen(filename) +1);
	memset(m->filename,0,strlen(m->filename));
	memcpy(m->filename,filename,strlen(filename));
	m->filename[strlen(filename)] = '\0';
	return m;	
}


file_list *create_list()
{

	file_list *m =(file_list*) malloc(sizeof(file_list));
	m->file = create_file_node(" "," ",0);
	m->size =0;
	return m;
}

char *is_pushed_list(file_list *ST, file_node *m)
{
	
	if(ST->size <=0)
		return NULL;
	file_node *t = ST->file;
	if(t == NULL)
		return NULL;
	while(t!= NULL)
	{
		if(strcmp(t->filename,m->filename)==0)
		{
			return t->path;
		}
		t=t->next;
	}
	return NULL;
}
void get_modified_time(time_t *t,char *file_path)
{
	struct stat buff;
	if(stat(file_path,&buff)== -1)
		return;
	*t = buff.st_mtime;
}

int is_file_modified(file_node *file1,char *file2)
{
	time_t t2;
	get_modified_time(&t2,file2);
	if(difftime(file1->last_modify,t2) ==0) return 0;
	return 1;
}
void update_last_modify(file_node *t,char *path)
{
	time_t time ;
	get_modified_time(&time,path);
	memcpy(&(t->last_modify),&time,sizeof(time));
	t->is_checked =0;
}
int list_push(file_list *ST,file_node *m)
{
	
	char *path = is_pushed_list(ST,m);
	file_node *t = ST->file;
	if(path == NULL)
	{
		while(t->next!=NULL)
			t=t->next;
		t->next = create_file_node(m->filename,m->path,0);
		ST->size ++;
		return 1;
	}
	t = ST->file;
	while(strcmp(t->path,path)!=0)
		t=t->next;
	if(is_file_modified(t,path) == 1)
		update_last_modify(t,path);	
	return 0;
}


char *path_cat (const char *str1, char *str2) {
	size_t str1_len = strlen(str1);
	size_t str2_len = strlen(str2);
	char *result;
	result = malloc((str1_len+str2_len+1)*sizeof *result);
	strcpy (result,str1);
	int i,j;
	for(i=str1_len, j=0; ((i<(str1_len+str2_len)) && (j<str2_len));i++, j++) {
		result[i]=str2[j];
	}
	result[str1_len+str2_len]='\0';
	return result;
}

void print_modified_time(char *path)
{
	struct stat buff;
	if(stat(path,&buff)== -1)
		return;
	struct tm  *ts;
	char   buffer[80];
	ts = localtime(&buff.st_mtime);
	strftime(buffer, sizeof(buffer), "%a %Y-%m-%d %H:%M:%S %Z", ts);
	printf("%s", buffer);
}
void print_file_list(file_list *ST)
{
	file_node *t = ST->file;
	while(t!=NULL)
	{
		puts(t->path);
		printf("\t");
		print_modified_time(t->path);
		printf("\n");
		t=t->next;
	}
			
}
void read_directory(char *dir_path,file_list *ST)
{
	struct dirent *dp;
	DIR *dir = opendir(dir_path);
	if(dir == NULL)
	{
		printf("\n This directory does not exist");
		exit(1);
	}
	while ((dp=readdir(dir)) != NULL) {
		if(strcmp(dp->d_name,".")==0)
			continue;
		if(strcmp(dp->d_name,"..")==0)
			continue;
		char *tmp;
		tmp = path_cat(dir_path, dp->d_name);
		file_node *t = create_file_node(dp->d_name,tmp,0);
		if(list_push(ST,t)==0){
			free(t->path);
			free(t->filename);
			free(t);
		}
		else
		{
			free(t->path);
			free(t->filename);
			free(t);
		}
		free(tmp);
		tmp=NULL;
	}
	closedir(dir);	
}

#ifndef _DIRECT_MA_H
#define _DIRECT_MA_H

#include <time.h>
typedef struct file_node
{
	struct file_node *next;
	char *path;
	char *filename;
	time_t last_modify;
	int is_checked;
}file_node;

typedef struct file_list
{
	file_node *file;
	int size;
}file_list;

void print_debug(char *s);
file_node *create_file_node(char *filename,char *path,int checked);
file_list *create_list();
char *is_pushed_list(file_list *ST, file_node *m);
void get_modified_time(time_t *t,char *file_path);
int is_file_modified(file_node *file1,char *file2);
void update_last_modify(file_node *t,char *path);
int list_push(file_list *ST,file_node *m);
char *path_cat (const char *str1, char *str2) ;
void print_modified_time(char *path);
void print_file_list(file_list *ST);
void read_directory(char *dir_path,file_list *ST);



#endif

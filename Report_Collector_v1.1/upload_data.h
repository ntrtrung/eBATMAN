#ifndef _UPLOAD_DATA_H_
#define _UPLOAD_DATA_H_

#include "direct_ma.h"

file_list *init_list_of_file(char * directory);
void update_file_list(char *directory,file_list *ST);
int upload_file(file_node *t, int sock);
int upload_data(file_list *ST ,char *ip_server,char * directory,char *node_id);
 char *itoa(int n);

#endif

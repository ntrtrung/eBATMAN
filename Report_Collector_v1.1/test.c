#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <net/if.h>
#include "upload_data.h"

char * directory = "/root/logfile/";
char *ip_server;
char *ip_data_server = "10.0.0.1";
file_list *ST;

void Upload_Data()
{
	if( upload_data(ST ,ip_data_server,directory) == -1)
		printf("Upload data error");
	else
        printf("\nData have been uploaded successfuly \n");

}
int main()
{
	ST = init_list_of_file(directory);
    Upload_Data();
}

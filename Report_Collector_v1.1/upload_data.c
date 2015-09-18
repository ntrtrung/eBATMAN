#include <stdio.h>
#include <netinet/in.h>
#include "direct_ma.h"
#include "ftp_main.h"
#include "upload_data.h"


void reverse(char *s)
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }
 char *itoa(int n)
 {
 
     int i, sign;
	 char *s;
	 s = (char *)malloc(5);
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
	return s;
 }


file_list *init_list_of_file(char * directory)
{
	 file_list *ST = create_list();
     read_directory(directory,ST);
	 return ST;
}

void update_file_list(char *directory,file_list *ST)
{
	 read_directory(directory,ST);
}

int upload_file(file_node *t, int sock)
{
	if (ftp_pasv(sock, 0) == 0)
		printf("[+] pasv\n");
	else
		return -1;
	
    if (ftp_put(sock,t->filename,t->path,0) == 0)
	{
		printf("[+] sent:%s\n",t->filename);
		return 1;
	}
	printf("[-] sent ERROR:%s\n",t->filename);
	return -1;

}

int upload_data(file_list *ST ,char *ip_server,char * directory,char* node_id)
{
	int sock;
	struct sockaddr_in server;
	sock = ftp_connect(ip_server, 21, &server, 1);
    char *filename;
	file_node *t = ST->file->next;
	update_file_list(directory,ST);
	
	//Connect to FILE SERVER --------------------------------------//
	if (ftp_auth(sock, "anonymous", "test@yaho.com", 0) == 0)
		printf("--[ Authenticated\n");
	else
		return -1;
	
	//-----------CREATE FOLDER---------------------//
	if (ftp_pasv(sock, 0) == 0)
		printf("[+] pasv\n");
	else
		return -1;
   if (ftp_mkd(sock,node_id, 0) == 0)
		printf("[+] mkdir\n");
		
   //--------------ENTER FOLDER------------------------//
   if (ftp_cwd(sock,node_id, 0) == 0)
	     	printf("[+] cwd\n");
   //-----------UPLOAD FILES---------------------------------------------------//
   
   	while(t!=NULL)
	{
		if(t->is_checked == 0)
		{
			
			if(upload_file(t,sock)!= -1)
			{
				t->is_checked = 1;
			}
		}
		t=t->next;
	}
	///-----------------------------------------------------------------------
	if (ftp_quit(sock, 0) == 0)
		printf("Operations Done!\n");
	//-----------------------------------------------------------------------
 	return 1;
}


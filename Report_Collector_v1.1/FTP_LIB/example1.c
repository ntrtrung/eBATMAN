/*

  gcc example1.c -lOftp -o example1
  ./example1

*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include "ftp_main.h"


int main(int argc, char *argv[]) {

int sock;
struct sockaddr_in server;

sock = ftp_connect("10.0.0.1", 21, &server, 1);

if (ftp_auth(sock, "anonymous", "test@yaho.com", 1) == 0)
	printf("--[ Authenticated\n");
else
	exit(1);

if (ftp_system (sock, 0) == 0) {
	extern char system_name[1050];
	printf("System Name: %s\n", system_name);
		
}

if (ftp_type (sock, 3, 1) == 0)
	printf("[+] type\n");

if (ftp_pasv(sock, 0) == 0)
	printf("[+] pasv\n");
	else
	printf("[-] Error\n");

if (ftp_put(sock,"libOftp.a","libOftp.a", 0) == 0)
	printf("[+] sent\n");
	else
	printf("[-] ERROR\n");

if (ftp_quit(sock, 0) == 0)
	printf("Operations Done!\n");
 return 1;
}

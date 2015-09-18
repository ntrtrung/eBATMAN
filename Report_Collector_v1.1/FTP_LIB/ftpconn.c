/*

    libftp 1.0 (stable): this is an FTP library to simplify the work to a Developer
    who want to work with FTP servers (RFC 959).
    Copyright (C) 2007/2008  omnipresent - omnipresent[at]email.it

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>

*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "ftp_main.h"
/* Struct sockaddr_in for the connections of the transfers way */
struct sockaddr_in ftp_connection;

/*** 
     ftp_connect:
	- char *host           => HostName/IP
	- int port             => Port to connect to
	- struct sockaddr_in * => pointer to sockaddr_in structure
	- int verbose          => Verbose?
           . 0 Do not Print
           . 1 Print
	- ret                  => Success?
	   . sock Socket Descriptor Returned
	   .-1 Error
***/


int ftp_connect(char *host, int port, struct sockaddr_in *ftp_server, int verbose) {


	memset(ftp_server,0,sizeof(ftp_server));
	ftp_server->sin_addr.s_addr = inet_addr(host);
	ftp_server->sin_port = htons(port);
	ftp_server->sin_family = AF_INET;
	
       ftp_connection.sin_family = AF_INET;
       ftp_connection.sin_addr.s_addr = inet_addr(host);

	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {

	if (verbose) {
	perror("Socket");
	}

	return -1;
	}
	connect(sock, (struct sockaddr *)ftp_server, sizeof(struct sockaddr));
	struct timeval tm; //tv_sec - tv_usec
	fd_set readfds;

	tm.tv_sec = 4;
	tm.tv_usec = 0;

	FD_ZERO(&readfds);
	
	FD_SET(sock, &readfds);
	char buf[1024];
	buf[1023] = '\0';
	int n;
	FD_ZERO(&readfds);

	tm.tv_sec = 3;
	tm.tv_usec = 0;

	FD_SET(sock, &readfds);
	sleep(1); //wait the banner..

	if (select(sock+1, &readfds, NULL, NULL, &tm) < 0 ) {
		if (verbose) {
		perror("select()");
		}
		return -1;
	} else {

		if (FD_ISSET(sock, &readfds)) {
			if ( (n = recv(sock, buf, 1022, 0)) < 0)  {
				if (verbose) {
				perror("recv()");
				}
			return -1;
			}

			if (verbose) {
			printf("[Server] %s\n", buf);
			}
		}
		else {
			if (verbose)
			printf("[-] Timeout\n");
			
			return -1;
		}
	}

	buf[n] = '\0';

	return sock; //Socket Descriptor returned!!
}

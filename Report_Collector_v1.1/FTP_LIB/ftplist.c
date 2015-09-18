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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

#include "ftp_main.h"

/***
	ftp_list:
	- int sck     => Socket Descriptor
	- int verbose => Verbose?
		. 0 Do not Print
		. 1 Print
	- ret         => Success?
		. 0 Success
		.-1 Error
		.-3 Error
***/

#define	DATA_CONNECTION_ERROR	-3
#define WRONG_DATA_P		-3

extern struct sockaddr_in ftp_connection;

int ftp_list (int sck, int verbose) {

	FILE *list;
	char *buffer;
	int n = 0;
	int flags;

	buffer = (char *)malloc(1024 * sizeof(char));
		if (buffer == NULL) {
		printf("Unable to alloc 1024 bytes to buffer (ftp_list)\n");
		return -1;
		}

	memset(buffer, 0x0, 1024);
	
	extern int data_port;

	if (data_port == 0) {
		free(buffer);
		return WRONG_DATA_P;
	}


	if (write(sck, "LIST\n", strlen("LIST\n")) == -1) {

		if (verbose) {
		if (errno == EBADF) {
		fprintf(stderr, "Socket Descriptor Not Valid!\n");
		free(buffer);
		return -1;

		} else if (errno == EIO) {

		fprintf(stderr, "I/O Error!\n");
		free(buffer);

		return -1;

		} else if (errno == EINTR) {

		fprintf(stderr, "Signal Interrupted The write() Function\n");
		free(buffer);
	
		return -1;
		}

		} else {

		if (errno == EBADF) {
		free(buffer);
		
		return -1;

		} else if (errno == EIO) {
		free(buffer);
		
		return -1;

		} else if (errno == EINTR) {
		free(buffer);
		
		return -1;
		}

		}

		free(buffer);
		
		return -1;

		}

	int sock;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
	
	free(buffer);
	if (verbose) {
	perror("Socket");
	}
	return -1;
	}

	extern struct sockaddr_in ftp_connection;
	ftp_connection.sin_port = htons(data_port);

	if ( (flags=fcntl(sock, F_GETFL,0)) < 0) {
	printf("--[ fcntl get flags error.\n");
	free(buffer);

	exit(1);
	}

	flags |= O_NONBLOCK;

	if ( fcntl (sock, F_SETFL, flags < 0 ) ) {
		printf("--[ Non Blocking Socket Error.\n");
		free(buffer);
		
		exit(1);
	}

	int con_err = 0;
	
	sleep(1);
	

    if ( (con_err = connect(sock, (struct sockaddr *)&ftp_connection, sizeof(struct sockaddr)) ) != 0)  {

		data_port = 0;
		n = recv(sck, buffer, 1024, 0);
		memset(buffer, 0x0, 1024);
		return DATA_CONNECTION_ERROR;
    }

	
	n = 0;
	list = fopen("LIST.txt", "w");

	if (list == NULL) {
		printf("Unable to open LIST file..\n");
		free(buffer);
		
		return -1;
	}

	buffer[1023] = '\0';

	while (1) {

	n = recv(sock, buffer, 1022, 0);
	
	if (n == 0) {
		sleep(1); //end of Data.. wait a second..
		break;
	}
	
	buffer[n] = '\0';
		
	n = 0;

	fprintf(list, "%s", buffer);

	}

	fclose(list);
	close(sock);

	struct timeval tm; //tv_sec - tv_usec
	fd_set readfds;
	tm.tv_sec = 4;
	tm.tv_usec = 5000;
	sleep(1);
	

	FD_ZERO(&readfds);
	
	FD_SET(sck, &readfds);

	if (select(sck+1, &readfds, NULL, NULL, &tm) < 0 ) {
		
		if (verbose) {
		perror("select()");
		}
		free(buffer);
		
		return -1;
	} else {

		
		if (FD_ISSET(sck, &readfds)) {

			if ( (n = recv(sck, buffer, 1022, 0)) < 0)  {
			if (verbose) {
			perror("recv()");
			}
			free(buffer);
			
			return -1;
			}

			if (verbose)
			printf("[Server] %s\n", buffer);
			
		}
		else {
			if (verbose)
			printf("[-] Timeout\n");
			
			return -1;
		}
	}

	buffer[n] = '\0';

	

	free(buffer);


	return 0;

}

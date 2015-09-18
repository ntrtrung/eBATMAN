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
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include "ftp_main.h"

/*** 
     ftp_get:
	- int sck        => Socket Descriptor
	- char *FileName => The file name that you want to download
	- int verbose    => Verbose?
           . 0 Do not Print
           . 1 Print
	- ret            => Success?
	   . 0 Success
	   .-1 Error
***/

#define	DATA_CONNECTION_ERROR	-3
#define WRONG_DATA_P		-3
#define USR_TOO_LONG		-2

time_t res_get;

int ftp_get (int sck, char *FileName, int verbose) {

	char *buffer;
	int n = 0;
	
	int flags;
	time_t start, end;

	buffer = (char *)malloc(4092*sizeof(char));
		if (buffer == NULL) {
		printf("Unable to alloc 4092 bytes to buffer (ftp_get)\n");
		return -1;
		}

	memset(buffer, 0x0, 4092);

	extern int data_port;

	if (data_port == 0) {

	free(buffer);
	
	return WRONG_DATA_P;

	}

	if (strlen(FileName) > 1024) {

	return USR_TOO_LONG;

	}

	sprintf(buffer, "RETR %s\n", FileName);

	if (write(sck, buffer, strlen(buffer)) == -1) {

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

	memset(buffer, 0x0, 4092);

	sleep(1);
	n = recv(sck, buffer, 4092, MSG_DONTWAIT);

	if (strlen(buffer) != 0) {

	if (ftp_file_handler (buffer, 0) != 0) {
	free(buffer);
	
	return 1;
	}

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
	exit(1);
	}

	flags |= O_NONBLOCK;

	if ( fcntl (sock, F_SETFL, flags < 0 ) ) {
		printf("--[ Non Blocking Socket Error.\n");
		exit(1);
	}

	int con_err = 0;

    if ( (con_err = connect(sock, (struct sockaddr *)&ftp_connection, sizeof(struct sockaddr)) ) != 0)  {
		if (con_err == ECONNREFUSED) {
		free(buffer);

		return -1;		
		}

		data_port = 0;
		if (verbose) {
		perror("connect");
		}

		free(buffer);

		return -1;
    }


	FILE *fd;
	char buff[4096];
	buff[4095] = '\0';

  	int n_bytes;
	int n_recv=0;

	fd = fopen(FileName, "wb");
		
		if (fd == NULL) {
		if (verbose) {
		printf("--[ Error: Unable to open '%s' in ftp_get function\n", FileName);
		}

		free(buffer);
		
		return -1;
		}

	time(&start);

	while (1) {

		usleep(150);
		n_recv = recv(sock, buff, 4094, 0);
		n_bytes = fwrite(buff, sizeof(char), n_recv, fd);
		
		if (n_recv == 0) {
			break;
		}

		n_recv = 0;

		memset(buff, 0x0, n_bytes);
		
		}

	
	fclose(fd);
	close(sock);

	

	memset(buffer, 0x0, 4092);

	while (1) {

	sleep(1);
	n = recv(sck, buffer, 4092, MSG_DONTWAIT);
	if (n == 0)
		break;

	if (n < 0) {

		if ( (errno == ESPIPE) || (errno == EAGAIN) ) {
		break;
		}

	if (verbose) {
		printf("--[ Error: Error in ftp_get() while getting the file\n");
	}
	free(buffer);
	

	return -1;
		
	}

		buffer[n] = '\0';

		
	n = 0;
	memset(buffer, 0x0, 4092);

	}

	memset(buffer, 0x0, 4092);

	time(&end);

	res_get = (int)(end - start);
	
	free(buffer);


	return 0;

}

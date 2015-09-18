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

/* GLOBAL VARIABLES */
/* Data Ports */
/* Port One from the PASV reply */
int data_port1;
/* Port Two from the PASV reply */
int data_port2;
/* Port for the Transfer Connection (p1 * 256) + p2 */
int data_port = 0;

#define WRONG_DATA_P	-3

/***
	ftp_pasv:
	- int sck     => Socket Descriptor
	- int verbose => Verbose?
		. 0 Do not Print
		. 1 Print
	- ret         => Success?
		. 0 Success
		.-1 Error
		.-3 Error
***/


int ftp_pasv (int sck, int verbose) {

	char *buffer;
	int n;
	buffer = (char *)malloc(1024*sizeof(char));
	memset(buffer, 0x0, 1024);


	if (write(sck, "PASV\n", strlen("PASV\n")) == -1) {

		if (verbose) {

		if (errno == EBADF) {
		free(buffer);
		
		fprintf(stderr, "Socket Descriptor Not Valid!\n"); 
		return -1;

		} else if (errno == EIO) {
		free(buffer);
		
		fprintf(stderr, "I/O Error!\n"); 
		return -1;

		} else if (errno == EINTR) {
		free(buffer);
		
		fprintf(stderr, "Signal Interrupted The write() Function\n"); 
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


	struct timeval tm; //tv_sec - tv_usec
	fd_set readfds;
	tm.tv_sec = 3;
	tm.tv_usec = 0;

	FD_ZERO(&readfds);
	
	FD_SET(sck, &readfds);

	buffer[1023] = '\0';

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

			if (verbose) {
			printf("[Server] %s\n", buffer);
			}
		}
		else {
			if (verbose) {
			printf("[-] Timeout\n");
			}
			return -1;
		}
	}

	buffer[n] = '\0';

		
	if (find_pasv(buffer, 0) != 0) { //Error found; passive mode message not found!
		free(buffer);
		data_port = 0;
		
		return -1; //return error

	}
		
	if (verbose)
	printf("(PASV) port: %d <-> %d\n", data_port1, data_port2);
	
	memset(buffer, 0x0, 1024);

	
	data_port = ftp_DataPort(data_port1, data_port2);

	if ( (data_port == 0) || (data_port < 0) ) {
		free(buffer);
		data_port1 = 0;
		data_port2 = 0;
		
		return WRONG_DATA_P;
	}

	free(buffer);

	return 0;

}

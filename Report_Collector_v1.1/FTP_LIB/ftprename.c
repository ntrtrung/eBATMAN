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
	ftp_rename:
	- int sck     => Socket Descriptor
	- int verbose => Verbose?
		. 0 Do not Print
		. 1 Print
	- ret         => Success?
		. 0 Success
		.-1 Error
		. 2 Error
***/

#define DIR_TOO_LONG	2
#define NAME_TOO_LONG	2


int ftp_rename (int sck, char *old_name, char *new_name, int verbose) {

	char buffer[1024]={'\0'};
	char *buf;
	int n;

	buf = (char *)malloc(1024*sizeof(char));

	memset(buffer, 0x0, 1024);
	memset(buf, 0x0, 1024);



if ( strlen(old_name) > 1015 ) {
	free(buf);
	
	return NAME_TOO_LONG;
	}

if ( strlen(new_name) > 1015 ) {
	free(buf);
	
	return NAME_TOO_LONG;
	}

sprintf(buffer, "RNFR %s\n", old_name);

	if (write(sck, buffer, strlen(buffer)) == -1) {

	if (verbose) {

		if (errno == EBADF) {
		free(buf);
		
		fprintf(stderr, "Socket Descriptor Not Valid!\n"); 
		return -1;
		} else if (errno == EIO) {
		free(buf);
		
		fprintf(stderr, "I/O Error!\n"); 
		return -1;
		} else if (errno == EINTR) {
		free(buf);
		
		fprintf(stderr, "Signal Interrupted The write() Function\n"); 
		return -1;
		}

	} else {

		if (errno == EBADF) {
		free(buf);
		
		return -1;
		} else if (errno == EIO) {
		free(buf);
		
		return -1;
		} else if (errno == EINTR) {
		free(buf);
		
		return -1;
		}
	}

		free(buf);
		
		return -1;

		}

	struct timeval tm; //tv_sec - tv_usec
	fd_set readfds;
	tm.tv_sec = 3;
	tm.tv_usec = 0;

	FD_ZERO(&readfds);
	
	FD_SET(sck, &readfds);

	if (select(sck+1, &readfds, NULL, NULL, &tm) < 0 ) {
		if (verbose) {
		perror("select()");
		}
		free(buf);
		
		return -1;
	} else {

		if (FD_ISSET(sck, &readfds)) {

			if ( (n = recv(sck, buf, 1022, 0)) < 0)  {
			if (verbose) {
			perror("recv()");
			}
			free(buf);
			
			return -1;
			}

			if (verbose)
			printf("[Server] %s\n", buf);
			
		}
		else {
			if (verbose)
			printf("[-] Timeout\n");
			
			return -1;
		}
	}

	buf[n] = '\0';


	if (ftp_file_handler(buf, 0) != 0) {
		free(buf);
		
		return -1;
		}

	memset(buffer, 0x0, 1024);
	memset(buf, 0x0, 1024);

sprintf(buffer, "RNTO %s\n", new_name);

	if (write(sck, buffer, strlen(buffer)) == -1) {

	if (verbose) {

		if (errno == EBADF) {
		free(buf);
		
		fprintf(stderr, "Socket Descriptor Not Valid!\n"); 
		return -1;
		} else if (errno == EIO) {
		free(buf);
		
		fprintf(stderr, "I/O Error!\n"); 
		return -1;
		} else if (errno == EINTR) {
		free(buf);
		
		fprintf(stderr, "Signal Interrupted The write() Function\n"); 
		return -1;
		}

	} else {

		if (errno == EBADF) {
		free(buf);
		
		return -1;
		} else if (errno == EIO) {
		free(buf);
		
		return -1;
		} else if (errno == EINTR) {
		free(buf);
		
		return -1;
		}

	}

		free(buf);
		
		return -1;

		}

	FD_ZERO(&readfds);
	
	FD_SET(sck, &readfds);

	if (select(sck+1, &readfds, NULL, NULL, &tm) < 0 ) {
		if (verbose) {
		perror("select()");
		}
		free(buf);
		
		return -1;
	} else {

		if (FD_ISSET(sck, &readfds)) {

			if ( (n = recv(sck, buf, 1022, 0)) < 0)  {
			if (verbose) {
			perror("recv()");
			}
			free(buf);
			
			return -1;
			}

			if (verbose)
			printf("[Server] %s\n", buf);
			
		}
		else {
			if (verbose)
			printf("[-] Timeout\n");
			
			return -1;
		}
	}

	buf[n] = '\0';


	if (ftp_file_handler(buf, 0) != 0) {
		free(buf);
		
		return -1; //Error
		}

	free(buf);
	
	return 0; //Renamed
}

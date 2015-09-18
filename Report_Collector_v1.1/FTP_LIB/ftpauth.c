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

#include "ftp_main.h"

/* Global Variable used to store the password length */

int pwd_len = 0;

#define    USR_TOO_LONG     -2
#define    PWD_TOO_LONG     -2

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



int ftp_auth(int sck, char *usr, char *pwd, int verbose) {

	if (strlen(usr) > 50) {

	return USR_TOO_LONG;

	}

	char *auth = malloc(strlen("USER ")+strlen(usr)+2);
	char buf[1024]={'\0'};
	int n = 0;
		
	sprintf(auth, "%s %s\n", "USER", usr);

	if (write(sck, auth, strlen(auth)) == -1) {

		if (verbose) {

		if (errno == EBADF) {

		fprintf(stderr, "Socket Descriptor Not Valid!\n");
		free(auth);
		return -1;
		} else if (errno == EIO) {

		fprintf(stderr, "I/O Error!\n");
		free(auth);
		
		return -1;
		} else if (errno == EINTR) {

		fprintf(stderr, "Signal Interrupted The write() Function\n");
		free(auth);
		return -1;
		}

		} else {

		if (errno == EBADF) {
		free(auth);
		return -1;
		} else if (errno == EIO) {
		free(auth);
		
		return -1;
		} else if (errno == EINTR) {
		free(auth);
		return -1;
				}

			}

		free(auth);
		
		return -1;

		}

	free(auth);

	buf[1023] = '\0';

	struct timeval tm; //tv_sec - tv_usec
	fd_set readfds;
	tm.tv_sec = 3;
	tm.tv_usec = 0;

	FD_ZERO(&readfds);
	
	FD_SET(sck, &readfds);
	sleep(1); //wait a little bit

	if (select(sck+1, &readfds, NULL, NULL, &tm) < 0 ) {
		if (verbose) {
		perror("select()");
		}
		return -1;
	} else {

		if (FD_ISSET(sck, &readfds)) {

			if ( (n = recv(sck, buf, 1022, 0)) < 0)  {
			if (verbose) {
			perror("recv()");
			}
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

	if (ftp_login_handler(buf, 0) != 2) { //2? Yes.. Need a Password
		return -1;
	}

	memset(buf, 0x0, 1024);

	if (strlen(pwd) > 50) {

		return PWD_TOO_LONG;

		}

	auth = malloc(strlen("PASS ")+strlen(pwd)+2);

	sprintf(auth, "%s %s\n", "PASS", pwd);

	if (write(sck, auth, strlen(auth)) == -1) {

		if (verbose) {
		if (errno == EBADF) {
		fprintf(stderr, "Socket Descriptor Not Valid!\n");
		free(auth);
		
		return -1;

		} else if (errno == EIO) {

		fprintf(stderr, "I/O Error!\n");
		free(auth);
		
		return -1;

		} else if (errno == EINTR) {

		fprintf(stderr, "Signal Interrupted The write() Function\n");
		free(auth);
		
		return -1;
		}

		} else {

		if (errno == EBADF) {
		free(auth);
		
		return -1;

		} else if (errno == EIO) {
		free(auth);
		
		return -1;

		} else if (errno == EINTR) {
		free(auth);
		
		return -1;
				}

			}

	free(auth);
	
	return -1;

	}
		
		pwd_len = strlen(pwd) - 1;

	free(auth);


	FD_ZERO(&readfds);
	
	FD_SET(sck, &readfds);

	sleep(1);

	if (select(sck+1, &readfds, NULL, NULL, &tm) < 0 ) {
		if (verbose) {
		perror("select()");
		}
		return -1;
	} else {

		if (FD_ISSET(sck, &readfds)) {

			if ( (n= recv(sck, buf, 1022, 0)) < 0)  {
			if (verbose) {
			perror("recv()");
			}
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
	
	if (ftp_login_handler(buf, 0) != 0) {
	
	return -1;
	}

	memset(buf, 0x0, 1024);

	return 0;

}

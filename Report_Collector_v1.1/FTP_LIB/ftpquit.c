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

/*** 
     ftp_quit:
     - int sck => Socket Descriptor
     - int verbose => Print Inforations:
           . 0 Do not Print
           . 1 Print
***/


int ftp_quit(int sck, int verbose) {


    if (write(sck, "QUIT\n", strlen("QUIT\n")-1) == -1) {

		if (verbose) {
		if (errno == EBADF) {
		fprintf(stderr, "Socket Descriptor Not Valid!\n");
		
		return -1;

		} else if (errno == EIO) {

		fprintf(stderr, "I/O Error!\n");
		
		return -1;

		} else if (errno == EINTR) {

		fprintf(stderr, "Signal Interrupted The write() Function\n");
		
		return -1;
		}
		
		} else {

		if (errno == EBADF) {
		
		return -1;

		} else if (errno == EIO) {
		
		return -1;

		} else if (errno == EINTR) {
		
		return -1;
		}

		}

		
		return -1;

		}

	if (close(sck) != 0)
		return -1;
	else
		return 0; //0 returned; operation sucessfull!

}

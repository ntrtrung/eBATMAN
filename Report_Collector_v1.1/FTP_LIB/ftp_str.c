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
#include "ftp_main.h"
#include <stdio.h>
#include <string.h>
/* Path of the Current Work Directory */
char path[1050]={'\0'};

/* 
*
* strfnbytes: look for needle into big_str for n bytes
* char *big_str: big string (where to look for needle)
* char *needle: string that you need
* int bytes: first n bytes
*
*/


int strfnbytes(char *big_str, char *needle, int bytes) {

	int i=0;
	int same=0;

		if (strlen(big_str)-1 < bytes) {
			printf("[-] Big String MUST BE major than %d\n", bytes);
			printf("Aborting...\n");
			return (-1);
		}


		if (strlen(needle)-1 >= bytes) {
			printf("[-] needle MUST BE the same as n bytes\n");
			printf("Aborting...\n");
			return (-1);
		}

	
	while (big_str[i] != '\0') {

			if (i >= bytes)
				break;
		
		if (needle[i] == big_str[i]) {
			
			same++;
		} else {
			
			same=0;
			break;
		}

		i++;
		
	}

	if (same != 0)
		return 0;
	else
		return -1;

}

/* 
*
* find_pasv: find the ports in the PASV message reply
* char *StrPasv: the reply string to working on it
* int verbose: verbose mode
* 	- 1 Yes
*	- 0 No
*
*/

int find_pasv (char *StrPasv, int verbose) {

char p1[100];
char p2[100]; 

int port1 = 0;
int port2 = 0;

int len = strlen(StrPasv);

len = len-2;
StrPasv[len] = '\0';

int count = 0;
int i = 0;

	
	if (strstr(StrPasv, ")") == NULL) { //mmh an error occur..
		return 1;
	}

	while (StrPasv[i] != '\0') {
		if (StrPasv[i] == ')') { 
			break;
		}

		i++;
	}

int temp = 0;

temp = i;
i = 0;

int tmp = 0;
int sum = 0;


extern int data_port1;
extern int data_port2;

data_port1 = 0;
data_port2 = 0;

count = temp;

count = count - 1;

memset(p1, 0x0, 100);
memset(p2, 0x0, 100);

	if (verbose == 1) {
	printf("--[ Server: %s\n", StrPasv);
	}

	while (StrPasv[count] != ',') {

	if (i >= 100)
		return -1;

	p1[i] = StrPasv[count];
	
	count--;
	i++;

	}

	p1[i] = '\0';

	
	count--;
	i = 0;

	while (StrPasv[count] != ',') {

	p2[i] = StrPasv[count];
	
	
	count--;
	i++;

	}

	p2[i] = '\0';

	i = 0;
	count = 0;
	tmp = 0;
	sum = 0;

	while (p1[i] != '\0') {

	tmp = 0;
	sum = 0;

	if (i == 1) {
		
	tmp = (int)p1[i] - 48;
	sum = 10*tmp;

	port1 = port1 + sum;

	} else if (i == 2) {

	tmp = (int)p1[i] - 48;
	
	sum = 100*tmp;
	
	port1 = port1 + sum;

	} else if (i == 0) {
		
	tmp = (int)p1[i] - 48;
		
	port1 = port1 + tmp;
	
	} else if ( i > 2) {

	return -1;

	}

	i++;
	
	}

	i = 0;
	count = 0;
	tmp = 0;
	sum = 0;

	while (p2[i] != '\0') {

	tmp = 0;
	sum = 0;

	if (i == 1) {
		
	tmp = (int)p2[i] - 48;
	sum = 10*tmp;

	port2 = port2 + sum;

	} else if (i == 2) {

	tmp = (int)p2[i] - 48;
	
	sum = 100*tmp;
	
	port2 = port2 + sum;

	} else if (i == 0) {
		
	tmp = (int)p2[i] - 48;
		
	port2 = port2 + tmp;

	} else if ( i > 2) {

	return -1;

	}

	i++;
	
	}

data_port2 = port1;
data_port1 = port2;


return 0;


}

/* 
*
* ftp_DataPort: Make the right Data Port for transfer connections
* int PortOne: First port in the PASV reply
* int PortTwo: Second port in the PASV reply
*
*/

int ftp_DataPort (int PortOne, int PortTwo) {

/*** HOWTO Data Transfer Connection:
((port1 * 256) + port2)
***/

extern int data_port;

data_port = 0;

data_port = PortOne * 256;
data_port = data_port + PortTwo;

return data_port;

}

/* 
*
* ftp_path: Reply with the Current Directory
* char *PathCurDir: Reply of the FTPD from the PWD command
* 
*/

int ftp_path (char *PathCurDir) {

int i = 0;
int count = 0;

if (strlen(PathCurDir) > 1024)
	return -1;

while (PathCurDir[i] != '"') {

	if (i >= 1024)
	return -1;
i++;

}

i++;

while (PathCurDir[i] != '"') {

	if (i >= 1024) {
	path[i] = '\0';
	break;
	}

path[count] = PathCurDir[i];
i++;
count++;
}

	if (i >= 1024) {
	path[i] = '\0';
	return 0;
	}

path[count] = '\0';

return 0;

}

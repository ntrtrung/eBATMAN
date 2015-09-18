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
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>

#include "ftp_main.h"


#define CMD_OK         "200"
#define CMD_NI         "202"
#define SYS_STATUS     "211"
#define HLP_MSG        "214"
#define NAME_SYS       "215"
#define READY          "220"
#define CLOSE          "221"
#define NO_TRANSFER    "225"
#define CLOSE_CON      "226"
#define PASSIVE_MOD    "227"
#define PATH_DONE      "257"
#define LOGGED         "230"
#define REQ_FILE_OK    "250"
#define PATH_CREATED   "257"
#define USER_OK	       "331"
#define NEED_ACCT      "332"
#define ACTION_PEND    "350" //350 Requested file action pending further information.
#define SERVICE_NA     "421"
#define NOT_OPEN_DT    "425"
#define CON_CLOSED     "426"
#define FILE_NOT_TAKEN "450" //File not available: eg busy
#define ACT_ABORTED    "451"
#define ACT_NOT_TAKEN  "452"
#define SYNTAX_ERR     "500" //Command unrecognized
#define SYNTAX_ERR_P   "501" //Syntax error in parameters or arguments
#define CMD_NOT_IMPL   "502"
#define BAD_SEQ_CMD    "503"
#define NOT_LOGGED     "530"
#define ACCT_FOR_FILE  "532"
#define FILE_NOT_FOUND "550" //File not found, no access to file
#define ACTION_ABORTED "551"
#define ALLOCATION_ERR "552"
#define FILE_ERR_NAME  "553"

/***
         200 Command okay.
         202 Command not implemented, superfluous at this site.
         211 System status, or system help reply.
         212 Directory status.
         213 File status.
         214 Help message.
             On how to use the server or the meaning of a particular
             non-standard command.  This reply is useful only to the
             human user.
         215 NAME system type.

             Where NAME is an official system name from the list in the
             Assigned Numbers document.
         220 Service ready for new user.
         221 Service closing control connection.
             Logged out if appropriate.
         225 Data connection open; no transfer in progress.
         226 Closing data connection.
             Requested file action successful (for example, file
             transfer or file abort).
         227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
         230 User logged in, proceed.
         250 Requested file action okay, completed.
         257 "PATHNAME" created.

         331 User name okay, need password.
         332 Need account for login.
         350 Requested file action pending further information.

         421 Service not available, closing control connection.
             This may be a reply to any command if the service knows it
             must shut down.
         425 Can't open data connection.
         426 Connection closed; transfer aborted.
         450 Requested file action not taken.
             File unavailable (e.g., file busy).
         451 Requested action aborted: local error in processing.
         452 Requested action not taken.
             Insufficient storage space in system.
         500 Syntax error, command unrecognized.
             This may include errors such as command line too long.
         501 Syntax error in parameters or arguments.
         502 Command not implemented.
         503 Bad sequence of commands.
         504 Command not implemented for that parameter.
         530 Not logged in.
         532 Need account for storing files.
         550 Requested action not taken.
             File unavailable (e.g., file not found, no access).
         551 Requested action aborted: page type unknown.
         552 Requested file action aborted.
             Exceeded storage allocation (for current directory or
             dataset).
         553 Requested action not taken.
             File name not allowed.
             
***/

/*** 
     error_handler:
     - int sck => Socket Descriptor
     - int connected => Value 0 or 1:
           . 0 System Connected
           . 1 System Not Connected
     - int verbose => Print Inforations:
           . 0 Do not Print
           . 1 Print
***/

int ftp_login_handler (char *cmd_reply, int verbose) {

if (verbose == 1) {

	if (strfnbytes(cmd_reply, NEED_ACCT, 3) == 0) {
		printf("--[ Server: Need an Account to Login.\n");
		return -1;
	} else if (strfnbytes(cmd_reply, LOGGED, 3) == 0) {
		printf("--[ Server: User Logged in.\n");
		return 0;
	} else if (strfnbytes(cmd_reply, USER_OK, 3) == 0) {
		printf("--[ Server: User Okay, need a Password.\n");
		return 2;
	} else if (strfnbytes(cmd_reply, NOT_LOGGED, 3) == 0) {
		printf("--[ Server: User not logged in.\n");
		return -1;
	} else {
		printf("--[ Server: Unknown reply at the login process.\n");
		return -1;
	}

} else {

	if (strfnbytes(cmd_reply, NEED_ACCT, 3) == 0) {
		return -1;
	} else if (strfnbytes(cmd_reply, LOGGED, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, USER_OK, 3) == 0) {
		return 2;
	} else if (strfnbytes(cmd_reply, NOT_LOGGED, 3) == 0) {
		return -1;
	} else {
		return -1;
	}

}

}

int ftp_dir_handler (char *cmd_reply, int verbose) {

if (verbose == 1) {

	if (strfnbytes(cmd_reply, CMD_OK, 3) == 0) {
		printf("--[ Server: Command Ok.\n");
		return 0;
	} else if (strfnbytes(cmd_reply, REQ_FILE_OK, 3) == 0) {
		printf("--[ Server: Command successful.\n");
		return 0;
	} else if (strfnbytes(cmd_reply, REQ_FILE_OK, 3) == 0) {
		printf("--[ Server: Command successful.\n");
		return 0;
	} else if (strfnbytes(cmd_reply, REQ_FILE_OK, 3) == 0) {
		printf("--[ Server: Command successful.\n");
		return 0;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR, 3) == 0) {
		printf("--[ Server: Syntax Error.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, PATH_CREATED, 3) == 0) {
		printf("--[ Server: Directory Successfully Created.\n");
		return 0;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR_P, 3) == 0) {
		printf("--[ Server: Syntax Error in Parameters.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_NOT_FOUND, 3) == 0) {
		printf("--[ Server: File Not Found.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, ACTION_ABORTED, 3) == 0) {
		printf("--[ Server: Requested action aborted.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_ERR_NAME, 3) == 0) {
		printf("--[ Server: File Name not allowed.\n");
		return 1;
	} else {
		printf("--[ Server: Unknown reply.\n");
		return 1;
	}

} else {

	if (strfnbytes(cmd_reply, CMD_OK, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, REQ_FILE_OK, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, REQ_FILE_OK, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, REQ_FILE_OK, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, PATH_CREATED, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR_P, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_NOT_FOUND, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, ACTION_ABORTED, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_ERR_NAME, 3) == 0) {
		return 1;
	} else {
		return 1;
	}

}

}

int ftp_file_handler (char *cmd_reply, int verbose) {

if (verbose == 1) {

	if (strfnbytes(cmd_reply, CMD_OK, 3) == 0) {
		printf("--[ Server: Command Ok.\n");
		return 0;
	} else if (strfnbytes(cmd_reply, ACTION_PEND, 3) == 0) {
		printf("--[ Server: Command successful.\n");
		return 0;
	} else if (strfnbytes(cmd_reply, REQ_FILE_OK, 3) == 0) {
		printf("--[ Server: Requested file action completed.\n");
		return 0;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR, 3) == 0) {
		printf("--[ Server: Syntax Error.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_NOT_FOUND, 3) == 0) {
		printf("--[ Server: File Not Found.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR_P, 3) == 0) {
		printf("--[ Server: Syntax Error in Parameters.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, BAD_SEQ_CMD, 3) == 0) {
		printf("--[ Server: File Not Found.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, ACTION_ABORTED, 3) == 0) {
		printf("--[ Server: Requested action aborted.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_NOT_TAKEN, 3) == 0) {
		printf("--[ Server: File busy.\n");
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_ERR_NAME, 3) == 0) {
		printf("--[ Server: File Name not allowed.\n");
		return 1;
	} else {
		printf("--[ Server: Unknown reply.\n");
		return 1;
	}

	

} else {

	if (strfnbytes(cmd_reply, CMD_OK, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, ACTION_PEND, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, REQ_FILE_OK, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_NOT_FOUND, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR_P, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, BAD_SEQ_CMD, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, ACTION_ABORTED, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_NOT_TAKEN, 3) == 0) {
		return 1;
	} else if (strfnbytes(cmd_reply, FILE_ERR_NAME, 3) == 0) {
		return 1;
	} else {
		return 1;
	}


}

}

int ftp_port_handler (char *cmd_reply, int verbose) {

if (verbose == 1) {

	if (strfnbytes(cmd_reply, SYNTAX_ERR, 3) == 0) {
		printf("--[ Server: Need an Account to Login.\n");
		return -1;
	} else if (strfnbytes(cmd_reply, CMD_OK, 3) == 0) {
		printf("--[ Server: OK!\n");
		return 0;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR_P, 3) == 0) {
		printf("--[ Server: User Okay, need a Password.\n");
		return -1;
	} else if (strfnbytes(cmd_reply, SERVICE_NA, 3) == 0) {
		printf("--[ Server: User not logged in.\n");
		return -1;
	} else {
		printf("--[ Server: Unknown reply at the login process.\n");
		return -1;
	}

} else {

	if (strfnbytes(cmd_reply, SYNTAX_ERR, 3) == 0) {
		return -1;
	} else if (strfnbytes(cmd_reply, CMD_OK, 3) == 0) {
		return 0;
	} else if (strfnbytes(cmd_reply, SYNTAX_ERR_P, 3) == 0) {
		return -1;
	} else if (strfnbytes(cmd_reply, SERVICE_NA, 3) == 0) {
		return -1;
	} else {
		return -1;
	}

}

}

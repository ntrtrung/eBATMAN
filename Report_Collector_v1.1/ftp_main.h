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

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

/* Error handlers */
extern int h_errno;
extern int errno;

#ifdef __cplusplus
	extern "C" {
#endif

int strfnbytes(char *big_str, char *needle, int bytes);
int find_pasv (char *StrPasv, int verbose);
int ftp_DataPort (int PortOne, int PortTwo);
int ftp_path (char *PathCurDir);

int ftp_cwd (int sck, char *Directory, int verbose);
int ftp_del (int sck, char *FileName,int verbose);
int ftp_mkd (int sck, char *Directory, int verbose);
int ftp_rename (int sck, char *old_name, char *new_name, int verbose);
int ftp_rmd (int sck, char *Directory, int verbose);
int ftp_type(int sck, int type_cmd, int verbose);
int ftp_noop(int sck, int verbose);
int ftp_dir(int sck, int verbose);
int ftp_system (int sck, int verbose);

int ftp_login_handler (char *cmd_reply, int verbose);
int ftp_dir_handler (char *cmd_reply, int verbose);
int ftp_file_handler (char *cmd_reply, int verbose);

int ftp_pasv (int sck, int verbose);
int ftp_auth(int sck, char *usr, char *pwd, int verbose);
int ftp_connect(char *host, int port, struct sockaddr_in *ftp_server, int verbose);
int ftp_quit(int sck, int verbose);
int ftp_get (int sck, char *FileName, int verbose);
int ftp_list (int sck, int verbose);
int ftp_nlist (int sck, int verbose);
int ftp_put (int sck, char *FileName,char *Path, int verbose);

#ifdef __cplusplus
	}
#endif

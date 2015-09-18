#ifndef _READ_CONFIG_H_
#define _READ_CONFIG_H_

typedef struct rc_configuration
{
	char *rc_interface;
	char *setting_file;
	char *data_server_list;
	char *log_file_path;
}rc_configuration;

int read_config_file(rc_configuration *rc,char *filename);

#endif

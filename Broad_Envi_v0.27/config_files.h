#ifndef _CONFIG_FILES_H_
#define _CONFIG_FILES_H_

typedef struct config_files
{
	char setting_file[50];
	char neighbor_table_bin[50];
	char be_config[50];
	char ip_to_ip[50];
	char action_record[50];
	char action_mapping[50];
	char action[50];

}config_files;

#define CONFIG_FILES_PATH "/root/Read_Config/config_files.xml"
void print_config_files(config_files *cf);
int read_config_files(config_files *cf);

#endif

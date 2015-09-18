#ifndef _READ_CONFIG_H_
#define _READ_CONFIG_H_

typedef struct be_configuration
{
	char *experiment_interface;
	char *control_interface;

}be_configuration;

int read_config_file(be_configuration *rc,char *filename);

#endif

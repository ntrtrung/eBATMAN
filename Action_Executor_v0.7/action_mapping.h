#ifndef _ACTION_MAPPING_H_
#define _ACTION_MAPPING_H_

#define ACTION_MAPPING "action_mapping"
#define ACTION_ID "id"
#define FILE_PATH "file_path"
#define ACTION    "action"

typedef struct action_mapping
{
	struct action_mapping *next;
	int action_id;
	char *filename;
}action_mapping;

action_mapping * create_action_mapping_rc(int id,char *file);
void print_action_mapping(action_mapping *ap);
action_mapping *read_action_mapping_file(char *filename,int *count);
char * look_up_action_file(action_mapping *am,int id);

#endif


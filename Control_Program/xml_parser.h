#ifndef _XML_PARSER_H_
#define _XML_PARSER_H_


typedef struct program_list
{
	char qomet[100];
	char broad_envi[100];
	char do_action[100];
	char olsrd[100];
}program_list;


int read_process_list_xml(program_list *list,char *file);


#endif

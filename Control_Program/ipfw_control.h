#ifndef _IPFW_CONTROL_H_
#define _IPFW_CONTROL_H_


#define GLOBAL_RULE 19999
#define MINIMUM_RULE 10000

void add_rule(int rule_num,char *source, int port, char *interface);
void delete_rule(int rule_num);
void add_global_rule(char * interface,int port);
void delete_global_rule();
void clear_all_rules();

#endif

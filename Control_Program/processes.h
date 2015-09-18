#ifndef _PROCESSES_H_
#define _PROCESSES_H_


typedef struct process
{
  struct process *next;       /* next process in pipeline */
  pid_t pid;                  /* process ID */
  int status;                 /* reported status value */
  float expire_time;
} process;


process * create_process();
void run_command(process *p,pid_t pgid, char** argv);
void run_process(process *p,pid_t pgid,char **argv);
void kill_all_processes(pid_t pgid);
int process_is_alive(process *p);


#endif 


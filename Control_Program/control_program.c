#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include "xml_parser.h"
#include "processes.h"
#include <sys/signal.h>  
#include <sys/types.h>
#include <fcntl.h>

typedef struct main_process
{
	pid_t pid;
	pid_t pgid;
	process *first_child_process;
	
}main_process;

main_process *main_p;
process *olsrd;
process *qomet;
process *broad_envi;
process *do_action;

program_list list_of_program;

int load_program_list(char *xmlfile)
{
	if(read_process_list_xml(&list_of_program,xmlfile) == -1)
	{
		printf("\n Cannot load process list file\n");
		exit(0);
	}
	return 0;
}
void kill_all_child_processes()
{
	if(do_action !=NULL)
		kill(do_action->pid,SIGTERM);
	else
	   printf("\ndo action is killed");	
	if(broad_envi!=NULL)
		kill(broad_envi->pid,SIGTERM);	
	else
	   printf("\nbroad envi is killed");
	if(olsrd!=NULL)
		kill(olsrd->pid,SIGTERM);	
	else
	   printf("\nolsrd is killed");
    clear_all_rules();
}
void signal_handler_main(int sig)
{
	switch(sig) 
	{
		case SIGHUP:
			printf("\nhangup signal catched");
			break;
		case SIGTERM:
		case SIGQUIT:
		case SIGKILL:
		case SIGINT:
			printf("\nterminate signal catched");
			kill_all_child_processes();
			kill_all_processes(main_p->pgid);	
			//clear_all_rules();
			exit(0);
			break;
	}
}
//init main process
void main_process_init()
{

	main_p = (main_process*)malloc(sizeof(main_process));
	main_p->pid = getpid();
	setpgid(main_p->pid,0);
	main_p->pgid=main_p->pid;
	main_p->first_child_process=NULL;
	printf("\n Main process :%d\n",main_p->pid);
}


void wait_to_close()
{
      int status;
	waitpid (WAIT_ANY,&status, WUNTRACED);
}

void handler(int sig)
{
  pid_t pid;

  pid = wait(NULL);

  printf("Pid %d exited.\n", pid);
  kill_all_child_processes();
  kill_all_processes(main_p->pgid);
  //clear_all_rules();

}


void call_programs()
{
	
	char *argv[2];
	argv[1] =(char*)0;

	broad_envi = create_process();
	if(broad_envi == NULL)
	{
			printf("\nerror in create a process\n");
			exit(0);
	}
	do_action = create_process();
	if(do_action == NULL)
	{
			printf("\nerror in create a process\n");
			exit(0);
	}
	olsrd = create_process();
	if(do_action == NULL)
	{
			printf("\nerror in create a process\n");
			exit(0);
	}
	qomet = create_process();
	if(qomet == NULL)
	{
			printf("\nerror in create a process\n");
			exit(0);
	}
	argv[0] = list_of_program.broad_envi;
	run_process(broad_envi,main_p->pid,argv);
	printf("\nargv[0]%s with pid:%d",argv[0],broad_envi->pid);

	argv[0] = list_of_program.do_action;
	run_process(do_action,main_p->pid,argv);
	printf("\nargv[0]%s with pid:%d",argv[0],do_action->pid);

	argv[0]= list_of_program.olsrd;
	run_process(olsrd,main_p->pid,argv);
	printf("\nargv[0]%s with pid:%d",argv[0],olsrd->pid);

	argv[0]= list_of_program.qomet;
	//system(qomet);

	printf("\n....");
    
	wait_to_close();
}

int main()
{
	main_process_init();
	signal(SIGCHLD, handler);
    signal(SIGTERM,signal_handler_main);
	signal(SIGQUIT,signal_handler_main);
	signal(SIGKILL,signal_handler_main);
	signal(SIGINT,signal_handler_main);
	if(load_program_list("program_list.xml")== -1)
	{
		exit(0);
	}
	call_programs();
	printf("\nControl Program shut down now");
	return 0;
}



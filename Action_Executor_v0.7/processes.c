#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include "processes.h"


//-----------------Code of process function-------------------------//
process * create_process()
{
	process *NEW;
	NEW = (process *)malloc(sizeof(process));
	NEW->next = NULL;
	NEW->status = 0;
	NEW->pid= (pid_t)0;
	NEW->expire_time=0;
	return NEW;
}
void run_command(process *p,pid_t pgid, char** argv)
{
	pid_t pid;
	pid = getpid ();
	int i;
	//put process run in background
	i=open("/dev/null",O_RDWR); /* open stdin */
	dup2 (i, STDIN_FILENO);
	dup2 (i, STDOUT_FILENO);
	dup2 (i, STDERR_FILENO);
     umask(027);
      //if (pgid == 0) pgid = pid;
      setpgid (pid, pgid);
     /* Exec the new process.  Make sure we exit.  */ 
	signal (SIGINT, SIG_DFL);
     signal (SIGQUIT, SIG_DFL);
     signal (SIGTSTP, SIG_DFL);
     signal (SIGTTIN, SIG_DFL);
     signal (SIGTTOU, SIG_DFL);
	signal (SIGCHLD, SIG_DFL);
     execv(argv[0],argv);
     perror ("execvp");
     return;
}
void run_process(process *p,pid_t pgid,char **argv)
{
    pid_t pid = vfork ();
    if (pid == 0){
        /* This is the child process.  */
        run_command(p,pgid,argv);
	}
	else if (pid < 0)
        {
          /* The fork failed.  */
          perror ("fork");
          return;
        }
      else
        {
	      	p->pid = pid;
	      printf("\nprocess p: %d",(int)p->pid);
        }
	/* This is the parent process.  */
	//waitpid( pid, &(p->status), WNOHANG);
	
}
void kill_all_processes(pid_t pgid)
{
   int  killReturn = killpg( pgid, SIGKILL);  // Kill child process group
   if( killReturn == ESRCH)      // pid does not exist
   {
      printf("Group does not exist!\n");
   }
}
int process_is_alive(process *p)
{
	if (WIFSTOPPED (p->status))
		return 1;
	return 0;
}




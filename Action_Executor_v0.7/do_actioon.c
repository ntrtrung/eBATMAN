#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "processes.h"
#include "action_mapping.h"
#include "action_record.h"
#include "id_ip.h"
#include "config_files.h"

config_files cf;

#define SERVER_PORT 9911
#define MAX_DIF 0.001

char *kill_process = "/root/Read_Config/kill_process.sh";

typedef struct main_process
{
	pid_t pid;
	pid_t pgid;
	process *first_child_process;
	
}main_process;
static main_process *main_p;

void signal_handler_main(int sig)
{
	switch(sig) 
	{
		case SIGHUP:
			printf("\nhangup signal catched");
			break;
		case SIGTERM:
		case SIGKILL:
		case SIGINT:
			printf("\nterminate signal catched");
			kill_all_processes(main_p->pgid);
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
	signal(SIGTERM,signal_handler_main);
	signal(SIGQUIT,signal_handler_main);
	printf("\n Main process :%\n",main_p->pid);
}

void add_process_to_list(process *p)
{
	process *addnode;
	addnode = create_process();
	memcpy(addnode,p,sizeof(process));
	//printf("\n add process :%d\n",p->pid);
	if(main_p->first_child_process == NULL)
	{
		main_p->first_child_process = addnode;
	}
	else
	{
		process *temp;
		temp=main_p->first_child_process;
		while(temp->next!=NULL)
		{
			temp=temp->next;
		}
		temp->next=addnode;
	}
}
process* find_process(float time)
{
	process *temp;
	if(main_p->first_child_process == NULL) return NULL;
	temp = main_p->first_child_process;
	while(1)
	{
		if(temp->expire_time <= time) return temp;
		if(temp->next != NULL) temp = temp->next;
		else
			return NULL;
	}

	return NULL;
}
void print_processes()
{
	process *temp;
	temp=main_p->first_child_process;
	while(temp!=NULL)
	{
		printf("\nprocess is alive :%d",temp->pid); 
		temp=temp->next;
	}
}
void delete_process_in_list(process *p)
{
	printf("\ndelete_process_in_list\n");
	if(p->status == -1)
		return;
	process *temp;
	if(main_p->first_child_process == NULL) return ;

       temp = main_p->first_child_process;

	if(temp->pid == p->pid)
	{
	       main_p->first_child_process = p->next;
		//printf("\nfirst list - free process id:%d with status %d",p->pid,p->status);
		p->status=-1;
		if(p!=NULL)
	          free(p);
		print_processes();
		return;
	}
	if(p->next == NULL)
	{
        	//printf("\nend list - free process id:%d",p->pid);
		while(1)
		{
			if(temp->next->pid== p->pid)
				break;
			temp=temp->next;
		}
		temp->next=NULL;
		if(p!=NULL)
	          free(p);
		print_processes();
		return;
	}
	temp = p->next;
	p->next = temp->next;
	p->pid = temp->pid;
	p->status = temp->status;
	p->expire_time = temp->expire_time;
	temp->status = -1;
	if(temp!= NULL)
	  free(temp);
	//printf("\nmiddle list -free process id:%d",temp->pid);
	print_processes();
	
}

int kill_a_process(float time)
{
	process *temp;
	if(main_p->first_child_process == NULL) return -1;

	temp =find_process(time);
	if(temp!=NULL)
	{
		//kill all child processes using script file//
		char command[100];
		command[0] = '\0';
   		sprintf(command,"sh %s %d",kill_process,(int)(temp->pid));
		system(command);

		//kill bash process
		kill(temp->pid,9);
		printf("\nkill process id:%d with status %d",temp->pid,temp->status);
		delete_process_in_list(temp);
		print_processes();
		return 1;
	}
	return -1;
}


int time_is_equal(float A,float B)
{
	float temp;
	temp=B-A;
	if(temp < 0) temp = -temp;
	if(temp <= MAX_DIF)
	  return 0;
	return 1;
}
void wait_to_close()
{
      int status;
	waitpid (WAIT_ANY,&status, WUNTRACED);
}

//receive the time from QOMET and do actions
//ActionRecord array must be orderred
void do_actions(int accept_sd,ActionRecord *ac,int ac_count, action_mapping *am, id_ip *p)
{
	printf("\n in do action\n");
	float buffer ;
        buffer =0;
	int index_ac =0;
	int i =0;
	int rc;
	char *argv[4];
	argv[0] = "/usr/local/bin/bash";
	for(i=0;i<ac_count;i++)
		PrintAction(&ac[i]);
	while(1)
	{
		/* Receive a message from the client */
		//printf("\nI am waiting client(s) to send message(s) to me...\n");
		rc = recv(accept_sd,&buffer, sizeof(float), 0);
		//printf("\n number byte rec %d\n",rc);
		//printf("\n time %f\n",buffer);
		if(rc <= 0)
		{	
			perror("TCP_QO_ACTION server - recv() error");
			goto bad_do_action;
		}
		else
		{
				//if there is any action in this time, run it
				if(index_ac > ac_count)
					continue;
				while(ac[index_ac].time == buffer)
				{		
					printf("\ncall action\n"); 	
					printf("\n time %f\n",buffer);
	  				PrintAction(&ac[index_ac]);
	    			       //create a process for start an action
					process *pr;
					//printf("\ncreate a process");
					pr = create_process();
					if(pr == NULL)
					{
						printf("\nerror in create a process");
						return;
					}
					pr->expire_time = ac[index_ac].time + ac[index_ac].duration;
					//build argument for action
								
					argv[1] =  look_up_action_file(am,ac[index_ac].id);
					if(argv[1] == NULL)
					{
						printf("\n cannot find action file of action id:%d",ac[index_ac].id);
						//goto bad_do_action;
						index_ac++;	
						continue;
					}
					printf("\naction file:%s",argv[1]);

					if (ac[index_ac].target != -1)
					{
						argv[2] = (char *)look_up_ip(p,ac[index_ac].target);
						if(argv[2] == NULL)
						{
							printf("\n cannot find ip adress of target id:%d",ac[index_ac].target);
							index_ac++;	
						       continue;
						}
						argv[3] =(char*)0;
					}
					else
						argv[2] = (char*)0;
					
					run_process(pr,main_p->pgid,argv);	
					index_ac++;	

					if(index_ac > ac_count)
						break;
					//printf("\nindex :%d and ac[index_ac].time:%f",index_ac,ac[index_ac].time);					     					
					//add the process into process list
					add_process_to_list(pr);
					//print_processes();	
				
				}
				//if there is any processes that needs close because of deadline, close them
				if(main_p->first_child_process != NULL)
					while(find_process(buffer)!=NULL)
					{
						printf("\nkill a process with time:%d\n",buffer);
						kill_a_process(buffer);
					}

		}
		//if the value is -1 , the program will be terminated
		if(buffer == (float)-1)
		{
			index_ac =0;
		}


	}		
	
bad_do_action:
	return ;
}

int main()
{
	if(read_config_files(&cf)== -1)
	{
		printf("\nError in reading config files");
		exit(0);
	}
//---declare variable of program----//
	
	int listen_sd, accept_sd;
	struct sockaddr_in addr;  
	int i;
 
//--------------load action record from file-----------------------//
	int numberofac;
	char *filename = cf.action_record;
	ActionRecord *ac;
	ac = ReadOutputFile(filename,&numberofac);
	if(ac == NULL)
	{
		printf("\nCannot read action record file");
		return 0;

	}
	//sort the array with the action's starting time
	SortActionArray(ac,numberofac);
	printf("\nnumber of action record:%d",numberofac);

	
//-------------load action mapping file------------------------//
	char *action_mapping_file = cf.action_mapping;
	action_mapping *ap ;
	int numberofap =0;
	ap = read_action_mapping_file(action_mapping_file,&numberofap);
	if(ap == NULL)
	{
		printf("\nCannot read action mapping file");
		return 0;

	}
	action_mapping *temp;
	temp = ap;
	while(temp!=NULL)
	{
		print_action_mapping(temp);
		temp=temp->next;
	}

		
	
	

//-------------------------------load setting file ---------------//

	char *setting_file = cf.setting_file;
	id_ip *ID_IP;
	int numberofnode;
	ID_IP =  read_setting_file(setting_file,&numberofnode);
	if(ID_IP == NULL)
	{
		printf("\nCannot read setting file");
		return 0;
	}
	printf("\nnumber of node in setting file:%d\n",numberofnode);
	

//------------INIT main process to control child processes----------------//

	main_process_init();

//--------------INIT server for listening the time value from QOMET-------//
    /* If an argument was specified, use it to */
    /* control the number of incoming connections */
     listen_sd = InitTCPServer(SERVER_PORT);
	if(listen_sd < 1)
	{
		fprintf(stderr,"Server cannot init");
		return 0;
	}

//--------------------------main loop----------------------//
	
	do_actions(listen_sd,ac,numberofac,ap,ID_IP);

//------------------------close program--------------------//
    /* Close the listen socket */
    close(listen_sd);
	//kill all child processes//
	kill_all_processes(main_p->pgid);
	//wait for close main process//
	wait_to_close();
    return 0;
//--------------close program in bad cases-----------------------//
bad_main:
	printf("\nclose do action program\n");
	close(listen_sd);
	close(accept_sd);
	kill_all_processes(main_p->pgid);
	exit(-1);
}

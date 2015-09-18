#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/if_ether.h> 
#include <net/ethernet.h>
#include <net/if.h> 
#include <net/if_dl.h> 
#include <net/if_types.h>
#include <pthread.h>

#include "packet_queue.h"
#include "routing.h"
#include "read_config.h"
#include "network.h"
#include "global.h"
#include "read_ip_to_ip.h"
#include "id_ip.h"
#include "neighbor_table.h"
#include "config_files.h"
#include "ipfw_control.h"

#define BROAD_ENVI_SERVER_PORT 9912
#define UDP_SERVER_PORT 9930
#define NUMBER_SEND_THREAD 4
#define MAX_NEIGHBOR_FOR_UNICAST 5
queue *q;
queue *q_for_receiver;

char *server_interface;
struct in_addr ip_normal_server;
struct in_addr broad_ip_normal_server;

int client_socket;
int normal_server_socket;
int broad_normal_server_socket;

int server_port = UDP_SERVER_PORT;
int tcp_server_port = BROAD_ENVI_SERVER_PORT;

struct sockaddr_in client_sock;
pthread_mutex_t rt_access_mutex     = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_for_queue = PTHREAD_MUTEX_INITIALIZER;;
pthread_cond_t condition_cond_for_queue  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t condition_mutex_for_queue = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t queue_for_receiver_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_cond_for_queue_for_receiver  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t condition_mutex_for_queue_for_receiver = PTHREAD_MUTEX_INITIALIZER;

int turn_off = 0;
be_configuration  be;
config_files cf;
ip_to_ip *ip_to_ip_table;
id_ip *id_ip_table;

char **neighbor_table;
int number_of_time_record;
int number_of_node;
int table_index;
unsigned long *ip_address_table_one ;
unsigned long *ip_address_table_two ;
int *realtime_routing_table;


int node_id;
int tap_fd;

char *TAP_MAC;

pthread_mutex_t fakeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fakeCond = PTHREAD_COND_INITIALIZER;

void thread_wait(int timeInSec)
{
	struct timespec timeToWait;
	struct timeval now;
	int rt;

	gettimeofday(&now,NULL);

	timeToWait.tv_sec = now.tv_sec + timeInSec;
	timeToWait.tv_nsec = now.tv_usec*1000;

	pthread_mutex_lock(&fakeMutex);
	rt = pthread_cond_timedwait(&fakeCond, &fakeMutex, &timeToWait);
	pthread_mutex_unlock(&fakeMutex);

}
void *delete_neighbor_rt_entry()
{
	int i;
	char cmd[50]; 
	while(turn_off == 0)
	{
		thread_wait(2);
		delete_routing_entry(neighbor_table,table_index,be.experiment_interface,ip_address_table_one,number_of_node);
	}
}

int read_config()
{
	if(read_config_files(&cf)== -1)
	{
		printf("\nCannot load config_files.xml");
		exit(0);
	}	

    if(read_config_file(&be,cf.be_config)== -1)
	{
		printf("\nCannot load be_config.xml");
		exit(0);
	}
	int count;
	ip_to_ip_table = read_ip_to_ip_file(cf.ip_to_ip,&count);
	if(ip_to_ip_table == NULL)
	{
		printf("\nCannot load %s",cf.ip_to_ip);
		exit(0);
	}
	id_ip_table = read_setting_file(cf.setting_file,&count);
	
	if(id_ip_table == NULL)
	{
		printf("\nCannot read setting file");
		exit(0);
	}
	printf("\nnumber of node in setting file:%d\n",count);
	return count;
}

void init_global_variables()
{
	//loading preconfigured files	
	int count;
	count=read_config();

	//create packet queue
	q = create_queue();
	q_for_receiver = create_queue();
	
	printf("\nExperiment interface:%s",be.experiment_interface);
	printf("\nControl interface:%s",be.control_interface);
       
	//get ip of UDP server	
	ip_normal_server.s_addr = get_ip_of_interface(be.control_interface);
	if(ip_normal_server.s_addr == 0)
		die("Cannot get ip of interface");
	else
		printf("\nNormal ip:%s",inet_ntoa(ip_normal_server));
		
    //get broadcast ip of broadcast UDP server	 
	broad_ip_normal_server.s_addr = get_broadcast_ip_of_interface(be.control_interface);
	if(broad_ip_normal_server.s_addr == 0)
		die("Cannot get broadcast ip of interface");
	else
		printf("\nBroadcast normal ip:%s",inet_ntoa(ip_normal_server));
		
	//get ip for TAP interface
       struct in_addr ip_experiment_inferface ;
	ip_experiment_inferface.s_addr = look_up_ip_one(ip_to_ip_table,&ip_normal_server);

	//get node id 
	node_id = look_up_id(id_ip_table,(char*)inet_ntoa(ip_experiment_inferface));
	if(node_id == -1)
		die("cannot find node id in setting file");
	printf("\nNode ID:%d",node_id);

	// init TAP device
       tap_fd = init_tun(be.experiment_interface,(char*)inet_ntoa(ip_experiment_inferface),"255.255.255.0");
	if(tap_fd >0)
		printf("\nTAP socket is ready");

	//init UDP client
	client_socket = init_udp_client();
       if(client_socket<0)
		die("Cannot init client");
	else
		printf("\nUDP client is ready");

	//init UDP server
	normal_server_socket = init_udp_server( (char*)inet_ntoa(ip_normal_server),server_port);
	if(normal_server_socket <=0 )
		die("Cannot init server");
	else
		printf("\nUDP server is ready");
	
	//init broadcast UDP server
	broad_normal_server_socket = init_udp_server((char*)inet_ntoa(broad_ip_normal_server),server_port);
	if(broad_normal_server_socket <=0 )
		die("Cannot init server");
	else
		printf("\nBroadcast UDP server is ready");

	//load records from record table file
	neighbor_table =  load_neighbor_record_table_file(cf.neighbor_table_bin,&number_of_time_record,&number_of_node,node_id);
	if(neighbor_table == NULL)
	{
		printf("\nCannot read neighbor record binary file");
		exit(0);
	}
	number_of_node--;
	
	int i = 0;
	if(number_of_node > count || number_of_node == 0)
	{
		printf("\nnumber of nodes in neighbor table are more than ones in setting file");
		exit(0);
	}
	
	//init two ip tables
	ip_address_table_one = (unsigned long *)malloc(number_of_node*sizeof(unsigned long));
	ip_address_table_two = (unsigned long *)malloc(number_of_node*sizeof(unsigned long));
	ip_to_ip *temp;
	temp = ip_to_ip_table;
	while(temp!=NULL)
	{
		ip_address_table_one[i] = temp->ip_address_one;
		ip_address_table_two[i] = temp->ip_address_two;
		temp=temp->next;
		i++;
	}
	
	//get mac address of tap interface
	TAP_MAC = get_mac_of(be.experiment_interface);
	if(TAP_MAC == NULL)
	{
		printf("\n Cannot get mac address of TAP interface");
		exit(0);
	}
	
}
void ipfw_controller()
{
	int i;
	struct in_addr ip_dest;
	for(i =0;i < number_of_node;i++)
	{
		//if(neighbor_table[table_index][i] == 1)
		//	printf("\n[%d][%d]  - [%d][%d]:%d",table_index,i,table_index -1,i,neighbor_table[table_index-1][i]);
		if(neighbor_table[table_index][i] == 1 && neighbor_table[table_index-1][i] == -1)
		{	
			printf("\n add rule for %d",table_index);
			ip_dest.s_addr = ip_address_table_two[i];
			add_rule(MINIMUM_RULE + i,(char*)inet_ntoa(ip_dest),UDP_SERVER_PORT,be.control_interface);
			continue;
		}
		if(neighbor_table[table_index][i] == -1 && neighbor_table[table_index-1][i] == 1)
		{
			printf("\n delete rule for %d",table_index);
			delete_rule(MINIMUM_RULE + i);
		}
	}
	
}

void *TCP_server_thread()
{
	printf("\nFor Qomet server thread");
	int listen_sd;
	struct sockaddr_in addr;  
	 listen_sd = InitTCPServer(tcp_server_port);
	 table_index =0;
	if(listen_sd < 1)
	{
		fprintf(stderr,"Server cannot init");
		return 0;
	}
    /* Go through the loop once for each connection */
    /* Wait for an incoming connection */
	float buffer ;
       buffer =0;
       int rc;
	//set ipfw globle rule
	add_global_rule((char*)inet_ntoa(ip_normal_server),be.control_interface,UDP_SERVER_PORT);
	//set rule for begining
	int i;
	char * ip_dest_s = (char*)malloc(30);
	struct in_addr ip_dest;
	for(i =0;i < number_of_node;i++)
	{
		if(neighbor_table[table_index][i] == 1)
		{
			ip_dest.s_addr = ip_address_table_two[i];
			ip_dest_s = (char*)inet_ntoa(ip_dest);
			add_rule(MINIMUM_RULE + i,ip_dest_s,UDP_SERVER_PORT,be.control_interface);
		}
	}	
	//receive time from server
	while(turn_off==0)
	{
		rc = recv(listen_sd,&buffer, sizeof(float), 0);
		printf("\n time %f\n",buffer);
		
		if(rc <= 0)
		{	
			perror("\nTCP server - recv() error");
			goto bad_tcp_server;
		}
		else
		{
				//if the value is -1 , the program will be terminated
				if(buffer == (float)-1)
				{
					table_index=0;
					clear_all_rules();		
					continue;
				}
				if(buffer == (float)0)
				{
					table_index=0;
					continue;
				}
				if(table_index < number_of_time_record){
					table_index++;			
					ipfw_controller();
				}
									
		}
		
	}		
	
	close(listen_sd);
	printf("\nTCP server close");
bad_tcp_server:
	printf("\nTCP server die");
	close(listen_sd);
	exit(0);
	return;	 
}


void *run_tap_cap()
{
	char packet[MAX_PACKET_SIZE];
	int l;
	while(turn_off==0)
	{
		if(tap_fd <= 0)
			break;
		 l = read(tap_fd,packet,MAX_PACKET_SIZE);
		 if(l <0)
		    continue;
		 pthread_mutex_lock(&mutex_for_queue);
		 enqueue(q,packet,l,TICKET_BROADCAST);
		 pthread_mutex_unlock(&mutex_for_queue);	
		 pthread_cond_signal( &condition_cond_for_queue);		 
	}
}

void *run_normal_server()
{
	printf("\nRun normal server thread");
	char packet[MAX_PACKET_SIZE];
	memset(packet,0,MAX_PACKET_SIZE);
       struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	int number_byte;
	char * mac_des = (char *)malloc(sizeof(char)*6);
	int is_promisc =0;
	int all_promisc =0;
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd <= 0)
	{
		printf("Cannot run normal server");
		exit(0);
	}
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name,be.experiment_interface, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	int flags;
	flags = (ifr.ifr_flags & 0xffff);
	printf("\nflags %x",flags);
       if(flags == PPROMISC){
		all_promisc =1;
		printf("\nInterface has been in promiscuous mode\n");
	}
	while(turn_off==0)
	{
		number_byte = recvfrom(normal_server_socket,packet,MAX_PACKET_SIZE,0, (struct sockaddr*)&addr, &len);
		if(len <= 0)
		{
			continue;
		}
		if(all_promisc == 1)
		{
			write(tap_fd,packet,number_byte);	
			continue;			
		}
		//checking for whether interface is in promiscuous mode or not
		ioctl(fd, SIOCGIFFLAGS, &ifr);
		flags = (ifr.ifr_flags & 0xffff);
		if(flags == PPROMISC)
		{	
			write(tap_fd,packet,number_byte);	
			continue;			
		}
		mac_des = (char*)get_mac_dest(packet);
		if(is_mac_equal(mac_des,TAP_MAC) == 1)
		{
				write(tap_fd,packet,number_byte);	
		}
			
	}
	close(normal_server_socket);
}
void *run_broad_normal_server()
{
	printf("\nRun broadcast normal server thread");
	char packet[MAX_PACKET_SIZE];
	memset(packet,0,MAX_PACKET_SIZE);
    struct sockaddr_in addr;
	
	int len = sizeof(struct sockaddr_in);
	int number_byte;
	char * mac_des = (char *)malloc(sizeof(char)*6);
	int is_promisc =0;
	int all_promisc =0;
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd <= 0)
	{
		printf("Cannot run broadcast normal server");
		exit(0);
	}
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name,be.experiment_interface, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	int flags;
	flags = (ifr.ifr_flags & 0xffff);
	printf("\nflags %x",flags);
       if(flags == PPROMISC){
		all_promisc =1;
		printf("\nInterface has been in promiscuous mode\n");
	}
	while(turn_off==0)
	{
		number_byte = recvfrom(broad_normal_server_socket,packet,MAX_PACKET_SIZE,0, (struct sockaddr*)&addr, &len);
		if(len <= 0)
		{
			continue;
		}
		if(all_promisc == 1)
		{
			pthread_mutex_lock(&queue_for_receiver_mutex);
			enqueue(q_for_receiver,packet,number_byte,TICKET_BROADCAST);
			pthread_mutex_unlock(&queue_for_receiver_mutex);
			pthread_cond_signal( &condition_cond_for_queue_for_receiver);
			continue;			
		}
		//checking for whether interface is in promiscuous mode or not
		ioctl(fd, SIOCGIFFLAGS, &ifr);
		flags = (ifr.ifr_flags & 0xffff);
		if(flags == PPROMISC)
		{	
			//write(tap_fd,packet,number_byte);	
			pthread_mutex_lock(&queue_for_receiver_mutex);
			enqueue(q_for_receiver,packet,number_byte,TICKET_BROADCAST);
			pthread_mutex_unlock(&queue_for_receiver_mutex);
			pthread_cond_signal( &condition_cond_for_queue_for_receiver);
			continue;			
		}
		mac_des = (char*)get_mac_dest(packet);
		if(is_mac_equal(mac_des,TAP_MAC) == 1)
		{
			pthread_mutex_lock(&queue_for_receiver_mutex);
			enqueue(q_for_receiver,packet,number_byte,TICKET_BROADCAST);
			pthread_mutex_unlock(&queue_for_receiver_mutex);
			pthread_cond_signal( &condition_cond_for_queue_for_receiver);
			//write(tap_fd,packet,number_byte);		
		}
	}
	close(normal_server_socket);
}
void *send_packet_to_tap()
{
    printf("\nRun Send packet to tap ");
    char *temp = (char*)malloc(1);
	int type;
	int length;
	while(turn_off==0)
	{	
		pthread_mutex_lock(&condition_mutex_for_queue_for_receiver);
		if(is_queue_empty(q_for_receiver) == 0)
		{
			pthread_cond_wait(&condition_cond_for_queue_for_receiver,&condition_mutex_for_queue_for_receiver);
		}
		
		pthread_mutex_lock(&queue_for_receiver_mutex);
		temp = dequeue(q_for_receiver,&type,&length);
		pthread_mutex_unlock(&queue_for_receiver_mutex);
		if(temp == NULL)
		{
			continue;
		}
		write(tap_fd,temp,length);		
		//sendto(client_socket,temp,length, 0,(struct sockaddr *)&si_broad,slen);
	}
}
void *send_packet_out( )
{
	printf("\nRun Send packet out ");
	struct sockaddr_in si_other;
	int slen=sizeof(struct sockaddr_in);
	 memset((char *)&si_other, 0, slen);
	 si_other.sin_family = AF_INET;
	 si_other.sin_port = htons(server_port);
   //for broadcast
   	struct sockaddr_in si_broad;
	//slen=sizeof(struct sockaddr_in);
	memset((char *)&si_broad, 0, slen);
	 si_broad.sin_family = AF_INET;
	 si_broad.sin_port = htons(server_port);
	 si_broad.sin_addr.s_addr = broad_ip_normal_server.s_addr;
	 
	 
	int i,j;
	char *temp = (char*)malloc(1);
	int type;
	int length;
	while(turn_off==0)
	{	
		pthread_mutex_lock(&condition_mutex_for_queue);
		if(is_queue_empty(q) == 0)
		{
			pthread_cond_wait(&condition_cond_for_queue,&condition_mutex_for_queue);
		}
		
		pthread_mutex_lock(&mutex_for_queue);
		temp = dequeue(q,&type,&length);
		pthread_mutex_unlock(&mutex_for_queue);
		if(temp == NULL)
		{
			continue;
		}
		if(neighbor_table[table_index][number_of_node] < MAX_NEIGHBOR_FOR_UNICAST)
		{
			for(i =0;i < number_of_node;i++)
			{

				if(neighbor_table[table_index][i] == 1)
				{
					si_other.sin_addr.s_addr = ip_address_table_two[i];
					sendto(client_socket,temp,length, 0,(struct sockaddr *)&si_other,slen);
				}
			}
		}
		else
			sendto(client_socket,temp,length, 0,(struct sockaddr *)&si_broad,slen);
	}
}

void signal_handler_main(int sig)
{
	switch(sig) 
	{
		case SIGHUP:
			printf("\nhangup signal catched\n");
			turn_off =1;
			close(client_socket);
			close(tap_fd);
			clear_all_rules();
			printf("\nSHUT DOWN\n");
			exit(0);
			break;
		case SIGTERM:
		case SIGKILL:
		case SIGINT:
			printf("\nterminate signal catched\n");
			turn_off =1;
			close(client_socket);
			close(tap_fd);
			clear_all_rules();
			printf("\nSHUT DOWN\n");
			exit(0);
			break;
	}
}
 main()
{
   init_global_variables(); 
  
  signal(SIGTERM,signal_handler_main);
  signal(SIGKILL,signal_handler_main);
  signal(SIGINT,signal_handler_main);
  signal(SIGHUP,signal_handler_main);
  
  pthread_t t_normal_server,t_run_libpcap,t_tap_cap,t_tcp_server,t_broad_normal_server,t_delete_route_thread;
  pthread_t t_send_packet[NUMBER_SEND_THREAD],t_send_packet_to_tap[NUMBER_SEND_THREAD];

  

  pthread_create(&t_tcp_server, NULL, &TCP_server_thread, NULL);
  pthread_create(&t_normal_server, NULL, &run_normal_server, NULL);
  pthread_create(&t_broad_normal_server, NULL, &run_broad_normal_server, NULL);
  pthread_create(&t_normal_server, NULL, &run_tap_cap, NULL);
  pthread_create(&t_delete_route_thread, NULL, &delete_neighbor_rt_entry, NULL);
  int i;
  for(i=0;i<NUMBER_SEND_THREAD;i++){
 	 pthread_create(&t_send_packet[i], NULL, &send_packet_out,NULL);
     pthread_create(&t_send_packet_to_tap[i],NULL,send_packet_to_tap,NULL);
  }


  pthread_join(t_tcp_server, NULL);
  pthread_join(t_normal_server, NULL);
  pthread_join(t_broad_normal_server, NULL);
  pthread_join(t_tap_cap, NULL);    
  pthread_join(t_delete_route_thread, NULL);    
  printf("\n");
  exit(0);
}

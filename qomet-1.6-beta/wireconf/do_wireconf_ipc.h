#include 	"global.h"
#include	<unistd.h>
#include 	<sys/ipc.h>
#include 	<sys/shm.h>
#include 	<sys/types.h>
#include	<stdlib.h>
#include	<stdio.h>
#include 	<errno.h>
#include 	<netinet/in.h>
#include 	<arpa/inet.h>
#include	<sys/socket.h>
#include 	<fcntl.h>
#include 	<sys/stat.h>
#include	<time.h>

#include	"message.h"

#define	SVSHM_MODE	(SHM_R | SHM_W | SHM_R>>3 | SHM_R>>6)
#define IPC_LINK_NAME		"/tmp/wireconf_link_ipc.shm"
#define IPC_CHANNEL_NAME	"/tmp/wireconf_channel_ipc.shm"
#define NUMBER_OF_CHANNELS	1
#define CHANNEL_ID		0
#define PRE_CH_UTIL		0.0


typedef struct
{
  in_addr_t ip_addr;
  int op_rate;
} qomet_link_info;

typedef struct
{
  int channel_id;
  float ch_util;
} qomet_channel_info;


void *ipc_locate (char *ipc_name, int mem_size);

int
ipc_link_write (qomet_link_info * ptr, int id, char *ip_char, int op_rate);

int ipc_channel_write (qomet_channel_info * ptr, int id, float ch_util);

int ipc_remove (char *ipc_name);

#if 1
void ipc_link_init (qomet_link_info * ptr, int mem_size_unit);

void ipc_channel_init (qomet_channel_info * ptr, int mem_size_unit);

int ipc_create_file ();

int ipc_remove_file ();

void ipc_time (char *s_time);
#endif

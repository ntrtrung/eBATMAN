#include "do_wireconf_ipc.h"
#include "global.h"

#define PERMS 0744		/* RW for owner, group, others */

void *
ipc_locate (char *ipc_name, int mem_size)
{
  int oflag = SVSHM_MODE | IPC_CREAT;
  //int oflag = 0666 | IPC_CREAT | IPC_EXCL;
  void *ptr;
  key_t key;
  int id;

  if ((key = ftok (ipc_name, 0)) == -1)
    {
      fprintf (stderr,
	       "WARNING: do_wireconf:ipc_locate:ftok:can not get key \
from path %s\n", ipc_name);
      return NULL;
    }

  if ((id = shmget (key, mem_size, oflag)) == -1)
    {
      if (errno == EEXIST)
	{
	  fprintf (stderr, "WARNING: wireconf:ipc_locate:shmget: IPC name \
existed, try to remove and create new one\n");
	  if (ipc_remove (ipc_name) == -1)
	    {
	      fprintf (stderr, "WARNING: do_wireconf:ipc_locate:shmget: \
can not remove the existing \n");
	      return NULL;
	    }
	  else if ((id = shmget (key, mem_size, oflag)) == -1)
	    {
	      fprintf (stderr, "WARNING: do_wireconf:shmget: can not locate \
the memory\n");
	      return NULL;
	    }
	}
      else
	{
	  fprintf (stderr, "WARNING: do_wireconf:ipc_locate:shmget: \
can not locate the memory\n");
	  return NULL;
	}
    }

  if ((ptr = shmat (id, NULL, 0)) == (void *) -1)
    {
      fprintf (stderr, "WARNING: do_wireconf:shmat: can not get back \
pointer to the memory\n");
      return NULL;
    }

  return ptr;
}

int
ipc_link_write (qomet_link_info * ptr, int id, char *ip_char, int op_rate)
{
  in_addr_t ip;

  //printf("ipc_link_write: receive this address %s\n",ip_char);        
  //if ( (ip=inet_addr(ip_char) != ERROR) )
  if (inet_pton (AF_INET, ip_char, &ip) != -1)
    {
      ptr = ptr + id;
      ptr->ip_addr = ip;
      ptr->op_rate = op_rate;

      //char *s;
      //inet_ntop(AF_INET, &ip, s, sizeof(struct sockaddr_in)); 
      //printf("ipc_link_write: Convert the IP back %s\n",s);
      return 0;
    }
  else
    return ERROR;
}


int
ipc_channel_write (qomet_channel_info * ptr, int id, float ch_util)
{
  ptr = ptr + id;
  ptr->channel_id = id;
  ptr->ch_util = ch_util;
  return 0;
}

int
ipc_remove (char *ipc_name)
{
  int id;
  int r;
  if ((id = shmget (ftok (ipc_name, 0), 0, SVSHM_MODE)) == -1)
    {
      fprintf (stderr, "WARNING: ipc_remove: could not get id for name %s\n",
	       ipc_name);
      r = ERROR;
    }
  if (shmctl (id, IPC_RMID, NULL) == 0)
    r = SUCCESS;
  else
    r = ERROR;

  return r;
}

#if 1
void
ipc_link_init (qomet_link_info * ptr, int mem_size_unit)
{
  qomet_link_info *walker;
  int i;

  walker = ptr;
  for (i = 0; i < mem_size_unit; i++)
    {
      walker->ip_addr = (in_addr_t) 0;
      walker->op_rate = (int) -1;
      walker++;
    }
  return;
}

void
ipc_channel_init (qomet_channel_info * ptr, int mem_size_unit)
{
  qomet_channel_info *walker;
  int i;

  walker = ptr;
  for (i = 0; i < mem_size_unit; i++)
    {
      walker->channel_id = (int) -1;
      walker->ch_util = (float) -1.0;
      walker++;
    }
  return;
}

int
ipc_create_file ()
{
  int r;
  // @Lan on Apr. 8: should create file@tmp here
  // file for link information
  if (creat (IPC_LINK_NAME, PERMS) == -1)
    {
      fprintf (stderr, "WARNING: do_wireconf: can't create %s, mode %03o for \
shared memory\n", IPC_LINK_NAME, PERMS);
      r = ERROR;
    }
  else
    fprintf (stderr, "Create the file %s successfully\n", IPC_LINK_NAME);

  // file for channel information
  if (creat (IPC_CHANNEL_NAME, PERMS) == -1)
    {
      fprintf (stderr, "WARNING: do_wireconf: can't create %s, mode %03o for \
shared memory\n", IPC_CHANNEL_NAME, PERMS);
      r = ERROR;
    }
  else
    fprintf (stderr, "Create the file %s successfully\n", IPC_CHANNEL_NAME);

  return r;
  // << @Lan on Apr. 8: should create file@tmp here 
}

int
ipc_remove_file ()
{
  int r;
  // added by Lan on Apr 8, try to delete/remove the file@tmp here
  //file for link information
  if (unlink (IPC_LINK_NAME) == -1)
    {
      fprintf (stderr, "WARNING: can't remove %s\n", IPC_LINK_NAME);
      r = ERROR;
    }
  else
    fprintf (stderr, "Remove the file %s successfully\n", IPC_LINK_NAME);


  //file for channel information
  if (unlink (IPC_CHANNEL_NAME) == -1)
    {
      fprintf (stderr, "WARNING: can't remove %s\n", IPC_CHANNEL_NAME);
      r = ERROR;
    }
  else
    fprintf (stderr, "Remove the file %s successfully\n", IPC_CHANNEL_NAME);

  return r;
  // << added by Lan on Apr 8, try to delete/remove the file@tmp here
}
#endif


void
ipc_time (char *s_time)
{
  struct tm *current;
  time_t now;

  time (&now);
  current = localtime (&now);

  sprintf (s_time, "%i/%i %i:%i:%i", current->tm_mon, current->tm_mday,
	   current->tm_hour, current->tm_min, current->tm_sec);
}

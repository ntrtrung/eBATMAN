
/*
 * Copyright (c) 2006-2010 The StarBED Project  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/************************************************************************
 *
 * QOMET Emulator Implementation
 *
 * File name: wireconf.c
 * Function: Main source file of the wired-network emulator 
 *           configurator library wireconf;
 *           at the moment it can be used with dummynet on FreeBSD 
 *
 *
 * Authors: Junya Nakata, Razvan Beuran
 *
 ***********************************************************************/

#include <sys/queue.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip_fw.h>
#include <netinet/ip_dummynet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "global.h"
#include "wireconf.h"
#include "message.h"
#include "statistics.h"


/////////////////////////////////////////////
// Special defines
/////////////////////////////////////////////

// this define is only used in conjunction with the new modified
// version of dummynet that is still in experimental phase 

//#define NEW_DUMMYNET #(version 6??)


///////////////////////////////////////////////
// Special functions
///////////////////////////////////////////////

// handle uint64 alignment (from ip_fw2.c)
static __inline uint64_t
align_uint64 (uint64_t * pll)
{
  uint64_t ret;

  bcopy (pll, &ret, sizeof (ret));
  return ret;
}


///////////////////////////////////////////////
// IPv4 address manipulation
///////////////////////////////////////////////

// convert a string containing an IPv4 address 
// to an IPv4 data structure;
// return SUCCESS on success, ERROR on error
static int
atoaddr (char *array_address, in4_addr * ipv4_address, uint16_t * port)
{
  int c, o;

  c = 0;
  o = 0;
  ipv4_address->word = 0;
  *port = 0;

  for (;;)
    {
      if (array_address[c] == '.')
	{
	  c++, o++;
	  continue;
	}
      else if (isdigit (array_address[c]) == 0)
	break;

      ipv4_address->octet[o] = ipv4_address->octet[o] * 10
	+ array_address[c] - '0';
      c++;
    }

  if (array_address[c] == '\0')
    return SUCCESS;
  else if ((o != 3) || (array_address[c] != ':'))
    return ERROR;
  c++;

  for (;;)
    {
      if (isdigit (array_address[c]) == 0)
	break;
      *port = *port * 10 + array_address[c] - '0';
      c++;
    }
  return SUCCESS;
}


/////////////////////////////////////////////
// Socket manipulation functions
/////////////////////////////////////////////

// get a socket identifier;
// return socket identifier on success, ERROR on error
int
get_socket (void)
{
  int socket_id;

  if ((socket_id = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
      WARNING ("Error creating socket");
      perror ("socket");
      return ERROR;
    }

  return socket_id;
}

// close socket specified by socket_id
void
close_socket (int socket_id)
{
  close (socket_id);
}

// get socket options related to ipfw commands for 'socket_id';
// options are specified by 'option_name', 'option_value' and
// a pointer to 'option_length';
// return SUCCESS on success, ERROR on error
static int
get_socket_options (int socket_id, int option_name, void *option_value,
		    socklen_t * option_length)
{

  // choose the action corresponding to the given option name
  switch (option_name)
    {

      //////////////////// RULE COMMANDS /////////////////////////////

      // add a rule to the firewall (the rule contains dummynet pipes);
      // 'option_value' should be of type "struct ip_fw*"
    case IP_FW_ADD:
#ifdef MESSAGE_DEBUG
      DEBUG ("Add ipfw rule: ");
      print_rule ((struct ip_fw *) option_value);
#endif

      if (getsockopt (socket_id, IPPROTO_IP, option_name,
		      (struct ip_fw *) option_value, option_length) < 0)
	{
	  WARNING ("Error getting socket options");
	  perror ("getsockopt");
	  return ERROR;
	}
      break;

      // get all firewall rules;
      // 'option_value' should be of type "struct ip_fw *"
    case IP_FW_GET:
      DEBUG ("Get ipfw rules");
      if (getsockopt (socket_id, IPPROTO_IP, option_name,
		      (struct ip_fw *) option_value, option_length) < 0)
	{
	  WARNING ("Error getting socket options");
	  perror ("getsockopt");
	  return ERROR;
	}
      DEBUG ("Got ipfw rules: data=%p data_size=%d",
	     (struct ip_fw *) option_value, *option_length);
      break;
      break;

    default:
      WARNING ("Unrecognized socket option_name: %d", option_name);
      return ERROR;
    }

  return SUCCESS;
}

// set socket options related to ipfw/dummynet commands for 'socket_id';
// options are specified by 'option_name', 'option_value' and
// 'option_length';
// return SUCCESS on success, ERROR on error
static int
set_socket_options (int socket_id, int option_name, void *option_value,
		    socklen_t option_length)
{

  // choose the action corresponding to the given option_name
  switch (option_name)
    {

      //////////////////// RULE COMMANDS /////////////////////////////

      // delete an ipfw rule containing dummynet pipes;
      // 'option_value' should be of type "u_int32_t *"
    case IP_FW_DEL:
      DEBUG ("Delete ipfw rule #%d", (*(u_int32_t *) option_value));

      if (setsockopt (socket_id, IPPROTO_IP, option_name,
		      (u_int32_t *) option_value, option_length) < 0)
	{
	  WARNING ("Error setting socket options");
	  perror ("setsockopt");
	  return ERROR;
	}
      break;


      //////////////////// PIPE COMMANDS /////////////////////////////

      // configure a dummynet pipe;
      // 'option_value' should be of type "struct dn_pipe *"
    case IP_DUMMYNET_CONFIGURE:
      DEBUG ("Configure ipfw dummynet pipe");

      if (setsockopt (socket_id, IPPROTO_IP, option_name,
		      (struct dn_pipe *) option_value, option_length) < 0)
	{
	  WARNING ("Error setting socket options");
	  perror ("setsockopt");
	  return ERROR;
	}
#ifdef MESSAGE_DEBUG
      DEBUG ("ipfw dummynet pipe configured: ");
      print_pipe ((struct dn_pipe *) option_value);
#endif
      break;

      // delete a dummynet pipe;
      // 'option_value' should be of type "struct dn_pipe *"
    case IP_DUMMYNET_DEL:
#ifdef MESSAGE_DEBUG
      DEBUG ("Delete ipfw dummynet pipe: ");
      print_pipe ((struct dn_pipe *) option_value);
#endif

      if (setsockopt (socket_id, IPPROTO_IP, option_name,
		      (struct dn_pipe *) option_value, option_length) < 0)
	{
	  WARNING ("Error setting socket options");
	  perror ("setsockopt");
	  return ERROR;
	}
      break;

    default:
      WARNING ("Unrecognized socket option_name: %d", option_name);
      return ERROR;
    }

  return SUCCESS;
}


////////////////////////////////////////////////
// ipfw rule related functions
////////////////////////////////////////////////

// add an ipfw rule containing a dummynet pipe to the firewall;
// rule number is 'rulenum', pipe number is 'pipe_nr', source and 
// destination are 'src' and 'dst', direction is 'direction';
// return SUCCESS on success, ERROR on error

// NOTE: both 'src' and 'dst' can be either specific 
// IP addresses or the string "any"
int
add_rule_and_pipe (int socket_id, uint16_t rulenum, int pipe_nr,
		   char *src, char *dst, int direction)
{
  int rule_length_counter;	// rule length counter
  in4_addr addr;		// IP address structure
  uint16_t port;
  ipfw_insn cmd[10];
  struct ip_fw *pipe;
  socklen_t pipe_len;


  // rule length counter initialization
  rule_length_counter = 0;

  // process source address
  if (strncmp (src, "any", strlen (src)) == 0)
    {
      // source address was "any"
      cmd[rule_length_counter].opcode = O_IP_SRC;
      cmd[rule_length_counter].len = 0;
      cmd[rule_length_counter].arg1 = 0;
    }
  else
    {
      if (atoaddr (src, &addr, &port) < 0)
	{
	  WARNING ("Invalid argument to add_rule: %s\n", src);
	  return ERROR;
	}

      cmd[rule_length_counter].opcode = O_IP_SRC;
      cmd[rule_length_counter].len = 2;
      cmd[rule_length_counter].arg1 = 0;
      ((uint32_t *) cmd)[rule_length_counter + 1] = addr.word;
      rule_length_counter += 2;
      if (port > 0)
	{
	  cmd[rule_length_counter].opcode = O_IP_SRCPORT;
	  cmd[rule_length_counter].len = 2;
	  cmd[rule_length_counter].arg1 = 0;
	  ((uint32_t *) cmd)[rule_length_counter + 1] = port | port << 16;
	  rule_length_counter += 2;
	}
    }

  // process destination address
  if (strncmp (dst, "any", strlen (dst)) == 0)
    {
      // destination address was "any"
      cmd[rule_length_counter].opcode = O_IP_DST;
      cmd[rule_length_counter].len = 0;
      cmd[rule_length_counter].arg1 = 0;
    }
  else
    {
      if (atoaddr (dst, &addr, &port) < 0)
	{
	  WARNING ("Invalid argument to add_rule: %s", dst);
	  return ERROR;
	}

      cmd[rule_length_counter].opcode = O_IP_DST;
      cmd[rule_length_counter].len = 2;
      cmd[rule_length_counter].arg1 = 0;
      ((uint32_t *) cmd)[rule_length_counter + 1] = addr.word;
      rule_length_counter += 2;
      if (port > 0)
	{
	  cmd[rule_length_counter].opcode = O_IP_DSTPORT;
	  cmd[rule_length_counter].len = 2;
	  cmd[rule_length_counter].arg1 = 0;
	  ((uint32_t *) cmd)[rule_length_counter + 1] = port | port << 16;
	  rule_length_counter += 2;
	}
    }

  // use in/out direction indicators; 
  // if both directions are requested, nothing needs to be done
  if (direction != DIRECTION_BOTH)
    {
      // basic command code for in/out operation
      cmd[rule_length_counter].opcode = O_IN;
      cmd[rule_length_counter].len = 1;

      // a negation mask is used for meaning "out"
      if (direction == DIRECTION_OUT)
	cmd[rule_length_counter].len |= F_NOT;

      rule_length_counter += 1;
    }

  // configure pipe
  cmd[rule_length_counter].opcode = O_PIPE;
  cmd[rule_length_counter].len = 2;
  cmd[rule_length_counter].arg1 = pipe_nr;
  ((uint32_t *) cmd)[rule_length_counter + 1] = 0;
  rule_length_counter += 1;	/* trick! */

  if ((pipe = (struct ip_fw *) malloc (sizeof (struct ip_fw)
				       + rule_length_counter * 4)) == NULL)
    {
      WARNING ("Could not allocate memory for a new rule");
      return ERROR;
    }
  bzero (pipe, sizeof (struct ip_fw));
  pipe->act_ofs = rule_length_counter - 1;
  pipe->cmd_len = rule_length_counter + 1;
  pipe->rulenum = rulenum;
  bcopy (cmd, &pipe->cmd, rule_length_counter * 4);

  // do the operation by getting socket options
  pipe_len = sizeof (struct ip_fw) + rule_length_counter * 4;
  if (get_socket_options (socket_id, IP_FW_ADD, pipe, &pipe_len) == ERROR)
    {
      WARNING ("Adding rule & pipe operation failed");
      return ERROR;
    }

  return SUCCESS;
}

// get ipfw rules; on return, rule data will be stored in 'rule_data';
// if 'rule_data' must be allocated, set 'rule_data_alloc_size' to
// UNDEFINED_RULE_DATA_SIZE when calling, and on return it wil contain
// the allocated size, otherwise it is assumed 'rule_data_alloc_size'
// contains the allocate size of 'rule_data' (this is useful for next
// call, to avoid reallocation each time); on return 'rules' is an array of
// pointers to consecutive rules, and 'rules_count' is the number of rules;
// NOTE: when calling rule_count should contain the maximum number of rules
// that can be used;
// return SUCCESS on success, ERROR on error
int
get_rules (uint socket_id, struct ip_fw **rule_data,
	   int *rule_data_alloc_size, struct ip_fw **rules, int *rule_count)
{
  struct ip_fw *rule_pointer;
  int MAX_RULE_COUNT = *rule_count;

#define NEXT_RULE(rule_pointer)	((struct ip_fw *)((char *)rule_pointer       \
						  + RULESIZE(rule_pointer)))

  // number of bytes to be allocated
  int n_alloc;

  // number of bytes passed to get_socket_options
  int n_bytes;

  // pointer limiting the allocated block 'rule_data'  
  char *limit_pointer;

  int read_success = FALSE;


  // check whether buffer needs to be allocated
  if (*rule_data_alloc_size == UNDEFINED_ALLOC_SIZE)
    {
      DEBUG ("Buffer needs to be allocated");
      // initialize 'n_alloc' to an arbitrary value 
      n_alloc = 1024;

      // try to reallocate
      if (((*rule_data) = realloc (*rule_data, n_alloc)) == NULL)
	{
	  WARNING ("realloc failed");
	  return ERROR;
	}
    }
  else
    {
      DEBUG ("Buffer is already allocated");
      n_alloc = *rule_data_alloc_size;
    }

  // initialize 'n_bytes'
  n_bytes = n_alloc;

  // repeat socket operation until success
  do
    {
      // get socket options
      if (get_socket_options (socket_id, IP_FW_GET, *rule_data,
			      (socklen_t *) & n_bytes) == ERROR)
	{
	  WARNING ("Getting rule operation failed");
	  return ERROR;
	}

      // check whether the answer to be returned is larger than
      // the allocated block
      if (n_bytes >= n_alloc)
	{
	  // need to reallocate => increase the number of bytes to
	  // be allocated (arbitrary values)
	  n_alloc = n_alloc * 2 + 20;
	  n_bytes = n_alloc;
	  // try to reallocate
	  if (((*rule_data) = realloc (*rule_data, n_bytes)) == NULL)
	    {
	      WARNING ("realloc failed");
	      return ERROR;
	    }
	  else
	    *rule_data_alloc_size = n_alloc;
	}
      else
	read_success = TRUE;
    }
  while (read_success == FALSE);

  // save last allocated block size
  *rule_data_alloc_size = n_alloc;


  // go through rules; they have variable size so we
  // need to scan the list

  // NOTE: the last rule always has the number 65535, and cannot be
  // modified or deleted

  // initialize number of rules
  *rule_count = 0;
  limit_pointer = (char *) (*rule_data) + n_bytes;

  // find start for each rule and count them
  for (rule_pointer = (*rule_data); (char *) rule_pointer < limit_pointer;
       rule_pointer = NEXT_RULE (rule_pointer))
    {
      DEBUG ("Rule #%d: %llu pkts %llu bytes\n",
	     r->rulenum, align_uint64 (&r->pcnt), align_uint64 (&r->bcnt));

      if ((*rule_count) >= MAX_RULE_COUNT)
	{
	  WARNING ("Rule count exceeds maximum allowed count (%d)",
		   MAX_RULE_COUNT);
	  return ERROR;
	}
      else
	{
	  rules[*rule_count] = rule_pointer;
	  (*rule_count)++;
	}
    }

  return SUCCESS;
}

// delete an ipfw rule specified by 'rule_number';
// return SUCCESS on success, ERROR on error
int
delete_rule (uint s, u_int32_t rule_number)
{
  // Note: rule number is of type u_int16_t in ip_fw.h,
  // but a comment in ip_fw2.c shows that the expected size
  // when applying socket options is u_int32_t (see comment below)

  /* COMMENT FROM IP_FW2.C
   *
   * IP_FW_DEL is used for deleting single rules or sets,
   * and (ab)used to atomically manipulate sets. Argument size
   * is used to distinguish between the two:
   *    sizeof(u_int32_t)
   *    delete single rule or set of rules,
   *    or reassign rules (or sets) to a different set.
   *    2*sizeof(u_int32_t)
   *    atomic disable/enable sets.
   *    first u_int32_t contains sets to be disabled,
   *    second u_int32_t contains sets to be enabled.
   */

  // do delete rule
  if (set_socket_options (s, IP_FW_DEL, &rule_number, sizeof (rule_number))
      == ERROR)
    {
      WARNING ("Delete rule operation failed");
      return ERROR;
    }

  return SUCCESS;
}

// print a rule structure given in 'rule'
void
print_rule (struct ip_fw *rule)
{
  printf ("Rule #%d (size=%d):\n", rule->rulenum, sizeof (*rule));
  printf ("\tnext=%p next_rule=%p\n", rule->next, rule->next_rule);
  printf ("\tact_ofs=%u cmd_len=%u rulenum=%u set=%u _pad=%u\n",
	  rule->act_ofs, rule->cmd_len, rule->rulenum, rule->set, rule->_pad);
  printf ("\tpcnt=%llu bcnt=%llu timestamp=%u\n", rule->pcnt, rule->bcnt,
	  rule->timestamp);
}


////////////////////////////////////////////////
// dummynet pipe related functions
////////////////////////////////////////////////

// configure a dummynet pipe identified by 'pipe_nr';
// the parameters to use are 'bandwidth' [bps], 'delay' [ms],
// 'loss_rate' [probability], and 'queue_size' [packets];
// return SUCCESS on success, ERROR on error
int
configure_pipe (int socket_id, int pipe_nr, float bandwidth,
		float delay, float loss_rate, int queue_size)
{
  struct dn_pipe pipe;

  // reset data structure  
  bzero (&pipe, sizeof (pipe));

  // initialize pipe number
  pipe.pipe_nr = pipe_nr;

  // prepare adjusted values for parameters

  // 1. if packet size is known bandwidth could be adjusted:
  //   multiplication factor 1.0778 was computed for 400 byte 
  //   datagrams because of 28 bytes header (428/400=1.07)
  //   however we consider bandwidth at Ethernet level, 
  //   therefore we don't multiply here but when plotting results
  //   Note: ip_dummynet.h says that bandwidth is in bytes/tick
  //         but we seem to obtain correct values using bits/second
  pipe.bandwidth = (int) round (bandwidth);	// * 2.56);

  // 2. no adjustment necessary for delay expressed in ms
  pipe.delay = (int) round (delay);	//(delay / 2);

  // 3. loss rate probability must be extended to 
  //    2^31-1 (=0x7fffffff) range, equivalent to 100% loss
  pipe.fs.plr = (int) round (loss_rate * 0x7fffffff);

  // you may set the pipe queue size to a small value to avoid large
  // delays in case bandwidth limitation is enforced
  pipe.fs.qsize = queue_size;

  // do socket operation
  if (set_socket_options (socket_id, IP_DUMMYNET_CONFIGURE,
			  &pipe, sizeof (pipe)) == ERROR)
    {
      WARNING ("Pipe configuration could not be applied");
      return ERROR;
    }

  return SUCCESS;
}

// delete an ipfw pipe identified by 'pipe_number' by using a system call;
// return SUCCESS on success, ERROR on error
int
delete_pipe_syscall (int pipe_number)
{
  // this is an example of using system calls for controlling ipfw;
  // however, we prefer socket operations since they are supposed
  // to be more efficient

  char cmd[MAX_STRING];
  int status;

  snprintf (cmd, MAX_STRING - 1, "ipfw pipe %d delete", pipe_number);

  INFO ("Making system call: %s", cmd);

  status = system (cmd);

  if (status != 0)
    return ERROR;
  else
    return SUCCESS;
}

// delete an ipfw pipe identified by 'pipe_number' by using socket operations;
// return SUCCESS on success, ERROR on error
int
delete_pipe (uint socket_id, int pipe_number)
{
  struct dn_pipe pipe;

  // reset data structure  
  bzero (&pipe, sizeof (pipe));

  // initialize appropriate fields
  pipe.pipe_nr = pipe_number;

  // do delete rule
  if (set_socket_options (socket_id, IP_DUMMYNET_DEL,
			  &pipe, sizeof (pipe)) == ERROR)
    {
      WARNING ("Delete pipe operation failed");
      return ERROR;
    }

  return SUCCESS;
}

// print a pipe structure given by 'pipe'
void
print_pipe (struct dn_pipe *pipe)
{
  printf ("Pipe #%d (size=%d):\n", pipe->pipe_nr, sizeof (*pipe));
  //printf("\tnext=%p pipe_nr=%u\n", pipe->next.sle_next, pipe->pipe_nr);

#ifdef NEW_DUMMYNET
  printf ("\tbandwidth=%u delay=%u delay_type=%u fs.plr=%u fs.qsize=%u\n",
	  pipe->bandwidth, pipe->delay, pipe->delay_type, pipe->fs.plr,
	  pipe->fs.qsize);
#else
  printf ("\tbandwidth=%u delay=%u fs.plr=%u fs.qsize=%u\n",
	  pipe->bandwidth, pipe->delay, pipe->fs.plr, pipe->fs.qsize);
#endif
}


////////////////////////////////////////////////
// wireconf library management functions
////////////////////////////////////////////////

// initialize the wireconf library; static data is stored in the
// structure 'wireconf'; 
// return SUCCESS on succes, ERROR on error
int
wireconf_init (struct wireconf_class *wireconf, int my_id,
	       in_addr_t * IP_addresses, int node_count)
{
  int i;

  // get starting time
  clock_gettime (CLOCK_REALTIME, &(wireconf->start_time));

  // store id and node count  
  wireconf->my_id = my_id;
  wireconf->node_count = node_count;

  // allocate array for storing IP addresses
  wireconf->IP_addresses =
    malloc (wireconf->node_count * sizeof (struct in_addr));

  if (wireconf->IP_addresses == NULL)
    {
      WARNING ("Cannot allocate memory for IP_addresses");
      return ERROR;
    }

  // copy IP address information
  for (i = 0; i < wireconf->node_count; i++)
    (wireconf->IP_addresses[i]).s_addr = IP_addresses[i];


  // allocate array for storing total channel utilizations
  wireconf->total_channel_utilizations =
    malloc (wireconf->node_count * sizeof (float));

  if (wireconf->total_channel_utilizations == NULL)
    {
      WARNING ("Cannot allocate memory for total_channel_utilizations");
      return ERROR;
    }

  // allocate array for storing total transmission probabilities
  wireconf->total_transmission_probabilities =
    malloc (wireconf->node_count * sizeof (float));

  // Lan allocate array for storing statistic of channel utilization per source
  wireconf->channel_utilization_per_source =
    malloc (wireconf->node_count * sizeof (float *));
  for (i = 0; i < wireconf->node_count; i++)
    {
      wireconf->channel_utilization_per_source[i] =
	malloc (wireconf->node_count * sizeof (float));
    }

  if (wireconf->total_transmission_probabilities == NULL)
    {
      WARNING ("Cannot allocate memory for total_transmission_probabilities");
      return ERROR;
    }

  // allocate array for storing self channel utilizations
  wireconf->self_channel_utilizations =
    malloc (wireconf->node_count * sizeof (float));

  if (wireconf->self_channel_utilizations == NULL)
    {
      WARNING ("Cannot allocate memory for self_channel_utilizations");
      return ERROR;
    }

  // allocate array for storing self transmission probabilities
  wireconf->self_transmission_probabilities =
    malloc (wireconf->node_count * sizeof (float));

  if (wireconf->self_transmission_probabilities == NULL)
    {
      WARNING ("Cannot allocate memory for self_transmission_probabilities");
      return ERROR;
    }

  // initialize channel_utilizations
  for (i = 0; i < wireconf->node_count; i++)
    {
      wireconf->total_channel_utilizations[i] = 0.0;
      wireconf->total_transmission_probabilities[i] = 0.0;
      wireconf->self_channel_utilizations[i] = 0.0;
      wireconf->self_transmission_probabilities[i] = 0.0;
    }

  // start wireconf library message
  fprintf (stderr, "\nStarting wireconf library for \
my_id=%d & my_IP_address=%s on %s\n", wireconf->my_id, 
	   inet_ntoa (wireconf->IP_addresses[wireconf->my_id]), 
	   ctime (&(wireconf->start_time.tv_sec)));

  // initialize do_interrupt flags
  wireconf->do_interrupt_send = FALSE;
  wireconf->do_interrupt_listen = FALSE;

  // start statistics listen thread
  INFO ("Starting statistics listen thread...");
  if (pthread_create (&(wireconf->stats_listen_thread), NULL,
		      stats_listen_thread, (void *) wireconf) != 0)
    {
      WARNING ("Cannot create statistics listen thread");
      return ERROR;
    }

  // start statistics send thread
  INFO ("Starting statistics send thread...");
  if (pthread_create (&(wireconf->stats_send_thread), NULL,
		      stats_send_thread, (void *) wireconf) != 0)
    {
      WARNING ("Cannot create statistics send thread");
      return ERROR;
    }

  // create pipe used to provide statistics data to the send thread
  if (pipe (wireconf->stats_provide_pipe) == -1)
    {
      WARNING ("Cannot create pipe of statistics send thread");
      perror ("pipe");
      return ERROR;
    }

  return SUCCESS;
}

// finalize wireconf execution and wait for end of thread execution;
// return SUCCESS on succes, ERROR on error
int
wireconf_finalize (struct wireconf_class *wireconf)
{
  // free array for storing IP addresses
  if (wireconf->IP_addresses != NULL)
    free (wireconf->IP_addresses);

  // free array of total channel utilizations
  if (wireconf->total_channel_utilizations != NULL)
    free (wireconf->total_channel_utilizations);

  // free array of total transmission probabilities
  if (wireconf->total_transmission_probabilities != NULL)
    free (wireconf->total_transmission_probabilities);

  // free array of self channel utilizations
  if (wireconf->self_channel_utilizations != NULL)
    free (wireconf->self_channel_utilizations);

  // free array of self transmission probabilities
  if (wireconf->self_transmission_probabilities != NULL)
    free (wireconf->self_transmission_probabilities);

  ////////////////////////////////////////////////////
  // stopping statistics send thread
  INFO ("Stopping wireconf statistics send thread...");
  wireconf->do_interrupt_send = TRUE;

  // close write end of the pipe
  // (this will make the reading end finish execution)
  close (wireconf->stats_provide_pipe[1]);

  // wait for thread to react to flag and pipe changes
  sleep (1);

  // close read end of the pipe
  close (wireconf->stats_provide_pipe[0]);

  // stop and wait for statistics send thread to end
  pthread_cancel (wireconf->stats_send_thread);
  if (pthread_join (wireconf->stats_send_thread, NULL) != 0)
    {
      WARNING ("Error joining statistics send thread");
      return ERROR;
    }


  ////////////////////////////////////////////////////
  // stopping statistics listen thread
  INFO ("Stopping wireconf statistics listen thread...");
  wireconf->do_interrupt_listen = TRUE;

  // wait for thread to react to flag change
  sleep (1);

  // stop and wait for statistics listen thread to end
  pthread_cancel (wireconf->stats_listen_thread);
  if (pthread_join (wireconf->stats_listen_thread, NULL) != 0)
    {
      WARNING ("Error joining statistics listen thread");
      return ERROR;
    }

  return SUCCESS;
}

// deliver traffic statistics to wireconf library for distribution
// to other instances;
// return SUCCESS on succes, ERROR on error
int
wireconf_deliver_stats (struct wireconf_class *wireconf,
			struct stats_class *stats)
{
  if (stats->channel_utilization > 1.0)
    stats->channel_utilization = 0.9999;

  INFO ("Deliver statistics to wireconf send thread: \
channel_utilization=%.4f transmission_probability=%.4f...", 
	stats->channel_utilization, stats->transmission_probability);

  // store data in local structure
  wireconf->total_channel_utilizations[wireconf->my_id] =
    stats->channel_utilization;
  wireconf->total_transmission_probabilities[wireconf->my_id] =
    stats->channel_utilization;

  // send data to pipe (the statistics send thread is reading at the
  // other end
  if (write (wireconf->stats_provide_pipe[1], stats,
	     sizeof (struct stats_class)) == -1)
    {
      WARNING ("Error writing to pipe");
      perror ("write");
      return ERROR;
    }

  return SUCCESS;
}

// convert a "struct timespec" time value to seconds
static __inline float
timespec2sec (struct timespec *time_spec)
{
  return ((float) time_spec->tv_sec + (float) time_spec->tv_nsec / 1e9);
}

// compute the difference in seconds between two "struct timespec" time values
static __inline float
timespec_diff2sec (struct timespec *time_spec1, struct timespec *time_spec2)
{
  /*
     return ((float)(time_spec1->tv_sec - time_spec2->tv_sec) + 
     (float)(time_spec1->tv_nsec - time_spec2->tv_nsec) / 1e9);
   */

  // the algorithm below is inspired from: 
  // http://www.delorie.com/gnu/docs/glibc/libc_428.html
  // but it is not yet clear if it is better than the above simple calculation
  int sec_no;
  struct timespec result;

  // carry for the later subtraction by updating time_spec2
  if (time_spec1->tv_nsec < time_spec2->tv_nsec)
    {
      sec_no = (time_spec2->tv_nsec - time_spec1->tv_nsec) / 1e9 + 1;
      time_spec2->tv_nsec -= (1e9 * sec_no);
      time_spec2->tv_sec += sec_no;
    }

  if (time_spec1->tv_nsec - time_spec2->tv_nsec > 1e9)
    {
      sec_no = (time_spec1->tv_nsec - time_spec2->tv_nsec) / 1e9;
      time_spec2->tv_nsec += (1e9 * sec_no);
      time_spec2->tv_sec -= sec_no;
    }

  // compute the time difference; tv_nsec is certainly positive
  result.tv_sec = time_spec1->tv_sec - time_spec2->tv_sec;
  result.tv_nsec = time_spec1->tv_nsec - time_spec2->tv_nsec;

  return timespec2sec (&result);
}

// get the time elapsed (in seconds) since wireconf started execution
float
get_elapsed_time (struct wireconf_class *wireconf)
{
  struct timespec crt_time;

  // get current time
  clock_gettime (CLOCK_REALTIME, &crt_time);

  return timespec_diff2sec (&crt_time, &(wireconf->start_time));
}

  // -----------------codes are added by ntrtrung---------------------//
int init_udp_client()
{
	int s;
       if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
                printf("\nsocket");
		  return -1;
	}
	return s;
}
int SendDataToServer(float *data,int socket,int server_port)
{
	struct sockaddr_in si_other;
	if(socket <= 0)
	{
		printf("\nsocket die");
		return -1;
	}
	 int slen=sizeof(struct sockaddr_in);
	 memset((char *)&si_other, 0, slen);
	 si_other.sin_family = AF_INET;
       si_other.sin_port = htons(server_port);
       si_other.sin_addr.s_addr =  htonl(INADDR_LOOPBACK);	 
	return sendto(socket, data, sizeof(float), 0,(struct sockaddr *)&si_other,slen);	
}



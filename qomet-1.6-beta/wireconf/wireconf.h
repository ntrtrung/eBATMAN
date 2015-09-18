
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
 * File name: wireconf.h
 * Function: Header file of wired-network emulator configurator library
 *
 * Authors: Junya Nakata, Razvan Beuran
 *
 ***********************************************************************/

#ifndef	__WIRECONF_H
#define	__WIRECONF_H


#include <sys/queue.h>
#include <sys/socket.h>

#include <sys/types.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip_fw.h>
#include <netinet/ip_dummynet.h>

#include <pthread.h>


#include "routing_info.h"


/////////////////////////////////////////////
// Basic constants
/////////////////////////////////////////////

// traffic directions to which dummynet configuration
// is to be applied
#define DIRECTION_BOTH                  0
#define DIRECTION_IN                    1
#define DIRECTION_OUT                   2

// default size of pipe queue
//#define PIPE_QSIZE                      50
#define PIPE_QSIZE                      1000

#define UNDEFINED_ALLOC_SIZE            -1

#define MAX_NODES_W                     100

// after recent changes to do_wireconf for binary 
// support, it is not safe to assume first node id
// can be different than 0 !!!!!!!!!!!!!!!!!!!!
#define FIRST_NODE_ID                   0


///////////////////////////////////////////////
// IPv4 address data structure
///////////////////////////////////////////////

// IPv4 address structure
typedef union
{
  uint8_t octet[4];
  uint32_t word;
} in4_addr;


//////////////////////////////////
// Wireconf structure
//////////////////////////////////

// wireconf class structure
struct wireconf_class
{
  // the starting time for this wireconf module
  struct timespec start_time;

  // my id in the experiment
  int my_id;

  // experiment-side IP addresses for all nodes in the experiment
  struct in_addr *IP_addresses;

  // the number of nodes in the experiment
  int node_count;

  // thread for statistics communication
  pthread_t stats_send_thread;

  // flag showing that send thread execution must be stopped
  int do_interrupt_send;

  // pipe for providing statistics to send thread
  int stats_provide_pipe[2];

  // thread for listening to statistics information
  pthread_t stats_listen_thread;

  // flag showing that listen thread execution must be stopped
  int do_interrupt_listen;

  // array of total channel utilizations for each source
  float *total_channel_utilizations;

  // array of total transmission probabilities for each source
  float *total_transmission_probabilities;

  // array of channel utilizations from this node to each destination
  float *self_channel_utilizations;

  // array of transmission probabilities from this node to each destination
  float *self_transmission_probabilities;

  float total_channel_utilization_others;
  float total_transmission_probability_others;
  float total_self_number_packets;

  //Lan: array of channel utilization per source of all other nodes
  float **channel_utilization_per_source;

  /* NOTE: the code below intended for the next version of wireconf,
     that will be able to read QOMET binary output

     // data queue of this chanel
     data_queue_class data_queue;

     // array of flags showing which destinations are
     // active (TRUE=>active, FALSE=>inactive)
     int active_destinations[MAX_DESTINATIONS];

     // configuration array
     deltaQ_class configurations[MAX_DESTINATIONS];
   */

};


/////////////////////////////////////////////
// Socket manipulation functions
/////////////////////////////////////////////

// get socket identifier;
// return socket identifier on success, ERROR on error
int get_socket (void);

// close socket specified by socket_id
void close_socket (int socket_id);


////////////////////////////////////////////////
// ipfw rule related functions
////////////////////////////////////////////////

// add an ipfw rule containing a dummynet pipe to the firewall;
// rule number is 'rulenum', pipe number is 'pipe_nr', source and 
// destination are 'src' and 'dst', direction is 'direction';
// return SUCCESS on success, ERROR on error

// NOTE: both 'src' and 'dst' can be either specific 
// IP addresses or the string "any"
int add_rule_and_pipe (int socket_id, uint16_t rulenum, int pipe_nr,
		       char *src, char *dst, int direction);

// get ipfw rules; on return, rule data will be stored in 'rule_data';
// if 'rule_data' must be allocated, set 'rule_data_alloc_size' to
// UNDEFINED_RULE_DATA_SIZE when calling, and on return it wil contain
// the allocated size, otherwise it is assumed 'rule_data_alloc_size'
// contains the allocate size of 'rule_data' (this is useful for next
// call, to avoid reallocation each time); on return 'rules' is an array of
// pointers to consecutive rules, and 'rules_no' is the number of rules;
// return SUCCESS on success, ERROR on error
int get_rules (uint socket_id, struct ip_fw **rule_data,
	       int *rule_data_alloc_size, struct ip_fw **rules, int *rule_no);

// delete an ipfw rule specified by 'rule_number';
// return SUCCESS on success, ERROR on error
int delete_rule (uint s, u_int32_t rule_number);

// print a rule structure given in 'rule'
void print_rule (struct ip_fw *rule);


////////////////////////////////////////////////
// dummynet pipe related functions
////////////////////////////////////////////////

// configure a dummynet pipe identified by 'pipe_nr';
// the parameters to use are 'bandwidth' [bps], 'delay' [ms],
// 'loss_rate' [probability], and 'queue_size' [packets];
// return SUCCESS on success, ERROR on error
int
configure_pipe (int socket_id, int pipe_nr, float bandwidth,
		float delay, float loss_rate, int queue_size);

// delete an ipfw pipe identified by 'pipe_number' by using a system call;
// return SUCCESS on success, ERROR on error
int delete_pipe_syscall (int pipe_number);

// delete an ipfw pipe identified by 'pipe_number' by using socket operations;
// return SUCCESS on success, ERROR on error
int delete_pipe (uint s, int pipe_number);

// print a pipe structure given by 'pipe'
void print_pipe (struct dn_pipe *pipe);


////////////////////////////////////////////////
// wireconf library management functions
////////////////////////////////////////////////

// initialize the wireconf library; static data is stored in the 
// structure 'wireconf';
// return SUCCESS on succes, ERROR on error
int wireconf_init (struct wireconf_class *wireconf, int my_id,
		   in_addr_t * IP_addresses, int node_count);

// finalize wireconf execution and wait for end of thread execution;
// return SUCCESS on succes, ERROR on error
int wireconf_finalize (struct wireconf_class *wireconf);


// get the time elapsed (in seconds) since wireconf started execution
float get_elapsed_time (struct wireconf_class *wireconf);


////////////////////////////////////////////////
// statistics functions
////////////////////////////////////////////////

// include here to make it easy for users to access functions from the 
// statistics module fo the library
#include "statistics.h"

// deliver traffic statistics to wireconf library for distribution
// to other instances;
// return SUCCESS on succes, ERROR on error
int wireconf_deliver_stats (struct wireconf_class *wireconf,
			    struct stats_class *stats);
int init_udp_client();
int SendDataToServer(float *data,int socket,int server_port);

#endif /* !__WIRECONF_H */

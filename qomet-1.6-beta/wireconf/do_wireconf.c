
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
 * File name: do_wireconf.c
 * Function: Main source file of the real-time wired-network emulator 
 *           configuration program. At the moment it can be used to 
 *           drive the network emulator "dummynet" on FreeBSD 
 *
 * Authors: Junya Nakata, Lan Nguyen Tien, Razvan Beuran
 *
 *   $Revision: 128 $
 *   $LastChangedDate: 2009-02-06 10:21:50 +0900 (Fri, 06 Feb 2009) $
 *   $LastChangedBy: razvan $
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

#include "../deltaQ/deltaQ.h"
#include "global.h"
#include "wireconf.h"
#include "../timer/timer.h"
#include "message.h"
#include "routing_info.h"
#include "do_wireconf_ipc.h"


/////////////////////////////////////////////
// Basic constants
/////////////////////////////////////////////

// -----------codes are added by ntrtrung----------------//
//#define server port for QO_ACTION and BROADCAST_ENVIRONMENT Servers
#define DO_ACTION_SERVER_PORT  9911
#define BROAD_ENVI_SERVER_PORT 9912
// -----------codes are added by ntrtrung----------------//

// controlling pipe ids
#define MIN_PIPE_ID_IN          10000
#define MIN_PIPE_ID_OUT         20000
#define MIN_PIPE_ID_IN_BCAST	30000
#define MIN_PIPE_ID_OUT_BCAST	40000

// maximum number of rules
//#define MAX_RULE_COUNT            100
#define MAX_RULE_COUNT		500

// default node number (when no setting file is provided)
#define DEFAULT_NODE_COUNT      2
#define DEFAULT_FRAME_SIZE      1024

#ifndef SCALING_FACTOR
#define SCALING_FACTOR          1.0	//1.0 => no scaling; >1.0 => slow down
#endif /* !SCALING_FACTOR */


// used to signify "undefined" for numbers
#define UNDEFINED_SIGNED        -1
#define UNDEFINED_UNSIGNED      65535


////////////////////////////////////////////////
// Constants for making a predefined experiment
////////////////////////////////////////////////

#define PREDEFINED_EXPERIMENT   FALSE	// enable (TRUE) or disable (FALSE)
					// predefined experiment conditions

#define BANDWIDTH               10e6	// bandwidth in bps
#define DELAY                   5	// delay in ms
#define PACKET_LOSS_RATE        0	// loss rate in range [0,1]
#define LOOP_COUNT              10	// number of iterations

#define GW_ID			0	// ID of Gateway

///////////////////////////////////
// Generic functions
///////////////////////////////////

// print a brief usage of this program
void
usage (char *argv0)
{
  fprintf (stderr, "\ndo_wireconf. Drive network emulation using QOMET \
data.\n\n");
  fprintf (stderr,
	   "do_wireconf can use the QOMET data in <qomet_output_file> \
in two different ways:\n");
  fprintf (stderr,
	   "(1) Configure a specified pair <from_node_id> (IP address \
<from_node_addr>)\n");
  fprintf (stderr, "    <to_node_id> (IP address <to_node_addr>) using the \
dummynet rule\n");
  fprintf (stderr, "    <rule_number> and pipe <pipe_number>, optionally in \
direction 'in' or 'out'.\n");
  fprintf (stderr, "    Usage: %s -q <qomet_output_file>\n"
	   "\t\t\t-f <from_node_id> \t-F <from_node_addr>\n"
	   "\t\t\t-t <to_node_id> \t-T <to_node_addr>\n"
	   "\t\t\t-r <rule_number> \t-p <pipe_number>\n"
	   "\t\t\t[-d {in|out}]\n", argv0);
  fprintf (stderr, "(2) Configure all connections from the current node \
<current_id> given the\n");
  fprintf (stderr, "    IP settings in <settings_file> and the OPTIONAL \
broadcast address\n");
  fprintf (stderr, "    <broadcast_address>; Interval between configurations \
is <time_period>.\n");
  fprintf (stderr, "    The settings file contains on each line pairs of ids \
and IP addresses.\n");
  fprintf (stderr, "    Usage: %s -q <qomet_output_file>\n"
	   "\t\t\t-i <current_id> \t-s <settings_file>\n"
	   "\t\t\t-m <time_period> \t[-b <broadcast_address>]\n", argv0);
  fprintf (stderr, "NOTE: If option '-s' is used, usage (2) is inferred, \
otherwise usage (1) is assumed.\n");
}

// read settings (node ids and corresponding IP adresses)
// from a file, and store the adresses in the array p at
// the corresponding index;
// return the number of addresses (=nodes) successfully read, 
// or -1 on ERROR
int
read_settings (char *path, in_addr_t * p, int p_size)
{
  static char buf[BUFSIZ];
  int i = 0;
  int line_nr = 0;
  FILE *fd;

  int node_id;
  char node_ip[IP_ADDR_SIZE];

  // open settings file
  if ((fd = fopen (path, "r")) == NULL)
    {
      WARNING ("Cannot open settings file '%s'", path);
      return -1;
    }

  // parse file
  while (fgets (buf, BUFSIZ, fd) != NULL)
    {
      line_nr++;

      // check we didn't exceed maximum size
      if (i >= p_size)
	{
	  WARNING ("Maximum number of IP addresses (%d) exceeded", p_size);
	  fclose (fd);
	  return -1;
	}
      else
	{
	  int scaned_items;

	  // parse each line for node id and IP
	  scaned_items = sscanf (buf, "%d %16s", &node_id, node_ip);
	  if (scaned_items < 2)
	    {
	      WARNING ("Skipped invalid line #%d in settings file '%s'",
		       line_nr, path);
	      continue;
	    }
	  if (node_id < 0 || node_id < FIRST_NODE_ID
	      || node_id >= MAX_NODES_W)
	    {
	      WARNING
		("Node id %d is not within the permitted range [%d, %d]",
		 node_id, FIRST_NODE_ID, MAX_NODES_W);
	      fclose (fd);
	      return -1;
	    }
	  if ((p[node_id] = inet_addr (node_ip)) != INADDR_NONE)
	    i++;
	}
    }

  fclose (fd);
  return i;
}


///////////////////////////////////
// Main function
///////////////////////////////////

int
main (int argc, char *argv[])
{

  /////////////////////////////////////////////////////
  // General variables

  // deltaQ paramters to be applied to dummynet
  float bandwidth, loss_rate, delay;

  // time for the current records
  float crt_record_time;

  // qomet file descriptor
  FILE *qomet_fd = NULL;

  // timer handle for regulating dummynet configuration
  timer_handle *timer;

  // loop count used in predefined experiments
  int loop_count = 0;

  // time at the beginning and end of the experiment; 
  // used to measure total execution time
  struct timeval tp_begin, tp_end;

  // information exhange with olsrd;
  // store information of links from this node
  qomet_link_info *qomet_link_ipc;
  // store information of channel, a view from this node
  qomet_channel_info *qomet_channel_ipc;

  // character used when parsing command line options
  char option_ch;

  // pointer used when calling the function strtol
  char *strtol_p;

  // dummynet control socket id
  int dummynet_socket_id;

  // show whether usage 1 or usage 2 are employed
  char usage_type;


  /////////////////////////////////////////////////////
  // Usage 1 variables

  // IP addresses of the from and to nodes (usage 1)
  char *from_ip_addr = NULL, *to_ip_addr = NULL;

  // ids of the from and to nodes (usage 1)
  int from_id = UNDEFINED_SIGNED, to_id = UNDEFINED_SIGNED;

  // number of the pipe used with dummynet (usage 1)
  int pipe_number = UNDEFINED_SIGNED;

  // number of the rule used with dummynet (usage 1)
  u_int32_t rule_number = UNDEFINED_UNSIGNED;

  // direction of dummynet pipe (usage 1)
  int direction = DIRECTION_BOTH;


  /////////////////////////////////////////////////////
  // Usage 2 variables

  // the id of this node (usage 2)
  int my_id = UNDEFINED_SIGNED;

  // array of next hop ids
  int *next_hop_ids = NULL;

  // array of IP adresses of all nodes in the experiments
  in_addr_t IP_addresses[MAX_NODES_W];

  // array of IP adresses of all nodes in the experiments
  // (string form)
  char IP_char_addresses[MAX_NODES_W * IP_ADDR_SIZE];

  // broadcast address as string
  char broadcast_address[IP_ADDR_SIZE];

  // number of nodes in the experiments
  // (determined from settings files)
  int node_count = UNDEFINED_SIGNED;

  // iterator for nodes
  int node_i;

  // time period used when configuring dummynet; 
  // if time_period <= step in QOMET file, no change occurs,
  // but if time_period > step in QOMET file, configurations
  // will be applied less frequently
  float time_period = UNDEFINED_SIGNED;

  // memory to store rule data as a block
  struct ip_fw *rule_data = NULL;

  // size of allocate memory for rule data
  int rule_data_alloc_size = UNDEFINED_ALLOC_SIZE;

  // arrary of pointers to rule data
  struct ip_fw *rules[MAX_RULE_COUNT];

  // effective number of rules  
  int rule_count;

  // arrays of counters for the number of packets/bytes
  // transmitted to each destination during the last
  // time interval
  long long unsigned int *last_pkt_counters = NULL;;
  long long unsigned int *last_byte_counters = NULL;;

  long long unsigned int *last_pkt_counters_per_source = NULL;;
  long long unsigned int *last_byte_counters_per_source = NULL;;

  // array of average frame size statistics
  float *avg_frame_sizes = NULL;

  // wireconf structure
  struct wireconf_class wireconf;

  // configuration file header data structure
  binary_header_class binary_header;

  // time records counter
  long int time_i;

  // dummynet configuration records
  binary_time_record_class binary_time_record;
  binary_record_class *binary_records = NULL;
  int binary_records_max_count;

  binary_record_class **my_records_ucast = NULL;
  binary_record_class *adjusted_records_ucast = NULL;
  int *my_records_ucast_changed = NULL;
  binary_record_class *my_records_bcast = NULL;
  int *my_records_bcast_changed = NULL;

  // channel utilizations
  float my_total_channel_utilization;
  float my_total_transmission_probability;
  //Lan for computing channel utilization per source 
  float *channel_utilization_per_source = NULL;
  float cwb_channel_utilization;

  // flag showing whtehr channel utilization was sent to OLSR or not
  int channel_utilization_sent;



  int pkt_i;

  /////////////////////////////////////////////////////
  // Basic variable initialization

  // default usage type
  usage_type = 1;

  // default broadcast address
  strncpy (broadcast_address, "255.255.255.255", IP_ADDR_SIZE);


  /////////////////////////////////////////////////////
  // Argument parsing

  // basic check for arguments
  if (argc < 2)
    {
      WARNING ("No arguments provided");
      usage (argv[0]);
      exit (1);
    }

  // parse command-line options
  while ((option_ch = getopt (argc, argv, "q:f:F:t:T:r:p:d:i:s:m:b:")) != -1)
    {
      int node_i;

      switch (option_ch)
	{
	  //QOMET output file
	case 'q':
	  if ((qomet_fd = fopen (optarg, "r")) == NULL)
	    {
	      WARNING ("Could not open QOMET output file '%s'", optarg);
	      exit (1);
	    }
	  else
	    INFO ("Using QOMET file: %s", optarg);
	  break;

	  /////////////////////////////////////////////
	  // Usage (1) parameters
	  /////////////////////////////////////////////
	  // from_node_id
	case 'f':
	  from_id = strtol (optarg, &strtol_p, 10);
	  if ((*optarg == '\0') || (*strtol_p != '\0'))
	    {
	      WARNING ("Invalid from_node_id '%s'", optarg);
	      exit (1);
	    }
	  break;

	  // IP address of from_node
	case 'F':
	  from_ip_addr = optarg;
	  break;

	  // to_node_id
	case 't':
	  to_id = strtol (optarg, &strtol_p, 10);
	  if ((*optarg == '\0') || (*strtol_p != '\0'))
	    {
	      WARNING ("Invalid to_node_id '%s'", optarg);
	      exit (1);
	    }
	  break;

	  // IP address of to_node
	case 'T':
	  to_ip_addr = optarg;
	  break;

	  // rule number for dummynet configuration
	case 'r':
	  rule_number = strtol (optarg, &strtol_p, 10);
	  if ((*optarg == '\0') || (*strtol_p != '\0'))
	    {
	      WARNING ("Invalid rule_number '%s'", optarg);
	      exit (1);
	    }
	  break;

	  // pipe number for dummynet configuration
	case 'p':
	  pipe_number = strtol (optarg, &strtol_p, 10);
	  if ((*optarg == '\0') || (*strtol_p != '\0'))
	    {
	      WARNING ("Invalid pipe_number '%s'", optarg);
	      exit (1);
	    }
	  break;

	  // direction option for dummynet configuration
	case 'd':
	  if (strcmp (optarg, "in") == 0)
	    {
	      direction = DIRECTION_IN;
	    }
	  else if (strcmp (optarg, "out") == 0)
	    {
	      direction = DIRECTION_OUT;
	    }
	  else
	    {
	      WARNING ("Invalid direction '%s'", optarg);
	      exit (1);
	    }
	  break;

	  /////////////////////////////////////////////
	  // Usage (2) parameters
	  /////////////////////////////////////////////
	  // current node ID
	case 'i':
	  my_id = strtol (optarg, NULL, 10);
	  break;

	  // settings file
	case 's':
	  usage_type = 2;
	  if ((node_count
	       = read_settings (optarg, IP_addresses, MAX_NODES_W)) < 1)
	    {
	      WARNING ("Invalid setting file: '%s'", optarg);
	      exit (1);
	    }
	  for (node_i = 0; node_i < node_count; node_i++)
	    snprintf (IP_char_addresses + node_i * IP_ADDR_SIZE, IP_ADDR_SIZE,
		      "%hu.%hu.%hu.%hu",
		      *(((uint8_t *) & IP_addresses[node_i]) + 0),
		      *(((uint8_t *) & IP_addresses[node_i]) + 1),
		      *(((uint8_t *) & IP_addresses[node_i]) + 2),
		      *(((uint8_t *) & IP_addresses[node_i]) + 3));

	  /* Lan added on March 25 for information exchange with olsrd */

	  // locate shared mem
	  fprintf (stderr, "Creating shared mem ...\n");

	  // create file for shared mem
	  if (ipc_create_file () == ERROR)
	    exit (1);

	  // memory for link information
	  if ((qomet_link_ipc =
	       ipc_locate (IPC_LINK_NAME,
			   node_count * sizeof (qomet_link_info))) == NULL)
	    {
	      WARNING ("Cannot locate share memory at %s\n", IPC_LINK_NAME);
	      //Lan remove on Jun 29 for prenting from experiment fails
	      //exit(1);
	    }

	  INFO ("Initializing link information ...");
	  ipc_link_init (qomet_link_ipc, node_count);

	  // Mem for channel information
	  if ((qomet_channel_ipc =
	       ipc_locate (IPC_CHANNEL_NAME,
			   NUMBER_OF_CHANNELS * sizeof (qomet_channel_info)))
	      == NULL)
	    {
	      printf ("Cannot locate share memory at %s\n", IPC_CHANNEL_NAME);
	      exit (1);
	    }

	  INFO ("Initializing channel information ...");
	  ipc_channel_init (qomet_channel_ipc, NUMBER_OF_CHANNELS);
	  /* << Lan added on March 25 for information exchange with olsrd */

	  break;

	  // time interval between settings
	case 'm':
	  // check if conversion was performed (we assume a time
	  // period of 0 is also invalid)
	  if ((time_period = strtod (optarg, NULL)) == 0)
	    {
	      WARNING ("Invalid time period");
	      exit (1);
	    }
	  break;

	case 'b':
	  strncpy (broadcast_address, optarg, IP_ADDR_SIZE);
	  break;

	  // help output
	case '?':
	default:
	  usage (argv[0]);
	  exit (1);
	}
    }

  // do id checking
  if (usage_type == 1)
    {
      // we must initialize node_count
      node_count = DEFAULT_NODE_COUNT;

      if ((from_id < FIRST_NODE_ID)
	  || (from_id >= node_count + FIRST_NODE_ID))
	{
	  WARNING ("Invalid <from_node_id>: '%d'. Valid range is [%d, %d]",
		   from_id, FIRST_NODE_ID, node_count + FIRST_NODE_ID - 1);
	  exit (1);
	}

      if ((to_id < FIRST_NODE_ID) || (to_id >= node_count + FIRST_NODE_ID))
	{
	  WARNING ("Invalid <to_node_id>: '%d'. Valid range is [%d, %d]",
		   to_id, FIRST_NODE_ID, node_count + FIRST_NODE_ID - 1);
	  exit (1);
	}
    }
  else if ((my_id < FIRST_NODE_ID) || (my_id >= node_count + FIRST_NODE_ID))
    {
      WARNING ("Invalid node id: '%d'. Valid range is [%d, %d]", my_id,
	       FIRST_NODE_ID, node_count + FIRST_NODE_ID - 1);
      exit (1);
    }

  // update argument-related counter and pointer
  argc -= optind;
  argv += optind;

  // check that all the required arguments were provided
  if (qomet_fd == NULL)
    {
      WARNING ("No QOMET data file was provided");
      usage (argv[0]);
      exit (1);
    }
  if (usage_type == 1)
    {
      if ((from_id == UNDEFINED_SIGNED) || (from_ip_addr == NULL)
	  || (to_id == UNDEFINED_SIGNED) || (to_ip_addr == NULL)
	  || (pipe_number == UNDEFINED_SIGNED)
	  || (rule_number == UNDEFINED_UNSIGNED))
	{
	  WARNING ("Insufficient arguments were provided for usage (1)");
	  usage (argv[0]);
	  fclose (qomet_fd);
	  exit (1);
	}
    }
  else if ((my_id == UNDEFINED_SIGNED) || (node_count == UNDEFINED_SIGNED)
	   || (time_period == UNDEFINED_SIGNED))
    {
      WARNING ("Insufficient arguments were provided for usage (2)");
      usage (argv[0]);
      fclose (qomet_fd);
      exit (1);
    }


  /////////////////////////////////////////////////////
  // More initialization

  // initialize my IP address
  if (usage_type == 1)
    {
      struct in_addr my_IP_address;

      // we assume the current node is the "from node"
      my_id = from_id;
      if (inet_aton (from_ip_addr, &my_IP_address) == 0)
	{
	  WARNING ("Could not convert IP address");
	  perror ("inet_aton");
	  exit (1);
	}
      else
	IP_addresses[my_id] = my_IP_address.s_addr;
    }

  // initialize wireconf library
  if (wireconf_init (&wireconf, my_id, IP_addresses, node_count) == ERROR)
    {
      WARNING ("Could not initialize wireconf library");
      exit (1);
    }

  // initialize timer library
  DEBUG ("Initialize timer...");
  if ((timer = timer_init ()) == NULL)
    {
      WARNING ("Could not initialize timer");
      exit (1);
    }

  // open dummynet socket
  DEBUG ("Open dummynet socket...");
  if ((dummynet_socket_id = get_socket ()) < 0)
    {
      WARNING
	("Could not open dummynet socket (requires root priviledges)\n");
      exit (1);
    }


  /////////////////////////////////////////////////////
  // Prepare dummynet rules/pipes

  INFO ("Adding rules and pipes to dummynet...");

  // add pipe to dummynet in normal manner
  if (usage_type == 1)
    {
      DEBUG ("Adding rule #%d with pipe #%d from %s to %s...",
	     rule_number, pipe_number, from_ip_addr, to_ip_addr);
      if (add_rule_and_pipe (dummynet_socket_id, rule_number, pipe_number,
			     from_ip_addr, to_ip_addr, direction) < 0)
	{
	  WARNING ("Could not add rule #%d with pipe #%d from %s to %s",
		   rule_number, pipe_number, from_ip_addr, to_ip_addr);
	  exit (1);
	}
    }
  // usage (2) => sets of rules must be added
  else
    {
      int j;

      // add rule & pipe for unicast traffic _to_ j
      for (j = FIRST_NODE_ID; j < node_count + FIRST_NODE_ID; j++)
	{
	  if (j == my_id)
	    continue;

	  DEBUG ("Node %d: Adding rule #%d with pipe #%d to destination \
%s (OUT)...", my_id, MIN_PIPE_ID_OUT + j, MIN_PIPE_ID_OUT + j, 
		 IP_char_addresses + (j - FIRST_NODE_ID) * IP_ADDR_SIZE);

	  if (add_rule_and_pipe
	      (dummynet_socket_id, MIN_PIPE_ID_OUT + j, MIN_PIPE_ID_OUT + j,
	       "any", IP_char_addresses + (j - FIRST_NODE_ID) * IP_ADDR_SIZE,
	       DIRECTION_OUT) < 0)
	    {
	      WARNING ("Node %d: Could not add rule #%d with pipe #%d to \
destination %s (OUT)", my_id, MIN_PIPE_ID_OUT + j, MIN_PIPE_ID_OUT + j, 
		       IP_char_addresses + (j - FIRST_NODE_ID) * IP_ADDR_SIZE);
	      exit (1);
	    }
	}			// end for loop

      // Lan added on Oct. 01 for doing statistic on source
      for (j = FIRST_NODE_ID; j < node_count + FIRST_NODE_ID; j++)
	{
	  DEBUG ("Node %d: Adding rule #%d with pipe #%d from source \
%s (OUT)...", my_id, MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j, 
		 MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j, 
		 IP_char_addresses + (j - FIRST_NODE_ID) * IP_ADDR_SIZE);

	  if (add_rule_and_pipe
	      (dummynet_socket_id, MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j,
	       MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j,
	       IP_char_addresses + (j - FIRST_NODE_ID) * IP_ADDR_SIZE,
	       IP_char_addresses + (GW_ID - FIRST_NODE_ID) * IP_ADDR_SIZE,
	       DIRECTION_OUT) < 0)
	    {
	      WARNING ("Node %d: Could not add rule #%d with pipe #%d from \
source %s to destination %s (OUT)", my_id, MIN_PIPE_ID_OUT 
		       + CWB_PIPE_ID_OFFSET + j, MIN_PIPE_ID_OUT 
		       + CWB_PIPE_ID_OFFSET + j, IP_char_addresses 
		       + (j - FIRST_NODE_ID) * IP_ADDR_SIZE, 
		       IP_char_addresses + (GW_ID - FIRST_NODE_ID) 
		       * IP_ADDR_SIZE);
	      exit (1);
	    }

	  // print current configuration info
	  INFO ("* Wireconf pipe=%d: #%d [%s] UCAST to #%d  [%s] (time=%.2f s): \
bandwidth=%.2fbit/s loss_rate=%.4f delay=%.4f ms", MIN_PIPE_ID_OUT +
		CWB_PIPE_ID_OFFSET + j, j, IP_char_addresses 
		+ (j - FIRST_NODE_ID) * IP_ADDR_SIZE, GW_ID,
		IP_char_addresses + (GW_ID - FIRST_NODE_ID) 
		* IP_ADDR_SIZE, crt_record_time, 100e6, 0.0, 0.0);
	  configure_pipe (dummynet_socket_id,
			  MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j, 100e6,
			  0.0, 0.0, PIPE_QSIZE);
	}


      // add rule & pipe for broadcast traffic _from_ j
      for (j = FIRST_NODE_ID; j < node_count + FIRST_NODE_ID; j++)
	{
	  if (j == my_id)
	    continue;

	  DEBUG ("Node %d: Adding rule #%d with pipe #%d to destination \
%s (IN)...", my_id, MIN_PIPE_ID_IN_BCAST + j, MIN_PIPE_ID_IN_BCAST + j, 
		 broadcast_address);
	  if (add_rule_and_pipe
	      (dummynet_socket_id, MIN_PIPE_ID_IN_BCAST + j,
	       MIN_PIPE_ID_IN_BCAST + j,
	       IP_char_addresses + (j - FIRST_NODE_ID) *
	       IP_ADDR_SIZE, broadcast_address, DIRECTION_IN) < 0)
	    {
	      WARNING ("Node %d: Could not add rule #%d with pipe #%d from %s \
to destination %s (IN)", my_id, MIN_PIPE_ID_IN_BCAST + j,
		       MIN_PIPE_ID_IN_BCAST + j, IP_char_addresses 
		       + (j - FIRST_NODE_ID) * IP_ADDR_SIZE, 
		       broadcast_address);
	      exit (1);
	    }
	}
    }


  ///////////////////////////////////////////////////////////
  // Read QOMET binary file header and do memory allocations

  INFO ("Start reading binary QOMET output file...");

  // read and check binary header
  if (io_read_binary_header_from_file (&binary_header, qomet_fd) == ERROR)
    {
      WARNING ("Aborting on input error (binary header)");
      exit (1);
    }
  io_binary_print_header (&binary_header);

  if (node_count != binary_header.node_number)
    {
      WARNING ("Number of nodes according to the settings file (%d) and \
number of nodes according to QOMET scenario (%d) differ", 
	       node_count, binary_header.node_number);
      exit (1);
    }

  // compute the maximum number of records we can expect
  binary_records_max_count = binary_header.node_number
    * (binary_header.node_number - 1);

  // allocate memory for binary records
  binary_records = (binary_record_class *)
    calloc (binary_records_max_count, sizeof (binary_record_class));
  if (binary_records == NULL)
    {
      WARNING ("Cannot allocate memory for binary records");
      exit (1);
    }

  // allocate memory for next hop ids
  next_hop_ids = (int *) calloc (binary_header.node_number, sizeof (int));
  if (next_hop_ids == NULL)
    {
      WARNING ("Cannot allocate memory for next_hop_ids");
      exit (1);
    }

  // allocate memory for my records when using UCAST
  my_records_ucast = (binary_record_class **)
    calloc (binary_header.node_number, sizeof (binary_record_class *));
  if (my_records_ucast == NULL)
    {
      WARNING ("Cannot allocate memory for my_records_ucast");
      exit (1);
    }
  for (node_i = 0; node_i < binary_header.node_number; node_i++)
    {
      my_records_ucast[node_i] = (binary_record_class *)
	calloc (binary_header.node_number, sizeof (binary_record_class));

      if (my_records_ucast[node_i] == NULL)
	{
	  WARNING ("Cannot allocate memory for my_records_ucast[%d]", node_i);
	  exit (1);
	}
    }

  // allocate memory for adjusted records when using UCAST
  adjusted_records_ucast = (binary_record_class *)
    calloc (binary_header.node_number, sizeof (binary_record_class));
  if (adjusted_records_ucast == NULL)
    {
      WARNING ("Cannot allocate memory for adjusted_records_ucast");
      exit (1);
    }

  // allocate memory for my records changed when using UCAST
  my_records_ucast_changed = (int *) calloc (binary_header.node_number,
					     sizeof (int));
  if (my_records_ucast_changed == NULL)
    {
      WARNING ("Cannot allocate memory for my_records_ucast_changed");
      exit (1);
    }

  // allocate memory for my records when using BCAST
  my_records_bcast =
    (binary_record_class *) calloc (binary_header.node_number,
				    sizeof (binary_record_class));
  if (my_records_bcast == NULL)
    {
      WARNING ("Cannot allocate memory for my_records_bcast");
      exit (1);
    }

  // allocate memory for my records changed when using BCAST
  my_records_bcast_changed = (int *) calloc (binary_header.node_number,
					     sizeof (int));
  if (my_records_bcast_changed == NULL)
    {
      WARNING ("Cannot allocate memory for my_records_bcast_changed");
      exit (1);
    }

  // allocate memory for last byte counters
  last_byte_counters = (long long unsigned int *)
    calloc (binary_header.node_number, sizeof (long long unsigned int));
  if (last_byte_counters == NULL)
    {
      WARNING ("Cannot allocate memory for last_byte_counters");
      exit (1);
    }

  // allocate memory for last pkt counters
  last_pkt_counters = (long long unsigned int *)
    calloc (binary_header.node_number, sizeof (long long unsigned int));
  if (last_pkt_counters == NULL)
    {
      WARNING ("Cannot allocate memory for last_pkt_counters");
      exit (1);
    }

  // Lan: allocate memory for last byte counters per source
  last_byte_counters_per_source = (long long unsigned int *)
    calloc (binary_header.node_number, sizeof (long long unsigned int));
  if (last_byte_counters_per_source == NULL)
    {
      WARNING ("Cannot allocate memory for last_byte_counters_per_source");
      exit (1);
    }

  // Lan: allocate memory for last pkt counters
  last_pkt_counters_per_source = (long long unsigned int *)
    calloc (binary_header.node_number, sizeof (long long unsigned int));
  if (last_pkt_counters_per_source == NULL)
    {
      WARNING ("Cannot allocate memory for last_pkt_counters per source");
      exit (1);
    }

  // allocate memory for avg frame size array
  avg_frame_sizes = (float *)
    calloc (binary_header.node_number, sizeof (float));
  if (avg_frame_sizes == NULL)
    {
      WARNING ("Cannot allocate memory for avg_frame_sizes");
      exit (1);
    }

  for (pkt_i = 0; pkt_i < binary_header.node_number; pkt_i++)
    {
      last_pkt_counters[pkt_i] = 0;
      last_byte_counters[pkt_i] = 0;
      last_pkt_counters_per_source[pkt_i] = 0;
      last_byte_counters_per_source[pkt_i] = 0;
      avg_frame_sizes[pkt_i] = DEFAULT_FRAME_SIZE;
    }

// Lan: allocate memory for channel utilization per source
  channel_utilization_per_source = (float *)
    calloc (MAX_NODES_W, sizeof (float));
  if (channel_utilization_per_source == NULL)
    {
      WARNING ("Cannot allocate memory for channel_utilization_per_source");
      exit (1);
    }
  // Init value for channel utilixation per source
  int ch_util_i;
  for (ch_util_i = 0; ch_util_i < MAX_NODES_W; ch_util_i++)
    {
      channel_utilization_per_source[ch_util_i] = 0.0;
    }


  //////////////////////////////////////////////////
  // Start experiment
   // -----------------codes are added by ntrtrung---------------------//
  //---------------------------------Init UDP client--------------//
  int server_fd_do_action;
  int server_do_action_connected;
  server_do_action_connected = 0;
  server_fd_do_action = init_udp_client();
  if(server_fd_do_action >0)
	server_do_action_connected = 1;
  else
	printf("\nCannot Init UDP Client\n");
  //----------------------------------------------------------------------//
  
  // get the time at the beginning of the experiment
  gettimeofday (&tp_begin, NULL);
   
    //--------codes are added by ntrtrung--------//
  float this_time,last_time_for_do_action,last_time_for_broad_envi;
  last_time_for_do_action=-1;
  last_time_for_broad_envi=-1;
  //--------codes are added by ntrtrung--------//
  // loop through all time records
  for (time_i = 0; time_i < binary_header.time_record_number; time_i++)
    {
      // records counter
      int rec_i;

      // read input data from QOMET output file
      INFO ("Reading QOMET data from file...");

      // read time record
      if (io_read_binary_time_record_from_file (&binary_time_record,
						qomet_fd) == ERROR)
	{
	  WARNING ("Aborting on input error (time record)");
	  exit (1);
	}
      io_binary_print_time_record (&binary_time_record);

      // check if time record must be skipped
      if (time_i != 0
	  && binary_time_record.time < crt_record_time + time_period)
	{
	  INFO ("Skipping time record for %.2f since <time_period> = %.2f",
		binary_time_record.time, time_period);
	  continue;
	}

      // get time from record
      crt_record_time = binary_time_record.time;

      // check number of records to be read doesn't exceed the maximum
      if (binary_time_record.record_number > binary_records_max_count)
	{
	  WARNING
	    ("The number of records to be read exceeds allocated size (%d)",
	     binary_records_max_count);
	  exit (1);
	}

      // read binary records in an array
      if (io_read_binary_records_from_file (binary_records,
					    binary_time_record.record_number,
					    qomet_fd) == ERROR)
	{
	  WARNING ("Aborting on input error (records)");
	  exit (1);
	}

      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // loop is not needed if we assume from_node and ids both start from 0;
      // in that case the correct index can be computed from my_id,
      // node_count and from_node or to_node ids
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
       //--------codes are added by ntrtrung--------//
     this_time = crt_record_time;
	 if(server_do_action_connected == 1)
	 {
		if(this_time != last_time_for_do_action && server_fd_do_action > 0)
		{
			if( SendDataToServer(&this_time,server_fd_do_action,DO_ACTION_SERVER_PORT) == -1)
			{
				server_do_action_connected = 0;
			}
			else
		 	   printf("\ntime is sent to do action server:%f \n",this_time);
			if( SendDataToServer(&this_time,server_fd_do_action,BROAD_ENVI_SERVER_PORT) == -1)
			{
				server_do_action_connected = 0;
			}
			else
			   printf("\ntime is sent to broad envi server:%f \n",this_time);

			last_time_for_do_action = this_time;
		}
	 }
	
	 //--------codes are added by ntrtrung--------//
	 
	 

      // loop through all binary records
      for (rec_i = 0; rec_i < binary_time_record.record_number; rec_i++)
	{
	  // check whether the from_node matches my id
	  // (data will be used in UCAST configuration)
	  /*
	     if (binary_records[rec_i].from_node == my_id)
	     {
	   */
	  // save all records!!!!!!!!!!!11
	  io_binary_print_record (&(binary_records[rec_i]));

	  // check source is in valid range
	  if (binary_records[rec_i].from_node < 0
	      || (binary_records[rec_i].from_node
		  > binary_header.node_number - 1))
	    {
	      INFO ("Destination with id = %d is out of the valid \
range [%d, %d]", binary_records[rec_i].to_node, 0, 
		    binary_header.node_number - 1);
	      exit (1);
	    }

	  // check destination is in valid range
	  if (binary_records[rec_i].to_node < 0
	      || (binary_records[rec_i].to_node
		  > binary_header.node_number - 1))
	    {
	      INFO ("Destination with id = %d is out of the valid \
range [%d, %d]", binary_records[rec_i].to_node, 0, 
		    binary_header.node_number - 1);
	      exit (1);
	    }

	  // copy records to the appropriate storage slot,
	  // and set the changed flags
	  io_binary_copy_record
	    (&(my_records_ucast[binary_records[rec_i].from_node]
	       [binary_records[rec_i].to_node]), &binary_records[rec_i]);
	  my_records_ucast_changed[binary_records[rec_i].to_node] = TRUE;
	  /*
	     }
	   */

	  // check whether the to_node matches my id
	  // (data will be used in BCAST configuration)
	  if (binary_records[rec_i].to_node == my_id)
	    {
	      io_binary_print_record (&(binary_records[rec_i]));

	      // check source is in valid range
	      if (binary_records[rec_i].from_node < 0
		  || (binary_records[rec_i].from_node
		      > binary_header.node_number - 1))
		{
		  INFO ("Source with id = %d is out of the valid \
range [%d, %d]", binary_records[rec_i].from_node, 0, 
			binary_header.node_number - 1);
		  exit (1);
		}

	      // copy records to the appropriate storage slot,
	      // and set the changed flags
	      io_binary_copy_record
		(&(my_records_bcast[binary_records[rec_i].from_node]),
		 &binary_records[rec_i]);
	      my_records_bcast_changed[binary_records[rec_i].from_node] =
		TRUE;
	    }
	}

      // when there are no records, conditions could still change if the
      // channel utilization for other nodes changed => could check this
      // and only go further if any change took place
      if (binary_time_record.record_number == 0)
	{
	  // NOT IMPLEMENTED YET
	}

      // if it is the first iteration, copy values to adjusted array
      if (time_i == 0)
	{
	  // copy deltaQ
	  for (rec_i = 0; rec_i < node_count; rec_i++)
	    // do not consider the node itself
	    if (rec_i != my_id)
	      //copy first all fields of the record
	      io_binary_copy_record (&(adjusted_records_ucast[rec_i]),
				     &(my_records_ucast[my_id][rec_i]));
	}
      // if it is not the first iteration
      else
	// adjust deltaQ parameters given the real traffic conditions
	// of the _other_nodes_ as computed at the previous step
	// (contention from others), as well as the self contention
	// caused by the node itself;
	// adjusted_records_ucast will contain the output data

	// TEMPORARY!!!!!!!!!!!!!!!!!!!!!!!!!
	// comment out the two lines below to disable deltaQ adjustment
	adjust_deltaQ (&wireconf, my_records_ucast,
		       adjusted_records_ucast, my_records_ucast_changed,
		       avg_frame_sizes);


      // waiting for the next timer deadline event;
      // if this is the first operation we reset the timer,
      // and proceed without waiting
      if (time_i == 0)
	timer_reset (timer);
      else
	{
	  if (SCALING_FACTOR == 1.0)
	    INFO ("Waiting to reach time %.2f s...", crt_record_time);
	  else
	    INFO ("Waiting to reach real time %.2f s (scenario \
time %.2f)...", crt_record_time * SCALING_FACTOR, crt_record_time);

	  // wait for for the next timer event
	  if (timer_wait (timer, crt_record_time * 1000000
			  * SCALING_FACTOR) < 0)
	    {
	      WARNING ("Timer deadline missed at time=%.2f s",
		       crt_record_time);
	    }
	}


      channel_utilization_sent = FALSE;


      // when timer event comes, apply configuration for all 
      // the unicast links with destination 'node_i'
      // NOTE: GETTING NEXT HOP COULD BE DONE WHILE WAITING
      for (node_i = FIRST_NODE_ID; node_i < (node_count + FIRST_NODE_ID);
	   node_i++)
	{
	  // neigbour's IP address as string
	  char neighbour_ip_addr_char[IP_ADDR_SIZE];

	  // id of the next hop
	  int next_hop_id;


	  // skip current node
	  if (node_i == my_id)
	    // Lan optimization for the routing experiment
	    //if ( (j != 1) || (i != 2) )
	    continue;

	  // get the next hop ID for destination node_i
	  // to find correct configuration lines
	  if ((next_hop_id =
	       get_next_hop_id (IP_addresses, IP_char_addresses, node_i,
				DIRECTION_OUT)) == ERROR)
	    {
	      WARNING ("Could not locate the next hop for \
destination node %i", node_i);
	      exit (1);
	    }
	  else
	    INFO ("Next_hop=%i for destination=%i", next_hop_id, node_i);

	  if (next_hop_id < 0
	      || (next_hop_id > binary_header.node_number - 1))
	    {
	      INFO ("Next hop with id = %d is out of the valid \
range [%d, %d]", binary_records[rec_i].to_node, 0, 
		    binary_header.node_number - 1);
	      exit (1);
	    }

	  // save next_hop_id
	  next_hop_ids[node_i] = next_hop_id;

	  // check if experiment should use predefined conditions
	  if (PREDEFINED_EXPERIMENT == TRUE)
	    {
	      bandwidth = BANDWIDTH;
	      delay = DELAY;
	      loss_rate = PACKET_LOSS_RATE;
	    }
	  else
	    {
	      /* IGNORE FLAG FOR THE MOMENT

	         // if the parameters for the next hop did not change, 
		 // there is no need to proceed any further
	         if (my_records_ucast_changed[next_hop_id] == FALSE)
	         continue;
	         // otherwise, set the flag to FALSE and proceed 
	         else
	         my_records_ucast_changed[next_hop_id] = FALSE;
	       */

	      bandwidth = adjusted_records_ucast[next_hop_id].bandwidth;
	      delay = adjusted_records_ucast[next_hop_id].delay;
	      loss_rate = adjusted_records_ucast[next_hop_id].loss_rate;
	    }

	  // print current configuration info
	  INFO ("* Wireconf pipe=%d: #%d UCAST to #%d (next_hop_id=%d) [%s] \
(time=%.2f s): bandwidth=%.2fbit/s loss_rate=%.4f delay=%.4f ms", 
		MIN_PIPE_ID_OUT + node_i, my_id, node_i, next_hop_id, 
		IP_char_addresses + (node_i - FIRST_NODE_ID) * IP_ADDR_SIZE, 
		crt_record_time, bandwidth, loss_rate, delay);
	  configure_pipe (dummynet_socket_id, MIN_PIPE_ID_OUT + node_i,
			  bandwidth, delay, loss_rate, PIPE_QSIZE);


	  /////////////////////////////////////////////////
	  // IPC communication

	  // if loss rate to node_i is 1.0, we skip this node
	  // when communicating with OLSR
	  if (adjusted_records_ucast[node_i].loss_rate >= 1.0)
	    continue;

	  // convert id to IP address
	  if (get_ip_from_id (IP_char_addresses, node_i,
			      neighbour_ip_addr_char) != ERROR)
	    {
	      char time_string[20];

	      // get time information
	      ipc_time (time_string);

	      // print link information
	      INFO ("IPC write: time=%s qomet_time=%f node_i=%d node_i_IP=%s ;\
 OP=%0.1f", time_string, crt_record_time, node_i, neighbour_ip_addr_char, 
		    adjusted_records_ucast[node_i].operating_rate / 1e6);

	      // write link information to shared memory
	      // (bandwidth is converted to int value in Mbps)
	      ipc_link_write
		(qomet_link_ipc, node_i, neighbour_ip_addr_char,
		 (int) (adjusted_records_ucast[node_i].operating_rate / 1e6));

	      if (channel_utilization_sent == FALSE)
		{
		  //write channel information
		  /*
		     INFO("IPC write: time=%s qomet_time=%f channel_id=%d \
		     ch_util=%0.2f", time_string, crt_record_time, (int) CHANNEL_ID, 
		     wireconf.total_channel_utilization_others);
		   */

		  // new channel utilization for CWB
		  // Lan added on Oct. 01 to send cwb_channel_util over IPC
		  cwb_channel_utilization =
		    compute_cwb_channel_utilization (&wireconf,
						     adjusted_records_ucast);
		  INFO ("IPC write: time=%s qomet_time=%f channel_id=%d \
ch_util=%0.2f", time_string, crt_record_time, (int) CHANNEL_ID, cwb_channel_utilization);

		  ipc_channel_write (qomet_channel_ipc, (int) CHANNEL_ID,
				     cwb_channel_utilization);
		  //ipc_channel_write(qomet_channel_ipc,(int) CHANNEL_ID, 
		  //                wireconf.total_channel_utilization_others); 

		  channel_utilization_sent = TRUE;
		}
	    }
	  else
	    fprintf (stderr, "WARNING: do_wireconf: Cannot find IP from \
id=%d\n", binary_records[rec_i].to_node);

	  // end IPC communication

	}			// end for loop unicast


      // config the broadcast links have source node id = i
      for (node_i = FIRST_NODE_ID; node_i < (node_count + FIRST_NODE_ID);
	   node_i++)
	{
	  if (node_i == my_id)
	    // Lan optimization for the routing experiment
	    //if ( (j != 1) || (i != 2) )
	    continue;

	  // check if experiment should use predefined conditions
	  if (PREDEFINED_EXPERIMENT == TRUE)
	    {
	      bandwidth = BANDWIDTH;
	      delay = DELAY;
	      loss_rate = PACKET_LOSS_RATE;
	    }
	  else
	    {
	      /* IGNORE FLAG FOR THE MOMENT

	         // if the parameters for the next hop did not change, there is no
	         // need to proceed any further
	         if (my_records_bcast_changed[node_i] == FALSE)
	         continue;
	         // otherwise, set the flag to FALSE and proceed 
	         else
	         my_records_bcast_changed[node_i] = FALSE;
	       */

	      bandwidth = my_records_bcast[node_i].bandwidth;
	      delay = my_records_bcast[node_i].delay;
	      loss_rate = my_records_bcast[node_i].loss_rate;

	    }

	  // print current configuration info
	  INFO ("* Wireconf pipe=%d: #%d BCAST from #%d [%s] (time=%.2f s): \
bandwidth=%.2fbit/s loss_rate=%.4f delay=%.4f ms", MIN_PIPE_ID_IN_BCAST + node_i, my_id, node_i, 
		IP_char_addresses + (node_i - FIRST_NODE_ID) * IP_ADDR_SIZE, crt_record_time, 
		bandwidth, loss_rate, delay);

	  configure_pipe (dummynet_socket_id, MIN_PIPE_ID_IN_BCAST + node_i,
			  bandwidth, delay, loss_rate, PIPE_QSIZE);
	}			// end for loop for broadcast
      // end config for broadcast traffic


      rule_count = MAX_RULE_COUNT;

      // get rules and print statistics for the current rule
      if (get_rules (dummynet_socket_id, &rule_data, &rule_data_alloc_size,
		     rules, &rule_count) == SUCCESS)
	{
	  int i, node_i, node_idx;

	  long long unsigned int delta_pkt_counter, delta_byte_counter;

	  long long unsigned int delta_pkt_counter_per_source,
	    delta_byte_counter_per_source;

	  struct stats_class stats;

	  float adjusted_delta_pkt_counter;

	  my_total_channel_utilization = 0;
	  my_total_transmission_probability = 0;


	  wireconf.total_self_number_packets = 0;


	  node_i = 0;

	  // Lan: compute the channel utilization per source
	  node_idx = 0;

	  // !!!!!!!!!!!!!!1 We assume there is one rule per node,
	  // and node ids and rule ids correspond !!!!!!!!!!!!!!!
	  for (i = 0; i < rule_count; i++)
	    {
	      // filter only the rules for UCAST
	      if (rules[i]->rulenum >= MIN_PIPE_ID_OUT
		  && rules[i]->rulenum < (MIN_PIPE_ID_OUT
					  + binary_header.node_number))
		{
		  // check whether current node (node_i) is the same with
		  // the running node (my_id), case for which no records exist;
		  // increment current node index after setting values to 0
		  if (node_i == my_id)
		    {
		      wireconf.self_channel_utilizations[node_i] = 0;
		      wireconf.self_transmission_probabilities[node_i] = 0;
		      node_i++;
		    }

		  // because of check above, we know that node_i != my_id
		  printf ("my_id=%d node_i=%d Rule #%d: \
\t%6llu pkts \t%9llu bytes\n", my_id, node_i, (rules[i])->rulenum, (rules[i])->pcnt, 
			  (rules[i])->bcnt);

		  delta_pkt_counter = rules[i]->pcnt
		    - last_pkt_counters[node_i];
		  delta_byte_counter = rules[i]->bcnt
		    - last_byte_counters[node_i];

		  last_pkt_counters[node_i] = rules[i]->pcnt;
		  last_byte_counters[node_i] = rules[i]->bcnt;

		  // update avg frame size statistics if packets exist
		  if (delta_pkt_counter > 0)
		    avg_frame_sizes[node_i] = (float) delta_byte_counter
		      / (float) delta_pkt_counter;

		  /*
		     wireconf.self_channel_utilizations[node_i] = 
		     compute_channel_utilization
		     (&(my_records_ucast[node_i]), delta_pkt_counter, 
		     delta_byte_counter);
		   */
		  wireconf.self_channel_utilizations[node_i] =
		    compute_channel_utilization
		    (&(adjusted_records_ucast[next_hop_ids[node_i]]),
		     delta_pkt_counter, delta_byte_counter,
		     0.5 /*!!!!!!!!!!!!!!!!!!!!! */ );


		  wireconf.self_transmission_probabilities[node_i] =
		    compute_transmission_probability
		    (&(adjusted_records_ucast[next_hop_ids[node_i]]),
		     delta_pkt_counter, delta_byte_counter,
		     0.5 /*!!!!!!!!!!!!!!!!!!!!! */ ,
		     &adjusted_delta_pkt_counter);

		  wireconf.total_self_number_packets
		    += adjusted_delta_pkt_counter;

		  /*
		     printf("my_total_channel_utilization=%f new util=%f\n", 
		     my_total_channel_utilization, 
		     my_channel_utilizations[i]);
		   */
		  my_total_channel_utilization +=
		    wireconf.self_channel_utilizations[node_i];
		  my_total_transmission_probability +=
		    wireconf.self_transmission_probabilities[node_i];

		  // go to next node_i
		  node_i++;
		}


	      if (rules[i]->rulenum >= MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET
		  && rules[i]->rulenum < (MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET
					  + binary_header.node_number))
		{
		  // check whether the source (node_idx) is the different than
		  // the destination (GW_ID), because no records exist for 
		  // same index pairs; increment source index
		  //if ( node_idx != my_id)
		  if (node_idx != GW_ID)
		    {
		      // because of above check, we know that source is not
		      // equal to destination
		      printf ("CWB: my_id=%d node_idx=%d (source) Rule #%d: \
    \t%6llu pkts \t%9llu bytes\n", my_id, node_idx, (rules[i])->rulenum, (rules[i])->pcnt, 
			      (rules[i])->bcnt);

		      delta_pkt_counter_per_source = rules[i]->pcnt
			- last_pkt_counters_per_source[node_idx];
		      delta_byte_counter_per_source = rules[i]->bcnt
			- last_byte_counters_per_source[node_idx];

		      last_pkt_counters_per_source[node_idx] = rules[i]->pcnt;
		      last_byte_counters_per_source[node_idx] =
			rules[i]->bcnt;

		      channel_utilization_per_source[node_idx] =
			compute_channel_utilization
			(&(adjusted_records_ucast[next_hop_ids[GW_ID]]),
			 delta_pkt_counter_per_source,
			 delta_byte_counter_per_source,
			 0.5 /*!!!!!!!!!!!!!!!!!!!!! */ );
		    }
		  node_idx++;
		}		// end if: computing per source channel utilization

	    }			//end loop

	  printf ("my_total_channel_utilization=%f\n",
		  my_total_channel_utilization);
	  if (my_total_channel_utilization > 1.0)
	    WARNING ("Total channel utilization exceeds 1.0");
	  if (my_total_transmission_probability > 1.0)
	    WARNING ("Total my_total_transmission_probability exceeds 1.0");

	  stats.channel_utilization = my_total_channel_utilization;
	  stats.transmission_probability = my_total_transmission_probability;

	  // Lan: assign value for channel utilization per source
	  for (i = 0; i < MAX_NODES_W; i++)
	    {
	      stats.channel_utilization_per_source[i] =
		channel_utilization_per_source[i];
	    }

	  wireconf_deliver_stats (&wireconf, &stats);
	}
      else
	WARNING ("Cannot obtain rule information");

      // increase loop counter
      loop_count++;

      if (PREDEFINED_EXPERIMENT == TRUE)
	if (loop_count > LOOP_COUNT)
	  break;
    }


  ///////////////////////////////////////////////
  // End experiment

  // get the time at the end of the experiment
  gettimeofday (&tp_end, NULL);
    
  //--------------codes are added by ntrtrung-------------------------//
  float time_out = -1;
  if(server_do_action_connected == 1)
  {
	  SendDataToServer(&time_out,server_fd_do_action,DO_ACTION_SERVER_PORT);
	  SendDataToServer(&time_out,server_fd_do_action,BROAD_ENVI_SERVER_PORT);
	  sleep(1);
	  close(server_fd_do_action);
  }
  //--------------codes are added by ntrtrung-------------------------//

  // print execution time
  INFO ("Experiment execution time=%.4f s",
	(tp_end.tv_sec + tp_end.tv_usec / 1.0e6) -
	(tp_begin.tv_sec + tp_begin.tv_usec / 1.0e6));

  // check unexpected state
  if (time_i >= binary_header.time_record_number)
    {
      INFO ("Finished reading configuration file");
      fflush (stdout);
    }
  else
    INFO ("Unknown problem");

  // if the loop was not entered not even once
  // there is an error
  if (loop_count == 0)
    {
      if (usage_type == 1)
	WARNING ("The specified pair from_node_id=%d & to_node_id=%d "
		 "could not be found", from_id, to_id);
      else
	WARNING ("No valid line was found for the node %d", my_id);
    }

  // release the timer
  timer_free (timer);


  ///////////////////////////////////////////////
  // Delete dummynet rules/pipes

  INFO ("Deleting rules and pipes from dummynet...");

  // usage (1) => delete the current dummynet rule
  if (usage_type == 1)
    {
      DEBUG ("Deleting rule #%d...", rule_number);
      if (delete_rule (dummynet_socket_id, rule_number) == ERROR)
	{
	  WARNING ("Could not delete rule #%d", rule_number);
	  //exit (1);
	}

      DEBUG ("Deleting pipe #%d...", pipe_number);
      if (delete_pipe (dummynet_socket_id, pipe_number) == ERROR)
	{
	  WARNING ("Could not delete pipe #%d", pipe_number);
	  //exit (1);
	}
    }
  // usage (2) => delete multiple rules
  else
    {
      int j;

      // delete the unicast dummynet rules
      for (j = FIRST_NODE_ID; j < node_count + FIRST_NODE_ID; j++)
	{
	  if (j == my_id)
	    continue;

	  DEBUG ("Deleting rule #%d...", MIN_PIPE_ID_OUT + j);
	  if (delete_rule (dummynet_socket_id, MIN_PIPE_ID_OUT + j) == ERROR)
	    {
	      WARNING ("Could not delete rule #%d", MIN_PIPE_ID_OUT + j);
	    }
	  DEBUG ("Deleting pipe #%d...", MIN_PIPE_ID_OUT + j);
	  if (delete_pipe (dummynet_socket_id, MIN_PIPE_ID_OUT + j) == ERROR)
	    {
	      WARNING ("Could not delete pipe #%d", MIN_PIPE_ID_OUT + j);
	    }
	}

      // Added by Lan: delete the unicast dummynet rules for statistic of CWB
      for (j = FIRST_NODE_ID; j < node_count + FIRST_NODE_ID; j++)
	{
	  DEBUG ("Deleting rule #%d...",
		 MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j);
	  if (delete_rule
	      (dummynet_socket_id,
	       MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j) == ERROR)
	    {
	      WARNING ("Could not delete rule #%d",
		       MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j);
	    }
	  DEBUG ("Deleting pipe #%d...",
		 MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j);
	  if (delete_pipe (dummynet_socket_id,
			   MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j) == ERROR)
	    {
	      WARNING ("Could not delete pipe #%d",
		       MIN_PIPE_ID_OUT + CWB_PIPE_ID_OFFSET + j);
	    }
	}

      //delete broadcast dummynet rules
      for (j = FIRST_NODE_ID; j < node_count + FIRST_NODE_ID; j++)
	{
	  if (j == my_id)
	    continue;

	  DEBUG ("Deleting rule #%d...", MIN_PIPE_ID_IN_BCAST + j);
	  if (delete_rule (dummynet_socket_id, MIN_PIPE_ID_IN_BCAST + j)
	      == ERROR)
	    {
	      WARNING ("Could not delete rule #%d", MIN_PIPE_ID_IN_BCAST + j);
	    }
	  DEBUG ("Deleting pipe #%d...", MIN_PIPE_ID_IN_BCAST + j);
	  if (delete_pipe (dummynet_socket_id, MIN_PIPE_ID_IN_BCAST + j)
	      == ERROR)
	    {
	      WARNING ("Could not delete pipe #%d", MIN_PIPE_ID_IN_BCAST + j);
	    }
	}
    }


  // close dummynet socket
  DEBUG ("Closing dummynet socket...");
  close_socket (dummynet_socket_id);

  /* Added by Lan on Apr 4 for CWB */
  // before removing shared mem, try to signal the reading process
  ipc_link_write (qomet_link_ipc, 0, IP_char_addresses, (int) -2);
  ipc_channel_write (qomet_channel_ipc, 0, (float) -2.0);

  // remove shared memory
  fprintf (stderr, "Removing link shared mem...");
  shmdt (qomet_link_ipc);
  if (ipc_remove (IPC_LINK_NAME) == SUCCESS)
    printf ("Successfully\n");
  else
    printf ("Fail\n");

  fprintf (stderr, "Removing channel shared mem...");
  shmdt (qomet_channel_ipc);
  if (ipc_remove (IPC_CHANNEL_NAME) == SUCCESS)
    fprintf (stderr, "Successfully\n");
  else
    fprintf (stderr, "Fail\n");

  // remove file for shared mem 
  if (ipc_remove_file () == ERROR)
    exit (1);
  /* Added by Lan on Apr 4 for CWB */

  // finalize wireconf library
  if (wireconf_finalize (&wireconf) == ERROR)
    {
      WARNING ("Could not finalize wireconf library");
      exit (1);
    }

  // free allocated memory

  if (rule_data != NULL)
    free (rule_data);

  if (binary_records != NULL)
    free (binary_records);

  if (next_hop_ids != NULL)
    free (next_hop_ids);

  for (node_i = 0; node_i < binary_header.node_number; node_i++)
    if (my_records_ucast[node_i] != NULL)
      free (my_records_ucast[node_i]);
  if (my_records_ucast != NULL)
    free (my_records_ucast);

  if (my_records_ucast_changed != NULL)
    free (my_records_ucast_changed);

  if (my_records_bcast != NULL)
    free (my_records_bcast);

  if (my_records_bcast_changed != NULL)
    free (my_records_bcast_changed);

  if (last_byte_counters != NULL)
    free (last_byte_counters);

  if (last_pkt_counters != NULL)
    free (last_pkt_counters);

  if (avg_frame_sizes != NULL)
    free (avg_frame_sizes);

  if (channel_utilization_per_source != NULL)
    free (channel_utilization_per_source);

  // close file descriptor
  fclose (qomet_fd);

  return 0;
}

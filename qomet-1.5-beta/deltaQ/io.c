
/*
 * Copyright (c) 2006-2009 The StarBED Project  All rights reserved.
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
 * File name: io.c
 * Function: I/O functionality for deltaQ library
 *
 * Author: Razvan Beuran
 *
 *   $Revision: 140 $
 *   $LastChangedDate: 2009-03-26 10:41:59 +0900 (Thu, 26 Mar 2009) $
 *   $LastChangedBy: razvan $
 *
 ***********************************************************************/

#include <stdio.h>
#include <math.h>

#include "io.h"
#include "global.h"
#include "message.h"


#define GRID_SIZE_X            300.0
#define GRID_SIZE_Y            300.0
#define NODE_RADIUS            5.0


////////////////////////////////////////////////
// Text I/O functions
////////////////////////////////////////////////

// write the header of the file in which connection description will be stored
void io_write_header_to_file(FILE *file_global, char *qomet_name)
{
  fprintf(file_global, "%% Output generated by %s\n", qomet_name);
  fprintf(file_global, "%% time from_node_id from_node_x from_node_y \
from_node_z to_node_id to_node_x to_node_y to_node_z distance Pr SNR FER \
num_retr op_rate bandwidth loss_rate delay jitter\n");
}

// write connection description to file
void io_write_to_file(connection_class *connection,
		      scenario_class *scenario, double time, 
		      int cartesian_coord_syst,
		      FILE *file_global)
{
  node_class *from_node, *to_node;

  coordinate_class saved_from, saved_to;
  coordinate_class point_blh;

  from_node = &(scenario->nodes[connection->from_node_index]);
  to_node = &(scenario->nodes[connection->to_node_index]);

  // if coordinate system is not cartesian, we convert
  // x & y to latitude & longitude before storing
  if(cartesian_coord_syst == FALSE)
    {
      coordinate_copy(&saved_from, &(from_node->position));
      coordinate_copy(&saved_to, &(to_node->position));

      // transform from_node x & y to lat & long
      en2ll(&(from_node->position), &point_blh);
      coordinate_copy(&(from_node->position), &point_blh);

      // transform to_node x & y to lat & long
      en2ll(&(to_node->position), &point_blh);
      coordinate_copy(&(to_node->position), &point_blh);
    }

  // write current connection description to file
  fprintf(file_global, "%.2f %d %.6f %.6f %.6f %d %.6f %.6f %.6f %.4f \
%.4f %.4f %.4f %.2f %.2f %.2f %.4f %.4f %.4f\n", time, from_node->id, 
	  from_node->position.c[0], from_node->position.c[1], 
	  from_node->position.c[2], to_node->id, 
	  to_node->position.c[0], to_node->position.c[1], 
	  to_node->position.c[2], connection->distance, connection->Pr, 
	  connection->SNR, connection->frame_error_rate, 
	  connection->num_retransmissions, 
	  connection_operating_rate(connection), connection->bandwidth, 
	  connection->loss_rate, connection->delay, connection->jitter);

  // restore coordinates
  if(cartesian_coord_syst == FALSE)
    {
      coordinate_copy(&(from_node->position), &saved_from);
      coordinate_copy(&(to_node->position), &saved_to);
    }
}

// write header of motion file in NAM format;
// return SUCCESS on succes, ERROR on error
int io_write_nam_motion_header_to_file(scenario_class *scenario, 
				       FILE *motion_file)
{
  int node_i;
  node_class *node;

  //double saved_node_x, saved_node_y;

  // write comment
  fprintf(motion_file, "# NAM trace movement file generated by QOMET\n");

  // write version of nam
  fprintf(motion_file, "%s\n", "V -t * -v 1.0a5 -a 0");

  // write scenario simulation size
  fprintf(motion_file, "W -t * -x %.2f -y %.2f\n", GRID_SIZE_X, GRID_SIZE_Y);

  // write node initial position and display parameters
  for(node_i=0; node_i<scenario->node_number; node_i++)
    {
      node=&(scenario->nodes[node_i]);

      // should convert to ll before output, but coordinates
      // would become too small!!!!!!!!!!!!!!11

      // if coordinate system is not cartesian, we convert
      // x & y to latitude & longitude before storing
      /*
      if(cartesian_coord_syst == FALSE)
	{
	}
      */

      // write data for current node
      fprintf(motion_file, 
	      "n -t * -s %d -x %.9f -y %.9f -z %f -v circle -c black -w\n", 
	      node->id, node->position.c[0], node->position.c[1], NODE_RADIUS);

      /*
      // restore coordinates
      if(cartesian_coord_syst == FALSE)
	{
	}
      */
    }

  return SUCCESS;
}

// write motion file information in NAM format;
// return SUCCESS on succes, ERROR on error
int io_write_nam_motion_info_to_file(scenario_class *scenario, 
				 FILE *motion_file, float time)
{
  int node_i;
  node_class *node;

  // write current positions of nodes
  for(node_i=0; node_i<scenario->node_number; node_i++)
    {
      node=&(scenario->nodes[node_i]);
      fprintf(motion_file, "n -t %.2f -s %d -x %.9f -y %.9f\n", time, 
	      node->id, node->position.c[0], node->position.c[1]);
    }

  return SUCCESS;
}

// write header of motion file in NS-2 format;
// return SUCCESS on succes, ERROR on error
int io_write_ns2_motion_header_to_file(scenario_class *scenario, 
				       FILE *motion_file)
{
  int node_i;
  node_class *node;

  //double saved_node_x, saved_node_y;

  // write comment
  fprintf(motion_file, "# NS-2 node movement file generated by QOMET\n");

  // write node initial position and display parameters
  for(node_i=0; node_i<scenario->node_number; node_i++)
    {
      node=&(scenario->nodes[node_i]);

      // should convert to ll before output, but coordinates
      // would become too small!!!!!!!!!!!!!!11

      // if coordinate system is not cartesian, we convert
      // x & y to latitude & longitude before storing
      /*
      if(cartesian_coord_syst == FALSE)
	{
	}
      */

      // write data for current node
      fprintf(motion_file, 
	      "$node_(%d) set X_ %.9f\n$node_(%d) set Y_ %.9f\n$node_(%d) \
set Z_ %.9f\n", node->id, node->position.c[0], node->id, node->position.c[1], 
	      node->id, node->position.c[2]);

      /*
      // restore coordinates
      if(cartesian_coord_syst == FALSE)
	{
	}
      */
    }

  return SUCCESS;
}

// write motion file information in NS-2 format;
// return SUCCESS on succes, ERROR on error
int io_write_ns2_motion_info_to_file(scenario_class *scenario, 
				     FILE *motion_file, float time)
{
  int node_i;
  node_class *node;

  // write current positions of nodes
  for(node_i=0; node_i<scenario->node_number; node_i++)
    {
      node=&(scenario->nodes[node_i]);
      fprintf(motion_file, "$ns_ at %.3f \"$node_(%d) setdest %.9f %.9f \
%.3f\"\n", time, node->id, node->position.c[0], node->position.c[1],
	      DEFAULT_NS2_SPEED);
    }

  return SUCCESS;
}


////////////////////////////////////////////////
// Binary I/O functions
////////////////////////////////////////////////

// print binary header
void io_binary_print_header(binary_header_class *binary_header)
{
  // print signature (only first 3 characters)
  printf("Header signature: %c%c%c\n", binary_header->signature[0],
         binary_header->signature[1], binary_header->signature[2]);

  printf("Version information: %d.%d.%d (revision %d)\n", 
	 binary_header->major_version, binary_header->minor_version, 
	 binary_header->subminor_version, binary_header->svn_revision);
  printf("Number of time records: %ld\n", binary_header->time_record_number);
}

// print binary time record
void io_binary_print_time_record(binary_time_record_class *binary_time_record)
{
  printf("> Time: %f s (%d records)\n", binary_time_record->time, 
	 binary_time_record->record_number);
}

// print binary record
void io_binary_print_record(binary_record_class *binary_record)
{
  printf("> > Record: from_node=%d to_node=%d loss_rate=%f\n", 
	 binary_record->from_node, binary_record->to_node,
	 binary_record->loss_rate);
}

// build binary record
void io_binary_build_record(binary_record_class *binary_record, 
			    connection_class *connection,
			    scenario_class *scenario)
{
  binary_record->from_node = 
    (scenario->nodes[connection->from_node_index]).id;
  binary_record->to_node =
    (scenario->nodes[connection->to_node_index]).id;
  binary_record->loss_rate = connection->loss_rate;
}

// compare with binary record;
// return TRUE if data is same with the one in the record,
// FALSE otherwise
int io_binary_compare_record(binary_record_class *binary_record, 
			     connection_class *connection,
			     scenario_class *scenario)
{
  if((binary_record->from_node == 
      (scenario->nodes[connection->from_node_index]).id) &&
     (binary_record->to_node ==
      (scenario->nodes[connection->to_node_index]).id) &&
     (fabs(binary_record->loss_rate - connection->loss_rate) < EPSILON))
    return TRUE;
  else
    return FALSE;
}

// read header of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_read_binary_header_from_file(binary_header_class *binary_header,
				    FILE *binary_input_file)
{
  // read header from file
  if(fread(binary_header, sizeof(binary_header_class),
	   1, binary_input_file)!=1)
    {
      INFO("Error reading binary header from file");
      return ERROR;
    }

  // check signature (DEFINE SOMEWHERE IN HEADER FILE...)
  if(!(binary_header->signature[0]=='Q' &&
       binary_header->signature[1]=='M' &&
       binary_header->signature[2]=='T' &&
       binary_header->signature[3]=='\0'))
    {
      INFO("Incorrect signature in binary file");
      return ERROR;
    }

  return SUCCESS;
}

// write header of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_write_binary_header_to_file(long int time_record_number, 
				   int major_version,
				   int minor_version,
				   int subminor_version,
				   int svn_revision,
				   FILE *binary_file)
{
  binary_header_class binary_header;

  binary_header.signature[0]='Q';
  binary_header.signature[1]='M';
  binary_header.signature[2]='T';
  binary_header.signature[3]='\0';
    
  binary_header.major_version = major_version;
  binary_header.minor_version = minor_version;
  binary_header.subminor_version = subminor_version;
  binary_header.svn_revision = svn_revision;

  binary_header.reserved[0]=0xde;
  binary_header.reserved[1]=0xad;
  binary_header.reserved[2]=0xbe;
  binary_header.reserved[3]=0xef;

  binary_header.time_record_number = time_record_number;

  // write header to file
  if(fwrite(&binary_header, sizeof(binary_header_class),
	    1, binary_file)!=1)
    {
      WARNING("Error writing binary output header to file");
      return ERROR;
    }

  return SUCCESS;
}

// read a time record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_read_binary_time_record_from_file(binary_time_record_class *
					 binary_time_record,
					 FILE *binary_input_file)
{
  // read time record from file
  if(fread(binary_time_record, sizeof(binary_time_record_class),
	   1, binary_input_file)!=1)
    {
      INFO("Error reading binary time record from file");
      return ERROR;
    }

  return SUCCESS;
}

// write a time record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_write_binary_time_record_to_file(float time, int record_number,
					FILE *binary_file)
{
  binary_time_record_class binary_time_record;

  binary_time_record.time = time;
  binary_time_record.record_number = record_number;

  // write time record to file
  if(fwrite(&binary_time_record, 
	    sizeof(binary_time_record_class), 1, binary_file)!=1)
    {
      WARNING("Error writing binary output time record to file");
      return ERROR;
    }

  return SUCCESS;
}

// directly write a time record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_write_binary_time_record_to_file2(binary_time_record_class *
					 binary_time_record, FILE *binary_file)
{
  // write time record to file
  if(fwrite(binary_time_record, 
	    sizeof(binary_time_record_class), 1, binary_file)!=1)
    {
      WARNING("Error writing binary output time record to file");
      return ERROR;
    }

  return SUCCESS;
}

// read a record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_read_binary_record_from_file(binary_record_class *binary_record,
				    FILE *binary_input_file)
{
  // record from file
  if(fread(binary_record, sizeof(binary_record_class),
	   1, binary_input_file)!=1)
    {
      INFO("Error reading binary record from file");
      return ERROR;
    }

  return SUCCESS;
}

// read 'number_records' records from a QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_read_binary_records_from_file(binary_record_class *binary_records,
				     int number_records,
				     FILE *binary_input_file)
{
  // records from file
  //printf("Reading %d records from file\n", number_records);
  //fflush(stdout);

  if(fread(binary_records, sizeof(binary_record_class),
	   number_records, binary_input_file)!=number_records)
    {
      INFO("Error reading binary records from file");
      return ERROR;
    }

  return SUCCESS;
}

// write a record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_write_binary_record_to_file(connection_class *connection,
				   scenario_class *scenario,
				   FILE *binary_file)
{
  binary_record_class binary_record;

  binary_record.from_node = 
    (scenario->nodes[connection->from_node_index]).id;
  binary_record.to_node =
    (scenario->nodes[connection->to_node_index]).id;
  binary_record.loss_rate = connection->loss_rate;

  //////////////////////////////////////////////////////////////////
  // NOTE: WHEN MORE FIELDS WILL BE ADDED, TAKE CARE TO CONVERT
  // X & Y COORDINATES TO LAT & LONG IF CARTESIAN SYSTEM IS NOT USED
  //////////////////////////////////////////////////////////////////

  // write record to file
  if(fwrite(&binary_record, sizeof(binary_record_class),
	    1, binary_file)!=1)
    {
      WARNING("Error writing binary output record to file");
      return ERROR;
    }

  return SUCCESS;
}

// directly write a record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_write_binary_record_to_file2(binary_record_class *binary_record,
				    FILE *binary_file)
{
  //////////////////////////////////////////////////////////////////
  // NOTE: WHEN MORE FIELDS WILL BE ADDED, TAKE CARE TO CONVERT
  // X & Y COORDINATES TO LAT & LONG IF CARTESIAN SYSTEM IS NOT USED
  //////////////////////////////////////////////////////////////////

  // write record to file
  if(fwrite(binary_record, sizeof(binary_record_class),
	    1, binary_file)!=1)
    {
      WARNING("Error writing binary output record to file");
      return ERROR;
    }

  return SUCCESS;
}

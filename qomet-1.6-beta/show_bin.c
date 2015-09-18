
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
 * File name: show_bin.c
 * Function: Display binary QOMET output
 *
 * Author: Razvan Beuran
 *
 *   $Revision: 143 $
 *   $LastChangedDate: 2009-03-31 11:30:13 +0900 (Tue, 31 Mar 2009) $
 *   $LastChangedBy: razvan $
 *
 ***********************************************************************/


#include <stdio.h>
#include <string.h>

#include "deltaQ.h"


///////////////////////////////////
// Generic variables and functions
///////////////////////////////////

#define MESSAGE_WARNING
//#define MESSAGE_DEBUG
#define MESSAGE_INFO

#ifdef MESSAGE_WARNING
#define WARNING(message...) do {                                          \
  fprintf(stderr, "show_bin WARNING: %s, line %d: ", __FILE__, __LINE__); \
  fprintf(stderr, message); fprintf(stderr,"\n");                         \
} while(0)
#else
#define WARNING(message...)	/* message */
#endif

#ifdef MESSAGE_DEBUG
#define DEBUG(message...) do {                                          \
  fprintf(stdout, "show_bin DEBUG: %s, line %d: ", __FILE__, __LINE__); \
  fprintf(stdout, message); fprintf(stdout,"\n");                       \
} while(0)
#else
#define DEBUG(message...)	/* message */
#endif

#ifdef MESSAGE_INFO
#define INFO(message...) do {                                       \
  fprintf(stdout, message); fprintf(stdout,"\n");                   \
} while(0)
#else
#define INFO(message...)	/* message */
#endif

// print usage info
static void
usage (FILE * f)
{
  fprintf (f, "\nshow_bin. Display binary QOMET output as text.\n\n");
  fprintf (f, "Usage: show_bin <scenario_file.xml.bin>\n");
  fprintf (f, "\n");
  fprintf (f, "See documentation for more details.\n");
  fprintf (f,
	   "Please send comments and bug reports to info@starbed.org.\n\n");
}

int
main (int argc, char *argv[])
{
  // binary file name and descriptor
  char bin_filename[MAX_STRING];
  FILE *bin_file;

  // binary file header data structure
  binary_header_class binary_header;

  // binary file time record data structure
  binary_time_record_class binary_time_record;

  // binary file record data structure and max count
  binary_record_class *binary_records = NULL;
  int binary_records_max_count;

  // counters for time and binary records
  long int time_i;
  int rec_i;

  // check argument count
  if (argc <= 1)
    {
      WARNING ("No binary QOMET output file was provided");
      usage (stdout);
      exit (1);
    }

  // save binary filename
  strncpy (bin_filename, argv[1], MAX_STRING - 1);

  INFO ("Showing binary QOMET output in '%s'...\n", bin_filename);

  // open binary file
  bin_file = fopen (argv[1], "r");
  if (bin_file == NULL)
    {
      WARNING ("Cannot open binary file '%s'", bin_filename);
      exit (1);
    }

  // read and check binary header
  if (io_read_binary_header_from_file (&binary_header, bin_file) == ERROR)
    {
      WARNING ("Aborting on input error (binary header)");
      // close binary file
      fclose (bin_file);

      exit (1);
    }
  io_binary_print_header (&binary_header);

  // allocate memory for binary records
  binary_records_max_count = binary_header.node_number
    * (binary_header.node_number - 1);
  binary_records =
    (binary_record_class *) calloc (binary_records_max_count,
				    sizeof (binary_record_class));
  if (binary_records == NULL)
    {
      WARNING ("Cannot allocate memory for records");
      // close binary file
      fclose (bin_file);

      exit (1);
    }

  // read all records
  for (time_i = 0; time_i < binary_header.time_record_number; time_i++)
    {
      // read time record
      if (io_read_binary_time_record_from_file (&binary_time_record,
						bin_file) == ERROR)
	{
	  WARNING ("Aborting on input error (time record)");
	  // close binary file
	  fclose (bin_file);

	  exit (1);
	}
      io_binary_print_time_record (&binary_time_record);

      if (binary_time_record.record_number > binary_records_max_count)
	{
	  WARNING ("Number of records exceeds maximum (%d)",
		   binary_records_max_count);
	  // close binary file
	  fclose (bin_file);

	  exit (1);
	}

      // read binary records
      if (io_read_binary_records_from_file (binary_records,
					    binary_time_record.record_number,
					    bin_file) == ERROR)
	{
	  WARNING ("Aborting on input error (records)");
	  // close binary file
	  fclose (bin_file);

	  exit (1);
	}

      // print binary records
      for (rec_i = 0; rec_i < binary_time_record.record_number; rec_i++)
	io_binary_print_record (&binary_records[rec_i]);
    }

  // free memory for binary records
  free (binary_records);

  // close binary file
  fclose (bin_file);

  exit (0);
}

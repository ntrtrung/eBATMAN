#ifndef __IO_H
#define __IO_H


//////////////////////////////////
// Constants
//////////////////////////////////

#define DEFAULT_NS2_SPEED       1e6
#define MAX_CONNECTIONS 50
#define TRUE                            1
#define FALSE                           0

#define SUCCESS                         0
#define ERROR                           -1

#ifndef MAX_STRING
#define MAX_STRING                      256
#endif

//////////////////////////////////
// Binary I/O file structures
//////////////////////////////////

// binary file header
typedef struct
{
  char signature[4];
  int major_version;
  int minor_version;
  int subminor_version;
  int svn_revision;
  //char reserved[4];
  int node_number;
  long int time_record_number;

} binary_header_class;

// binary file time record
typedef struct
{
  float time;
  int record_number;
} binary_time_record_class;

// binary file record holding most important fields
// NOTE: update 'io_binary_print_record', 'io_binary_build_record', 
// 'io_copy_record' and 'io_binary_compare_record' when making changes
typedef struct
{
  int from_node;
  int to_node;
  float frame_error_rate;
  float num_retransmissions;
  float operating_rate;
  float bandwidth;
  float loss_rate;
  float delay;
  //float jitter; // not needed yet
} binary_record_class;

typedef struct
{
  binary_time_record_class binary_time_record;
  binary_record_class binary_records[MAX_CONNECTIONS];
  int state_changed[MAX_CONNECTIONS];
} io_connection_state_class;


////////////////////////////////////////////////
// Binary I/O functions
////////////////////////////////////////////////

// print binary header
void io_binary_print_header (binary_header_class * binary_header);

// print binary time record
void io_binary_print_time_record (binary_time_record_class *
				  binary_time_record);

// print binary record
void io_binary_print_record (binary_record_class * binary_record);

// copy binary record
void io_binary_copy_record (binary_record_class * binary_record_dst,
			    binary_record_class * binary_record_src);


// read header of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_read_binary_header_from_file (binary_header_class * binary_header,
				     FILE * binary_input_file);

// write header of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_write_binary_header_to_file (int node_number,
				    long int time_record_number,
				    int major_version,
				    int minor_version,
				    int subminor_version,
				    int svn_revision, FILE * binary_file);

// read a time record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_read_binary_time_record_from_file (binary_time_record_class *
					  binary_time_record,
					  FILE * binary_input_file);

// write a time record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_write_binary_time_record_to_file (float time, int record_number,
					 FILE * binary_file);

// directly write a time record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_write_binary_time_record_to_file2 (binary_time_record_class *
					  binary_time_record,
					  FILE * binary_file);

// read a record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_read_binary_record_from_file (binary_record_class * binary_record,
				     FILE * binary_input_file);

// read 'number_records' records from a QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_read_binary_records_from_file (binary_record_class * binary_records,
				      int number_records,
				      FILE * binary_input_file);

// directly write a record of QOMET binary output file;
// return SUCCESS on succes, ERROR on error
int io_write_binary_record_to_file2 (binary_record_class * binary_record,
				     FILE * binary_file);

#endif

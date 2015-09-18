
###########################################
# Makefile for qomet library timer
###########################################

#  $Revision: 128 $
#  $LastChangedDate: 2009-02-06 10:21:50 +0900 (Fri, 06 Feb 2009) $
#  $LastChangedBy: razvan $


.SUFFIXES:	.c .o
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
.o:
	$(CC) -o $@ $< $(LDFLAGS) $(LDLIBS)

CFLAGS	= -I. -g -Wall -Werror -DMESSAGE_WARNING -DMESSAGE_INFO
#CFLAGS	= -I. -g -Wall -Werror -DMESSAGE_WARNING -DMESSAGE_DEBUG -DMESSAGE_INFO

TARGETS	= libtimer.a

all:	  $(TARGETS)

libtimer.a: timer.o
	ar rc libtimer.a timer.o && ranlib libtimer.a	

timer.o: timer.c timer.h

clean:
	rm -f $(TARGETS) *.o *.a core


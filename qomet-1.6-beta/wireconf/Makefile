
###########################################
# Makefile for qomet library wireconf
###########################################

#  $Revision: 128 $
#  $LastChangedDate: 2009-02-06 10:21:50 +0900 (Fri, 06 Feb 2009) $
#  $LastChangedBy: razvan $

# expat library defines
# NOTE: EXPAT_PATH is only needed for non-standard installations
EXPAT_PATH=/usr/local
EXPAT_INC=-I${EXPAT_PATH}/include
# uncomment STATIC_EXPAT so as to cause static linking and enable running 
# on other PCs than the one on which do_wireconf was compiled; otherwise
# uncomment EXPAT_LIB for dynamic linking
STATIC_EXPAT=/usr/local/lib/libexpat.a
#EXPAT_LIB=-L${EXPAT_PATH}/lib -lexpat

# timer library defines
TIMER_PATH=../timer
TIMER_INC=-I${TIMER_PATH}
TIMER_LIB=-L${TIMER_PATH} -ltimer

# deltaQ library defines
DELTAQ_PATH=../deltaQ
DELTAQ_INC=-I${DELTAQ_PATH}
DELTAQ_LIB=-L${DELTAQ_PATH} -ldeltaQ

.SUFFIXES:	.c .o
.c.o:
	$(CC) $(CFLAGS) ${INCS} -c -o $@ $<
#.o:
#	$(CC) -o $@ $< $(LDFLAGS) $(LDLIBS)

CFLAGS	= -I. -g -Wall -Werror -DMESSAGE_WARNING -DMESSAGE_INFO
#CFLAGS	= -I. -g -Wall -Werror -DMESSAGE_WARNING -DMESSAGE_DEBUG -DMESSAGE_INFO
LDFLAGS	= -L.
LDLIBS	= -lwireconf -lm -lpthread ${TIMER_LIB} ${DELTAQ_LIB} ${EXPAT_LIB}
INCS=${EXPAT_INC} ${TIMER_INC} ${DELTAQ_INC}

TARGETS	= libwireconf.a do_wireconf

all:	  $(TARGETS)

libwireconf.a: wireconf.o statistics.o routing_info.o
	ar rc libwireconf.a wireconf.o statistics.o routing_info.o \
	&& ranlib libwireconf.a

wireconf.o: wireconf.c wireconf.h global.h message.h

statistics.o: statistics.c statistics.h global.h message.h wireconf.h

routing_info.o: routing_info.c routing_info.h message.h

do_wireconf_ipc.o: do_wireconf_ipc.c do_wireconf_ipc.h message.h

#do_wireconf: do_wireconf.c do_wireconf_ipc.o libwireconf.a ${TIMER_PATH}/libtimer.a
#	$(CC) $(CFLAGS) do_wireconf_ipc.o -o $@ do_wireconf.c $(LDFLAGS) $(LDLIBS)

do_wireconf.o: do_wireconf.c global.h message.h 

do_wireconf: do_wireconf.o do_wireconf_ipc.o \
	libwireconf.a ${TIMER_PATH}/libtimer.a ${DELTAQ_PATH}/libdeltaQ.a
	$(CC) $(CFLAGS) do_wireconf_ipc.o do_wireconf.o \
	-o $@ $(LDFLAGS) $(LDLIBS) ${INCS} $(STATIC_EXPAT)

clean:
	rm -f $(TARGETS) *.o *.a core


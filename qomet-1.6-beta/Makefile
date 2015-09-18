
##############################################
# Makefile for qomet: network emulation tools
##############################################

#  $Revision: 154 $
#  $LastChangedDate: 2009-05-27 16:24:39 +0900 (Wed, 27 May 2009) $
#  $LastChangedBy: razvan $


#COMPILE_TYPE=debug
#COMPILE_TYPE=profile
COMPILE_TYPE=release


# only need to define EXPAT_PATH for non-standard installations of expat
EXPAT_PATH=/usr/local
EXPAT_INC=-I${EXPAT_PATH}/include
EXPAT_LIB=-L${EXPAT_PATH}/lib -lexpat

# paths for various QOMET components
TIMER_PATH=./timer
CHANEL_PATH=./chanel
WIRECONF_PATH=./wireconf
DELTAQ_PATH=./deltaQ
DELTAQ_INC=-I${DELTAQ_PATH}
DELTAQ_LIB=-L${DELTAQ_PATH} -ldeltaQ
ROUTING_PATH=./routing

# different options and flags
INCS=${EXPAT_INC} ${DELTAQ_INC}
LIBS=-lm ${EXPAT_LIB} ${DELTAQ_LIB}
GENERAL_FLAGS=$(SVN_DEFINE) -Wall -Werror

# compiler flags
ifeq ($(COMPILE_TYPE), debug)
GCC_FLAGS = $(GENERAL_FLAGS) -g    # generate debugging info
endif

ifeq ($(COMPILE_TYPE), profile)
GCC_FLAGS = $(GENERAL_FLAGS) -pg   # generate profiling info
endif

ifeq ($(COMPILE_TYPE), release)
GCC_FLAGS = $(GENERAL_FLAGS)       # no additional info
endif

# get the name of the operating system
OS_NAME=$(shell uname)

ANY_OS_TARGETS = qomet generate_scenario show_bin ${DELTAQ_PATH}/libdeltaQ.a ${CHANEL_PATH}/do_chanel
FREEBSD_TARGETS = ${TIMER_PATH}/libtimer.a ${WIRECONF_PATH}/libwireconf.a ${ROUTING_PATH}/do_routing

# compile wireconf only of FreeBSD systems
# decide what make command will be used for submodules
ifeq ($(OS_NAME),FreeBSD)
all : ${ANY_OS_TARGETS} ${FREEBSD_TARGETS}
MAKE_CMD = gmake
else
all : ${ANY_OS_TARGETS}
MAKE_CMD = make
endif

# defining revision info
SVN_REVISION := "$(shell svnversion -n .)"

# if revision info cannot be obtained from SVN, try to get it from a file
ifeq ($(SVN_REVISION), "")
#$(warning No SVN revision info, trying to get it from 'svn_revision.txt')
SVN_REVISION := "$(shell cat svn_revision.txt)"
else
#$(warning Writing revision info to file 'svn_revision.txt')
$(shell echo "$(SVN_REVISION)" | cat > svn_revision.txt)
endif
SVN_DEFINE = -D'SVN_REVISION=$(SVN_REVISION)'


# created object qomet.o so that module dependencies
# can be visualized using "nmdepend"
qomet : qomet.o ${DELTAQ_PATH}/libdeltaQ.a
	gcc $(GCC_FLAGS) qomet.o -o qomet ${INCS} ${LIBS}

qomet.o : qomet.c
	gcc $(GCC_FLAGS) qomet.c -c ${INCS}

generate_scenario : generate_scenario.c ${DELTAQ_PATH}/libdeltaQ.a
	gcc $(GCC_FLAGS) generate_scenario.c -o generate_scenario ${INCS} ${LIBS}

show_bin : show_bin.c ${DELTAQ_PATH}/libdeltaQ.a
	gcc $(GCC_FLAGS) show_bin.c -o show_bin ${INCS} ${LIBS}

bin2out.o : 
	gcc $(GCC_FLAGS) bin2out.c -c $(INCS)


${DELTAQ_PATH}/libdeltaQ.a :  ${DELTAQ_PATH}/*.c ${DELTAQ_PATH}/*.h
	cd ${DELTAQ_PATH}; ${MAKE_CMD} COMPILE_TYPE=$(COMPILE_TYPE)

${TIMER_PATH}/libtimer.a :  ${TIMER_PATH}/*.c ${TIMER_PATH}/*.h
	cd ${TIMER_PATH}; ${MAKE_CMD}

${CHANEL_PATH}/do_chanel : ${CHANEL_PATH}/*.c ${CHANEL_PATH}/*.h
	cd ${CHANEL_PATH}; ${MAKE_CMD} COMPILE_TYPE=$(COMPILE_TYPE)

${WIRECONF_PATH}/libwireconf.a : ${WIRECONF_PATH}/*.c ${WIRECONF_PATH}/*.h
	cd ${WIRECONF_PATH}; ${MAKE_CMD}

${ROUTING_PATH}/do_routing : ${ROUTING_PATH}/*.c ${ROUTING_PATH}/*.h
	cd ${ROUTING_PATH}; ${MAKE_CMD}

clean:
	rm -f ${ANY_OS_TARGETS} ${FREEBSD_TARGETS} *.o core; (cd ${CHANEL_PATH} && ${MAKE_CMD} clean); (cd ${DELTAQ_PATH} && ${MAKE_CMD} clean); (cd ${WIRECONF_PATH} && ${MAKE_CMD} clean); (cd ${ROUTING_PATH} && ${MAKE_CMD} clean); (cd ${TIMER_PATH} && ${MAKE_CMD} clean)

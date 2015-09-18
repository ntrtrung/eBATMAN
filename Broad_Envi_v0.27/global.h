#ifndef _GLOBAL_H_
#define _GLOBAL_H_




#define BUFFER_SIZE 65536
unsigned char buf[BUFFER_SIZE];

/* tcpdump header (ether.h) defines ETHER_HDRLEN) */
#ifndef ETHER_HDRLEN 
#define ETHER_HDRLEN 14
#endif

#define TICKET_RARP -3
#define TICKET_BROADCAST -2
#define TICKET_NORMAL -1

//target ip address in arp header 



#endif

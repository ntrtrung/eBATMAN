#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <netinet/in.h>

#define BUFLEN 512
#define NPACK 10
#define MAX_PACKET_SIZE 1500

#define PPROMISC 0x8943
#define PROMISC 0x8902

#define TUNSETNOCSUM (('T'<< 8) | 200)

struct my_ip {
	u_int8_t	ip_vhl;		/* header length, version */
#define IP_V(ip)	(((ip)->ip_vhl & 0xf0) >> 4)
#define IP_HL(ip)	((ip)->ip_vhl & 0x0f)
	u_int8_t	ip_tos;		/* type of service */
	u_int16_t	ip_len;		/* total length */
	u_int16_t	ip_id;		/* identification */
	u_int16_t	ip_off;		/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	u_int8_t	ip_ttl;		/* time to live */
	u_int8_t	ip_p;		/* protocol */
	u_int16_t	ip_sum;		/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
};


int init_udp_server(char *ip, int server_port);
int init_udp_client();
int send_packet_to(int socket,char *data,int data_size,long server_ip,int server_port);
int receive_packet_from(int sd,char *packet,struct in_addr *server);
unsigned long get_ip_in_arphdr(u_char *packet);
int InitTCPServer(int port);
int AcceptTCPConnection(int listen_sd);
int init_tun(char* tap_name,char *ip, char *netmask);
int get_ip_of_interface(char *dev);
int get_broadcast_ip_of_interface(char *dev);
int is_ip_packet(char *packet);
unsigned long get_ip_dest(char *packet);
u_char *get_mac_dest(char *packet);
int is_mac_equal(char *mac1, char *mac2);
char *get_mac_of(char *device);
int open_device_for_check_promisc(char *dev);
int is_promisc(int fd);

#endif



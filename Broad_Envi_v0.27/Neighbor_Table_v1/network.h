#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <netinet/in.h>

#define BUFLEN 512
#define NPACK 10
#define MAX_PACKET_SIZE 1500

#define TUNSETNOCSUM (('T'<< 8) | 200)


int init_udp_server(char *ip, int server_port);
int init_udp_client();
int send_packet_to(int socket,char *data,int data_size,long server_ip,int server_port);
int receive_packet_from(int sd,char *packet,struct in_addr *server);
int InitTCPServer(int port);
int AcceptTCPConnection(int listen_sd);
int init_tun(char* tap_name,char *ip, char *netmask);
int get_ip_of_interface(char *dev);
unsigned char *get_mac_of(char *device);

#endif



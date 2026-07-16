#ifndef NET_H
#define NET_H

typedef struct
{
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
} IPV4_ADDR;

typedef struct
{
    char name[12];
    unsigned int link_up;
    unsigned int hardware;
    unsigned char mac[6];
    IPV4_ADDR ip;
    IPV4_ADDR gateway;
    IPV4_ADDR dns;
    unsigned int tx_packets;
    unsigned int rx_packets;
    unsigned int dropped_packets;
} NET_DEVICE;

typedef struct
{
    IPV4_ADDR ip;
    char mac[18];
    unsigned int used;
} ARP_ENTRY;

typedef struct
{
    unsigned short port;
    char protocol[5];
    char state[9];
    unsigned int used;
} NET_SOCKET;

void net_init();
NET_DEVICE* net_get_device();
void net_set_link(unsigned int up);
void net_set_ip(IPV4_ADDR ip);
void net_set_gateway(IPV4_ADDR gateway);
ARP_ENTRY* net_get_arp(unsigned int index);
unsigned int net_arp_count();
int net_arp_add(IPV4_ADDR ip,char* mac);
int net_socket_open(unsigned short port,char* protocol);
int net_socket_close(unsigned short port);
NET_SOCKET* net_get_socket(unsigned int index);
unsigned int net_socket_count();
int net_dns_resolve(char* name,IPV4_ADDR* out);
void net_dhcp_refresh();
void net_print_ip(IPV4_ADDR ip);
int net_parse_ipv4(char* text,IPV4_ADDR* out);
int net_ping(IPV4_ADDR target);
int net_send(IPV4_ADDR target,char* payload);
int net_send_raw_demo(char* payload);
int net_send_arp_request(IPV4_ADDR target);
int net_send_udp(IPV4_ADDR target,unsigned short port,char* payload);
void net_receive_demo(char* payload);

#endif

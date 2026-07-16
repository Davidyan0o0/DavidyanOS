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

void net_init();
NET_DEVICE* net_get_device();
void net_set_link(unsigned int up);
void net_set_ip(IPV4_ADDR ip);
void net_set_gateway(IPV4_ADDR gateway);
ARP_ENTRY* net_get_arp(unsigned int index);
unsigned int net_arp_count();
int net_arp_add(IPV4_ADDR ip,char* mac);
void net_print_ip(IPV4_ADDR ip);
int net_parse_ipv4(char* text,IPV4_ADDR* out);
int net_ping(IPV4_ADDR target);
int net_send(IPV4_ADDR target,char* payload);
void net_receive_demo(char* payload);

#endif

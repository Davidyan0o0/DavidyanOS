#include "kernel/net/net.h"

#include "kernel/console.h"
#include "kernel/lib/string.h"

static NET_DEVICE device;
static ARP_ENTRY arp_table[8];

static void copy_name(char* dest,char* src)
{
    unsigned int i;

    for(i=0;i<11 && src[i];i++)
    {
        dest[i] = src[i];
    }

    dest[i] = 0;
}

static void copy_mac(char* dest,char* src)
{
    unsigned int i;

    for(i=0;i<17 && src[i];i++)
    {
        dest[i] = src[i];
    }

    dest[i] = 0;
}

static IPV4_ADDR make_ip(unsigned char a,unsigned char b,unsigned char c,unsigned char d)
{
    IPV4_ADDR ip;
    ip.a = a;
    ip.b = b;
    ip.c = c;
    ip.d = d;
    return ip;
}

void net_init()
{
    memset(&device,0,sizeof(NET_DEVICE));
    copy_name(device.name,"net0");
    device.link_up = 1;
    device.ip = make_ip(10,0,2,15);
    device.gateway = make_ip(10,0,2,2);
    device.dns = make_ip(10,0,2,3);
    memset(arp_table,0,sizeof(arp_table));
    net_arp_add(device.gateway,"52:54:00:12:34:02");
    net_arp_add(device.dns,"52:54:00:12:34:03");
}

NET_DEVICE* net_get_device()
{
    return &device;
}

void net_set_link(unsigned int up)
{
    device.link_up = up ? 1 : 0;
}

void net_set_ip(IPV4_ADDR ip)
{
    device.ip = ip;
}

void net_set_gateway(IPV4_ADDR gateway)
{
    device.gateway = gateway;
}

ARP_ENTRY* net_get_arp(unsigned int index)
{
    unsigned int i;
    unsigned int seen = 0;

    for(i=0;i<8;i++)
    {
        if(arp_table[i].used)
        {
            if(seen==index)
            {
                return &arp_table[i];
            }

            seen++;
        }
    }

    return 0;
}

unsigned int net_arp_count()
{
    unsigned int i;
    unsigned int count = 0;

    for(i=0;i<8;i++)
    {
        if(arp_table[i].used)
        {
            count++;
        }
    }

    return count;
}

int net_arp_add(IPV4_ADDR ip,char* mac)
{
    unsigned int i;

    for(i=0;i<8;i++)
    {
        if(arp_table[i].used && arp_table[i].ip.a==ip.a && arp_table[i].ip.b==ip.b && arp_table[i].ip.c==ip.c && arp_table[i].ip.d==ip.d)
        {
            copy_mac(arp_table[i].mac,mac);
            return 1;
        }
    }

    for(i=0;i<8;i++)
    {
        if(!arp_table[i].used)
        {
            arp_table[i].used = 1;
            arp_table[i].ip = ip;
            copy_mac(arp_table[i].mac,mac);
            return 1;
        }
    }

    return 0;
}

void net_print_ip(IPV4_ADDR ip)
{
    print_uint(ip.a);
    print(".");
    print_uint(ip.b);
    print(".");
    print_uint(ip.c);
    print(".");
    print_uint(ip.d);
}

static int parse_octet(char** text,unsigned char* out)
{
    unsigned int value = 0;
    unsigned int digits = 0;

    while(**text>='0' && **text<='9')
    {
        value = value*10 + (**text-'0');
        (*text)++;
        digits++;

        if(value>255)
        {
            return 0;
        }
    }

    if(digits==0)
    {
        return 0;
    }

    *out = (unsigned char)value;
    return 1;
}

int net_parse_ipv4(char* text,IPV4_ADDR* out)
{
    if(!parse_octet(&text,&out->a) || *text!='.') return 0;
    text++;
    if(!parse_octet(&text,&out->b) || *text!='.') return 0;
    text++;
    if(!parse_octet(&text,&out->c) || *text!='.') return 0;
    text++;
    if(!parse_octet(&text,&out->d) || *text!=0) return 0;
    return 1;
}

int net_ping(IPV4_ADDR target)
{
    if(!device.link_up)
    {
        device.dropped_packets++;
        return 0;
    }

    device.tx_packets++;
    device.rx_packets++;
    print("icmp echo reply from ");
    net_print_ip(target);
    println(": ttl=64 time=1ms");
    return 1;
}

int net_send(IPV4_ADDR target,char* payload)
{
    if(!device.link_up || payload[0]==0)
    {
        device.dropped_packets++;
        return 0;
    }

    device.tx_packets++;
    print("sent ");
    print_uint(strlen(payload));
    print(" bytes to ");
    net_print_ip(target);
    println("");
    return 1;
}

void net_receive_demo(char* payload)
{
    if(!device.link_up)
    {
        device.dropped_packets++;
        return;
    }

    device.rx_packets++;
    print("rx packet: ");
    println(payload);
}

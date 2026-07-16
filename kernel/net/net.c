#include "kernel/net/net.h"

#include "kernel/console.h"
#include "kernel/lib/string.h"
#include "drivers/net/ne2k.h"

static NET_DEVICE device;
static ARP_ENTRY arp_table[8];
static NET_SOCKET sockets[8];

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

static void copy_socket_text(char* dest,char* src,unsigned int max)
{
    unsigned int i;

    for(i=0;i<max-1 && src[i];i++)
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

static void put16(unsigned char* buffer,unsigned int offset,unsigned short value)
{
    buffer[offset] = (unsigned char)(value>>8);
    buffer[offset+1] = (unsigned char)(value&0xFF);
}

static unsigned short ip_checksum(unsigned char* data,unsigned int length)
{
    unsigned int sum = 0;
    unsigned int i;

    for(i=0;i<length;i+=2)
    {
        sum += ((unsigned int)data[i]<<8) + data[i+1];
        while(sum>>16)
        {
            sum = (sum&0xFFFF) + (sum>>16);
        }
    }

    return (unsigned short)(~sum);
}

static void write_mac(unsigned char* dest,unsigned char* src)
{
    unsigned int i;

    for(i=0;i<6;i++)
    {
        dest[i] = src[i];
    }
}

static void write_ip(unsigned char* dest,IPV4_ADDR ip)
{
    dest[0] = ip.a;
    dest[1] = ip.b;
    dest[2] = ip.c;
    dest[3] = ip.d;
}

void net_init()
{
    unsigned int i;

    memset(&device,0,sizeof(NET_DEVICE));
    copy_name(device.name,"net0");
    device.link_up = 1;
    device.ip = make_ip(10,0,2,15);
    device.gateway = make_ip(10,0,2,2);
    device.dns = make_ip(10,0,2,3);
    memset(arp_table,0,sizeof(arp_table));
    memset(sockets,0,sizeof(sockets));
    ne2k_init();
    device.hardware = ne2k_present();
    if(device.hardware)
    {
        for(i=0;i<6;i++)
        {
            device.mac[i] = ne2k_mac()[i];
        }
    }
    net_arp_add(device.gateway,"52:54:00:12:34:02");
    net_arp_add(device.dns,"52:54:00:12:34:03");
}

int net_socket_open(unsigned short port,char* protocol)
{
    unsigned int i;

    for(i=0;i<8;i++)
    {
        if(sockets[i].used && sockets[i].port==port)
        {
            return 0;
        }
    }

    for(i=0;i<8;i++)
    {
        if(!sockets[i].used)
        {
            sockets[i].used = 1;
            sockets[i].port = port;
            copy_socket_text(sockets[i].protocol,protocol,5);
            copy_socket_text(sockets[i].state,"LISTEN",9);
            return 1;
        }
    }

    return 0;
}

int net_socket_close(unsigned short port)
{
    unsigned int i;

    for(i=0;i<8;i++)
    {
        if(sockets[i].used && sockets[i].port==port)
        {
            memset(&sockets[i],0,sizeof(NET_SOCKET));
            return 1;
        }
    }

    return 0;
}

NET_SOCKET* net_get_socket(unsigned int index)
{
    unsigned int i;
    unsigned int seen = 0;

    for(i=0;i<8;i++)
    {
        if(sockets[i].used)
        {
            if(seen==index)
            {
                return &sockets[i];
            }

            seen++;
        }
    }

    return 0;
}

unsigned int net_socket_count()
{
    unsigned int i;
    unsigned int count = 0;

    for(i=0;i<8;i++)
    {
        if(sockets[i].used)
        {
            count++;
        }
    }

    return count;
}

int net_dns_resolve(char* name,IPV4_ADDR* out)
{
    if(strcmp(name,"localhost")==0 || strcmp(name,"os.local")==0)
    {
        *out = make_ip(127,0,0,1);
        return 1;
    }

    if(strcmp(name,"gateway.local")==0)
    {
        *out = device.gateway;
        return 1;
    }

    return 0;
}

void net_dhcp_refresh()
{
    if(device.link_up)
    {
        device.ip = make_ip(10,0,2,15);
        device.gateway = make_ip(10,0,2,2);
        device.dns = make_ip(10,0,2,3);
    }
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

int net_send_raw_demo(char* payload)
{
    unsigned char frame[64];
    unsigned int i;
    unsigned int payload_length = strlen(payload);

    if(!device.link_up || !device.hardware || payload[0]==0)
    {
        device.dropped_packets++;
        return 0;
    }

    memset(frame,0,sizeof(frame));
    for(i=0;i<6;i++)
    {
        frame[i] = 0xFF;
        frame[6+i] = device.mac[i];
    }
    frame[12] = 0x88;
    frame[13] = 0xB5;
    if(payload_length>46)
    {
        payload_length = 46;
    }
    memcpy(frame+14,payload,payload_length);

    if(!ne2k_send(frame,60))
    {
        device.dropped_packets++;
        return 0;
    }

    device.tx_packets++;
    return 1;
}

int net_send_arp_request(IPV4_ADDR target)
{
    unsigned char frame[60];
    unsigned int i;

    if(!device.link_up || !device.hardware)
    {
        device.dropped_packets++;
        return 0;
    }

    memset(frame,0,sizeof(frame));
    for(i=0;i<6;i++)
    {
        frame[i] = 0xFF;
    }
    write_mac(frame+6,device.mac);
    put16(frame,12,0x0806);
    put16(frame,14,0x0001);
    put16(frame,16,0x0800);
    frame[18] = 6;
    frame[19] = 4;
    put16(frame,20,1);
    write_mac(frame+22,device.mac);
    write_ip(frame+28,device.ip);
    write_ip(frame+38,target);

    if(!ne2k_send(frame,60))
    {
        device.dropped_packets++;
        return 0;
    }
    device.tx_packets++;
    return 1;
}

int net_send_udp(IPV4_ADDR target,unsigned short port,char* payload)
{
    unsigned char frame[128];
    unsigned int payload_length = strlen(payload);
    unsigned int total_length;
    unsigned short checksum;
    unsigned int i;

    if(!device.link_up || !device.hardware || payload[0]==0)
    {
        device.dropped_packets++;
        return 0;
    }
    if(payload_length>80)
    {
        payload_length = 80;
    }

    memset(frame,0,sizeof(frame));
    for(i=0;i<6;i++)
    {
        frame[i] = 0xFF;
    }
    write_mac(frame+6,device.mac);
    put16(frame,12,0x0800);

    total_length = 20 + 8 + payload_length;
    frame[14] = 0x45;
    frame[15] = 0x00;
    put16(frame,16,(unsigned short)total_length);
    put16(frame,18,0x1234);
    put16(frame,20,0x0000);
    frame[22] = 64;
    frame[23] = 17;
    write_ip(frame+26,device.ip);
    write_ip(frame+30,target);
    checksum = ip_checksum(frame+14,20);
    put16(frame,24,checksum);

    put16(frame,34,40000);
    put16(frame,36,port);
    put16(frame,38,(unsigned short)(8+payload_length));
    put16(frame,40,0);
    memcpy(frame+42,payload,payload_length);

    if(!ne2k_send(frame,14+total_length))
    {
        device.dropped_packets++;
        return 0;
    }
    device.tx_packets++;
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

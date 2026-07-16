#include "drivers/net/ne2k.h"

#include "kernel/arch/x86/io.h"
#include "kernel/lib/string.h"

#define NE2K_BASE 0x300
#define NE2K_CMD 0x00
#define NE2K_PSTART 0x01
#define NE2K_PSTOP 0x02
#define NE2K_BNRY 0x03
#define NE2K_TPSR 0x04
#define NE2K_TBCR0 0x05
#define NE2K_TBCR1 0x06
#define NE2K_ISR 0x07
#define NE2K_RSAR0 0x08
#define NE2K_RSAR1 0x09
#define NE2K_RBCR0 0x0A
#define NE2K_RBCR1 0x0B
#define NE2K_RCR 0x0C
#define NE2K_TCR 0x0D
#define NE2K_DCR 0x0E
#define NE2K_IMR 0x0F
#define NE2K_DATA 0x10

#define NE2K_CURR 0x07
#define NE2K_MAR0 0x08

#define NE2K_PAGE_TX 0x40
#define NE2K_PAGE_RX_START 0x46
#define NE2K_PAGE_RX_STOP 0x80

static unsigned int present = 0;
static unsigned int tx_packets = 0;
static unsigned int rx_packets = 0;
static unsigned int last_status = 0;
static unsigned char mac_addr[6];

static void ne2k_page0()
{
    outb(NE2K_BASE+NE2K_CMD,0x22);
}

static void ne2k_page1()
{
    outb(NE2K_BASE+NE2K_CMD,0x62);
}

static void ne2k_remote_read(unsigned short address,unsigned short length,unsigned char* out)
{
    unsigned int i;
    unsigned short word;

    ne2k_page0();
    outb(NE2K_BASE+NE2K_ISR,0x40);
    outb(NE2K_BASE+NE2K_RBCR0,(unsigned char)(length&0xFF));
    outb(NE2K_BASE+NE2K_RBCR1,(unsigned char)(length>>8));
    outb(NE2K_BASE+NE2K_RSAR0,(unsigned char)(address&0xFF));
    outb(NE2K_BASE+NE2K_RSAR1,(unsigned char)(address>>8));
    outb(NE2K_BASE+NE2K_CMD,0x0A);

    for(i=0;i<length;i+=2)
    {
        word = inw(NE2K_BASE+NE2K_DATA);
        out[i] = (unsigned char)(word&0xFF);
        if(i+1<length)
        {
            out[i+1] = (unsigned char)(word>>8);
        }
    }
}

static void ne2k_remote_write(unsigned short address,unsigned short length,unsigned char* data)
{
    unsigned int i;
    unsigned short word;

    ne2k_page0();
    outb(NE2K_BASE+NE2K_ISR,0x40);
    outb(NE2K_BASE+NE2K_RBCR0,(unsigned char)(length&0xFF));
    outb(NE2K_BASE+NE2K_RBCR1,(unsigned char)(length>>8));
    outb(NE2K_BASE+NE2K_RSAR0,(unsigned char)(address&0xFF));
    outb(NE2K_BASE+NE2K_RSAR1,(unsigned char)(address>>8));
    outb(NE2K_BASE+NE2K_CMD,0x12);

    for(i=0;i<length;i+=2)
    {
        word = data[i];
        if(i+1<length)
        {
            word |= ((unsigned short)data[i+1])<<8;
        }
        outw(NE2K_BASE+NE2K_DATA,word);
    }
}

void ne2k_init()
{
    unsigned char prom[32];
    unsigned int i;

    present = 0;
    tx_packets = 0;
    rx_packets = 0;
    memset(mac_addr,0,sizeof(mac_addr));

    outb(NE2K_BASE+NE2K_CMD,0x21);
    outb(NE2K_BASE+NE2K_DCR,0x49);
    outb(NE2K_BASE+NE2K_RBCR0,0);
    outb(NE2K_BASE+NE2K_RBCR1,0);
    outb(NE2K_BASE+NE2K_RCR,0x20);
    outb(NE2K_BASE+NE2K_TCR,0x02);
    outb(NE2K_BASE+NE2K_PSTART,NE2K_PAGE_RX_START);
    outb(NE2K_BASE+NE2K_PSTOP,NE2K_PAGE_RX_STOP);
    outb(NE2K_BASE+NE2K_BNRY,NE2K_PAGE_RX_START);
    outb(NE2K_BASE+NE2K_ISR,0xFF);
    outb(NE2K_BASE+NE2K_IMR,0x00);

    ne2k_remote_read(0,32,prom);
    for(i=0;i<6;i++)
    {
        mac_addr[i] = prom[i*2];
    }

    if(mac_addr[0]==0 && mac_addr[1]==0 && mac_addr[2]==0 && mac_addr[3]==0 && mac_addr[4]==0 && mac_addr[5]==0)
    {
        return;
    }

    ne2k_page1();
    for(i=0;i<6;i++)
    {
        outb(NE2K_BASE+i,mac_addr[i]);
    }
    for(i=0;i<8;i++)
    {
        outb(NE2K_BASE+NE2K_MAR0+i,0xFF);
    }
    outb(NE2K_BASE+NE2K_CURR,NE2K_PAGE_RX_START+1);

    ne2k_page0();
    outb(NE2K_BASE+NE2K_RCR,0x04);
    outb(NE2K_BASE+NE2K_TCR,0x00);
    outb(NE2K_BASE+NE2K_CMD,0x22);
    present = 1;
}

unsigned int ne2k_present()
{
    return present;
}

unsigned char* ne2k_mac()
{
    return mac_addr;
}

unsigned int ne2k_tx_count()
{
    return tx_packets;
}

unsigned int ne2k_rx_count()
{
    return rx_packets;
}

unsigned int ne2k_last_status()
{
    return last_status;
}

int ne2k_send(unsigned char* data,unsigned int length)
{
    if(!present || length==0 || length>NE2K_PACKET_MAX)
    {
        return 0;
    }
    if(length<60)
    {
        length = 60;
    }

    ne2k_remote_write((unsigned short)(NE2K_PAGE_TX*256),length,data);
    outb(NE2K_BASE+NE2K_TPSR,NE2K_PAGE_TX);
    outb(NE2K_BASE+NE2K_TBCR0,(unsigned char)(length&0xFF));
    outb(NE2K_BASE+NE2K_TBCR1,(unsigned char)(length>>8));
    outb(NE2K_BASE+NE2K_CMD,0x26);
    tx_packets++;
    last_status = inb(NE2K_BASE+NE2K_ISR);
    return 1;
}

int ne2k_poll(unsigned char* buffer,unsigned int* length)
{
    (void)buffer;
    (void)length;
    if(!present)
    {
        return 0;
    }
    last_status = inb(NE2K_BASE+NE2K_ISR);
    return 0;
}

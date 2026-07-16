#include "drivers/storage/ata.h"

#include "kernel/arch/x86/io.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6
#define ATA_REG_DATA 0
#define ATA_REG_ERROR 1
#define ATA_REG_SECCOUNT 2
#define ATA_REG_LBA0 3
#define ATA_REG_LBA1 4
#define ATA_REG_LBA2 5
#define ATA_REG_HDDEVSEL 6
#define ATA_REG_STATUS 7
#define ATA_REG_COMMAND 7

#define ATA_SR_ERR 0x01
#define ATA_SR_DRQ 0x08
#define ATA_SR_BSY 0x80

#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_IDENTIFY 0xEC

static unsigned int last_status = 0;

static void ata_delay()
{
    inb(ATA_PRIMARY_CTRL);
    inb(ATA_PRIMARY_CTRL);
    inb(ATA_PRIMARY_CTRL);
    inb(ATA_PRIMARY_CTRL);
}

static int ata_wait_not_busy()
{
    unsigned int i;

    for(i=0;i<100000;i++)
    {
        last_status = inb(ATA_PRIMARY_IO+ATA_REG_STATUS);
        if((last_status&ATA_SR_BSY)==0)
        {
            return 1;
        }
    }

    return 0;
}

static int ata_wait_drq()
{
    unsigned int i;

    for(i=0;i<100000;i++)
    {
        last_status = inb(ATA_PRIMARY_IO+ATA_REG_STATUS);
        if(last_status&ATA_SR_ERR)
        {
            return 0;
        }
        if((last_status&ATA_SR_BSY)==0 && (last_status&ATA_SR_DRQ))
        {
            return 1;
        }
    }

    return 0;
}

static void ata_select_lba(unsigned int lba)
{
    outb(ATA_PRIMARY_IO+ATA_REG_HDDEVSEL,(unsigned char)(0xE0|((lba>>24)&0x0F)));
    ata_delay();
    outb(ATA_PRIMARY_IO+ATA_REG_SECCOUNT,1);
    outb(ATA_PRIMARY_IO+ATA_REG_LBA0,(unsigned char)(lba&0xFF));
    outb(ATA_PRIMARY_IO+ATA_REG_LBA1,(unsigned char)((lba>>8)&0xFF));
    outb(ATA_PRIMARY_IO+ATA_REG_LBA2,(unsigned char)((lba>>16)&0xFF));
}

void ata_init()
{
    outb(ATA_PRIMARY_CTRL,0x02);
    ata_identify();
}

int ata_identify()
{
    unsigned short data[256];

    outb(ATA_PRIMARY_IO+ATA_REG_HDDEVSEL,0xA0);
    ata_delay();
    outb(ATA_PRIMARY_IO+ATA_REG_SECCOUNT,0);
    outb(ATA_PRIMARY_IO+ATA_REG_LBA0,0);
    outb(ATA_PRIMARY_IO+ATA_REG_LBA1,0);
    outb(ATA_PRIMARY_IO+ATA_REG_LBA2,0);
    outb(ATA_PRIMARY_IO+ATA_REG_COMMAND,ATA_CMD_IDENTIFY);

    last_status = inb(ATA_PRIMARY_IO+ATA_REG_STATUS);
    if(last_status==0)
    {
        return 0;
    }

    if(!ata_wait_drq())
    {
        return 0;
    }

    insw(ATA_PRIMARY_IO+ATA_REG_DATA,data,256);
    return 1;
}

int ata_read_sector(unsigned int lba,unsigned char* buffer)
{
    if(!ata_wait_not_busy())
    {
        return 0;
    }

    ata_select_lba(lba);
    outb(ATA_PRIMARY_IO+ATA_REG_COMMAND,ATA_CMD_READ);

    if(!ata_wait_drq())
    {
        return 0;
    }

    insw(ATA_PRIMARY_IO+ATA_REG_DATA,buffer,256);
    return 1;
}

int ata_write_sector(unsigned int lba,unsigned char* buffer)
{
    if(!ata_wait_not_busy())
    {
        return 0;
    }

    ata_select_lba(lba);
    outb(ATA_PRIMARY_IO+ATA_REG_COMMAND,ATA_CMD_WRITE);

    if(!ata_wait_drq())
    {
        return 0;
    }
    outsw(ATA_PRIMARY_IO+ATA_REG_DATA,buffer,256);
    outb(ATA_PRIMARY_IO+ATA_REG_COMMAND,0xE7);
    ata_wait_not_busy();
    return 1;
}

unsigned int ata_last_status()
{
    return last_status;
}

#include "drivers/bus/pci.h"

#include "kernel/arch/x86/io.h"
#include "kernel/console.h"
#include "kernel/lib/string.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define PCI_MAX_DEVICES 32

static PCI_DEVICE devices[PCI_MAX_DEVICES];
static unsigned int device_count = 0;

unsigned int pci_config_read32(unsigned char bus,unsigned char slot,unsigned char function,unsigned char offset)
{
    unsigned int address;

    address = 0x80000000;
    address |= ((unsigned int)bus)<<16;
    address |= ((unsigned int)slot)<<11;
    address |= ((unsigned int)function)<<8;
    address |= offset&0xFC;

    outl(PCI_CONFIG_ADDRESS,address);
    return inl(PCI_CONFIG_DATA);
}

void pci_config_write32(unsigned char bus,unsigned char slot,unsigned char function,unsigned char offset,unsigned int value)
{
    unsigned int address;

    address = 0x80000000;
    address |= ((unsigned int)bus)<<16;
    address |= ((unsigned int)slot)<<11;
    address |= ((unsigned int)function)<<8;
    address |= offset&0xFC;

    outl(PCI_CONFIG_ADDRESS,address);
    outl(PCI_CONFIG_DATA,value);
}

static void pci_record_device(unsigned char bus,unsigned char slot,unsigned char function)
{
    unsigned int id;
    unsigned int class_reg;
    unsigned int header_reg;
    unsigned int i;
    PCI_DEVICE* dev;

    if(device_count>=PCI_MAX_DEVICES)
    {
        return;
    }

    id = pci_config_read32(bus,slot,function,0x00);
    if((id&0xFFFF)==0xFFFF)
    {
        return;
    }

    dev = &devices[device_count];
    memset(dev,0,sizeof(PCI_DEVICE));
    dev->bus = bus;
    dev->slot = slot;
    dev->function = function;
    dev->vendor_id = (unsigned short)(id&0xFFFF);
    dev->device_id = (unsigned short)((id>>16)&0xFFFF);

    class_reg = pci_config_read32(bus,slot,function,0x08);
    dev->prog_if = (unsigned char)((class_reg>>8)&0xFF);
    dev->subclass = (unsigned char)((class_reg>>16)&0xFF);
    dev->class_code = (unsigned char)((class_reg>>24)&0xFF);

    header_reg = pci_config_read32(bus,slot,function,0x0C);
    dev->header_type = (unsigned char)((header_reg>>16)&0xFF);

    for(i=0;i<6;i++)
    {
        dev->bar[i] = pci_config_read32(bus,slot,function,(unsigned char)(0x10+(i*4)));
    }

    device_count++;
}

void pci_init()
{
    unsigned int bus;
    unsigned int slot;
    unsigned int function;
    unsigned int header;
    unsigned int functions;

    device_count = 0;
    memset(devices,0,sizeof(devices));

    for(bus=0;bus<256;bus++)
    {
        for(slot=0;slot<32;slot++)
        {
            if((pci_config_read32((unsigned char)bus,(unsigned char)slot,0,0)&0xFFFF)==0xFFFF)
            {
                continue;
            }

            header = pci_config_read32((unsigned char)bus,(unsigned char)slot,0,0x0C);
            functions = (((header>>16)&0x80)!=0) ? 8 : 1;
            for(function=0;function<functions;function++)
            {
                pci_record_device((unsigned char)bus,(unsigned char)slot,(unsigned char)function);
            }
        }
    }
}

unsigned int pci_device_count()
{
    return device_count;
}

PCI_DEVICE* pci_get_device(unsigned int index)
{
    if(index>=device_count)
    {
        return 0;
    }

    return &devices[index];
}

unsigned int pci_find_class(unsigned char class_code,unsigned char subclass,unsigned int start,PCI_DEVICE* out)
{
    unsigned int i;

    for(i=start;i<device_count;i++)
    {
        if(devices[i].class_code==class_code && devices[i].subclass==subclass)
        {
            if(out)
            {
                memcpy(out,&devices[i],sizeof(PCI_DEVICE));
            }
            return i+1;
        }
    }

    return 0;
}

void pci_print_devices()
{
    unsigned int i;
    PCI_DEVICE* dev;

    for(i=0;i<device_count;i++)
    {
        dev = &devices[i];
        print_uint(dev->bus);
        print(":");
        print_uint(dev->slot);
        print(".");
        print_uint(dev->function);
        print(" vendor=");
        print_hex(dev->vendor_id);
        print(" device=");
        print_hex(dev->device_id);
        print(" class=");
        print_hex(dev->class_code);
        print(" sub=");
        print_hex(dev->subclass);
        print(" if=");
        print_hex(dev->prog_if);
        println("");
    }
}

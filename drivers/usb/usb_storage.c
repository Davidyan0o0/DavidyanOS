#include "drivers/usb/usb_storage.h"

#include "drivers/bus/pci.h"
#include "kernel/console.h"
#include "kernel/lib/string.h"

static USB_STORAGE_STATUS status;

static unsigned int mmio_read32(unsigned int base,unsigned int offset)
{
    volatile unsigned int* address = (volatile unsigned int*)(base+offset);
    return *address;
}

static unsigned short mmio_read16(unsigned int base,unsigned int offset)
{
    volatile unsigned short* address = (volatile unsigned short*)(base+offset);
    return *address;
}

static unsigned char mmio_read8(unsigned int base,unsigned int offset)
{
    volatile unsigned char* address = (volatile unsigned char*)(base+offset);
    return *address;
}

static char* controller_name(unsigned char prog_if)
{
    if(prog_if==PCI_PROGIF_UHCI)
    {
        return "UHCI";
    }
    if(prog_if==PCI_PROGIF_OHCI)
    {
        return "OHCI";
    }
    if(prog_if==PCI_PROGIF_EHCI)
    {
        return "EHCI";
    }
    if(prog_if==PCI_PROGIF_XHCI)
    {
        return "XHCI";
    }
    return "USB";
}

static void xhci_probe(PCI_DEVICE* dev)
{
    unsigned int command;
    unsigned int hcsparams1;
    unsigned int hcsparams2;
    unsigned int hccparams1;
    unsigned int base;

    if(dev->bar[0]&1)
    {
        return;
    }

    command = pci_config_read32(dev->bus,dev->slot,dev->function,0x04);
    command |= 0x00000006;
    pci_config_write32(dev->bus,dev->slot,dev->function,0x04,command);

    base = dev->bar[0]&0xFFFFFFF0;
    status.xhci_mmio_base = base;
    status.xhci_caplength = mmio_read8(base,0x00);
    status.xhci_version = mmio_read16(base,0x02);
    hcsparams1 = mmio_read32(base,0x04);
    hcsparams2 = mmio_read32(base,0x08);
    hccparams1 = mmio_read32(base,0x10);
    status.xhci_db_offset = mmio_read32(base,0x14)&0xFFFFFFFC;
    status.xhci_runtime_offset = mmio_read32(base,0x18)&0xFFFFFFE0;

    status.xhci_slots = hcsparams1&0xFF;
    status.xhci_ports = (hcsparams1>>24)&0xFF;
    status.xhci_scratchpads = ((hcsparams2>>21)&0x1F) | (((hcsparams2>>27)&0x1F)<<5);
    (void)hccparams1;

    if(status.xhci_caplength>=0x20 && status.xhci_version!=0 && status.xhci_version!=0xFFFF)
    {
        status.xhci_accessible = 1;
    }
}

void usb_storage_init()
{
    PCI_DEVICE dev;
    unsigned int next = 0;

    memset(&status,0,sizeof(status));
    status.stage = USB_STORAGE_STAGE_NO_CONTROLLER;

    while((next = pci_find_class(PCI_CLASS_SERIAL_BUS,PCI_SUBCLASS_USB,next,&dev))!=0)
    {
        status.controller_count++;
        if(!status.controller_found)
        {
            status.controller_found = 1;
            status.controller_prog_if = dev.prog_if;
            status.controller_bar = dev.bar[0];
            status.stage = USB_STORAGE_STAGE_CONTROLLER_FOUND;
            if(dev.prog_if==PCI_PROGIF_XHCI)
            {
                xhci_probe(&dev);
            }
        }
    }

    if(status.controller_found)
    {
        status.stage = USB_STORAGE_STAGE_NEEDS_HOST_DRIVER;
    }
}

USB_STORAGE_STATUS usb_storage_get_status()
{
    return status;
}

int usb_storage_read_sector(unsigned int lba,unsigned char* buffer)
{
    (void)lba;
    (void)buffer;
    return 0;
}

int usb_storage_write_sector(unsigned int lba,unsigned char* buffer)
{
    (void)lba;
    (void)buffer;
    return 0;
}

void usb_storage_print_status()
{
    print("usb controllers=");
    print_uint(status.controller_count);
    print(" storage=");
    println(status.storage_ready ? "ready" : "not ready");

    if(!status.controller_found)
    {
        println("no PCI USB controller found");
        return;
    }

    print("first controller=");
    print(controller_name(status.controller_prog_if));
    print(" prog_if=");
    print_hex(status.controller_prog_if);
    print(" bar0=");
    print_hex(status.controller_bar);
    println("");
    if(status.controller_prog_if==PCI_PROGIF_XHCI)
    {
        print("xhci mmio=");
        print_hex(status.xhci_mmio_base);
        print(" caplen=");
        print_uint(status.xhci_caplength);
        print(" version=");
        print_hex(status.xhci_version);
        println(status.xhci_accessible ? " accessible" : " not-accessible");
        print("xhci slots=");
        print_uint(status.xhci_slots);
        print(" ports=");
        print_uint(status.xhci_ports);
        print(" scratchpads=");
        print_uint(status.xhci_scratchpads);
        println("");
        print("xhci dboff=");
        print_hex(status.xhci_db_offset);
        print(" rtsoff=");
        print_hex(status.xhci_runtime_offset);
        println("");
    }
    println("mass storage needs host controller transfers and SCSI BOT");
}

#ifndef PCI_H
#define PCI_H

#define PCI_CLASS_SERIAL_BUS 0x0C
#define PCI_SUBCLASS_USB 0x03

#define PCI_PROGIF_UHCI 0x00
#define PCI_PROGIF_OHCI 0x10
#define PCI_PROGIF_EHCI 0x20
#define PCI_PROGIF_XHCI 0x30

typedef struct
{
    unsigned char bus;
    unsigned char slot;
    unsigned char function;
    unsigned short vendor_id;
    unsigned short device_id;
    unsigned char class_code;
    unsigned char subclass;
    unsigned char prog_if;
    unsigned char header_type;
    unsigned int bar[6];
} PCI_DEVICE;

void pci_init();
unsigned int pci_device_count();
PCI_DEVICE* pci_get_device(unsigned int index);
unsigned int pci_find_class(unsigned char class_code,unsigned char subclass,unsigned int start,PCI_DEVICE* out);
unsigned int pci_config_read32(unsigned char bus,unsigned char slot,unsigned char function,unsigned char offset);
void pci_config_write32(unsigned char bus,unsigned char slot,unsigned char function,unsigned char offset,unsigned int value);
void pci_print_devices();

#endif

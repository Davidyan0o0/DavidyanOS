#ifndef USB_STORAGE_H
#define USB_STORAGE_H

#define USB_STORAGE_STAGE_NO_CONTROLLER 0
#define USB_STORAGE_STAGE_CONTROLLER_FOUND 1
#define USB_STORAGE_STAGE_NEEDS_HOST_DRIVER 2
#define USB_STORAGE_STAGE_READY 3

typedef struct
{
    unsigned int controller_found;
    unsigned int controller_count;
    unsigned int storage_ready;
    unsigned int stage;
    unsigned char controller_prog_if;
    unsigned int controller_bar;
    unsigned int xhci_mmio_base;
    unsigned int xhci_caplength;
    unsigned int xhci_version;
    unsigned int xhci_slots;
    unsigned int xhci_ports;
    unsigned int xhci_scratchpads;
    unsigned int xhci_db_offset;
    unsigned int xhci_runtime_offset;
    unsigned int xhci_accessible;
} USB_STORAGE_STATUS;

void usb_storage_init();
USB_STORAGE_STATUS usb_storage_get_status();
int usb_storage_read_sector(unsigned int lba,unsigned char* buffer);
int usb_storage_write_sector(unsigned int lba,unsigned char* buffer);
void usb_storage_print_status();

#endif

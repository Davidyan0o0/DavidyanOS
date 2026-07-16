#include "kernel/console.h"
#include "kernel/system.h"
#include "kernel/shell.h"
#include "kernel/arch/x86/pic.h"
#include "kernel/arch/x86/idt.h"
#include "kernel/arch/x86/timer.h"
#include "kernel/memory/heap.h"
#include "kernel/fs/ramfs.h"
#include "kernel/net/net.h"
#include "kernel/device/device.h"
#include "drivers/storage/ramdisk.h"
#include "kernel/graphics/image.h"
#include "drivers/storage/ata.h"
#include "kernel/fs/diskfs.h"

void kernel_main()
{


    console_init();


    println("================================");

    println("        Davidyan OS");

    println("================================");


    println("");

    println("Kernel Started");

    println("VGA Driver OK");

    println("Console Driver OK");

    system_print_info();

    println("");

    pic_init();


    idt_init();

    timer_init();

    heap_init();

    device_init();

    ramdisk_init();

    ata_init();

    device_register("rd0","block");

    device_register("hd0","ata");

    device_register("net0","network");

    ramfs_init();

    diskfs_init();

    net_init();

    image_init();

    shell_init();


    asm volatile("sti");

    while(1)
    {

    }


}

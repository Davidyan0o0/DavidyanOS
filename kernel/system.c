#include "kernel/system.h"

#include "kernel/console.h"

void system_print_info()
{
    println("");
    println("[System Info]");
    println("Kernel: DavidyanOS 0.6");
    println("Architecture: x86 protected mode");
    println("Loader: LBA kernel load at sector 5, 64 sectors");
    println("IRQ: timer IRQ0, keyboard IRQ1");
    println("Timer: PIT 100Hz");
    println("RTC: CMOS date/time reader");
    println("Power: reboot and halt commands");
    println("FS: ramfs with ls, cat, write, rm, fsinfo");
    println("Block: rd0 RAM disk, 32 sectors, 512 bytes each");
    println("Devices: kernel device registry with devs command");
    println("Video: VGA color, boxes, rect drawing");
    println("Network: net0 IPv4 config, ARP table, ping/send stats");
    println("Shell: help, devs, disk, fs, gfx, net commands");
    println("Author: Davidyan");
    println("");
}

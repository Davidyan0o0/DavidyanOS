#include "kernel/system.h"

#include "kernel/console.h"

void system_print_info()
{
    println("System Info");
    println("  Kernel: Davidyan OS 0.2");
    println("  Architecture: x86 protected mode");
    println("  Memory: bootloader loads kernel at disk sector 5");
    println("  IRQ: keyboard interrupt enabled on IRQ1");
}

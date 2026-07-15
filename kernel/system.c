#include "kernel/system.h"

#include "kernel/console.h"

void system_print_info()
{
    println("");
    println("[System Info]");
    println("Kernel: DavidyanOS 0.3");
    println("Architecture: x86 protected mode");
    println("Loader: LBA kernel load at sector 5");
    println("IRQ: timer IRQ0, keyboard IRQ1");
    println("Timer: PIT 100Hz");
    println("Shell: help, clear, info, uptime, echo");
    println("Author: Davidyan");
    println("");
}

#include "kernel/console.h"
#include "kernel/system.h"
#include "kernel/shell.h"
#include "kernel/arch/x86/pic.h"
#include "kernel/arch/x86/idt.h"

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

    shell_init();


    asm volatile("sti");

    while(1)
    {

    }


}

#include "kernel/arch/x86/power.h"

#include "kernel/arch/x86/io.h"


void system_reboot()
{

    unsigned char status;


    asm volatile("cli");


    do
    {

        status = inb(0x64);

    } while(status&0x02);


    outb(0x64,0xFE);


    while(1)
    {

        asm volatile("hlt");

    }
}



void system_halt()
{

    asm volatile("cli");


    while(1)
    {

        asm volatile("hlt");

    }
}

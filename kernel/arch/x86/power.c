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



void system_shutdown()
{

    asm volatile("cli");


    outw(0x604,0x2000);
    outw(0xB004,0x2000);
    outw(0x4004,0x3400);


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

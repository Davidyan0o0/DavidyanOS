#include "pic.h"

#include "kernel/arch/x86/io.h"


void pic_init()
{

    // 主PIC

    outb(0x20,0x11);

    outb(0x21,0x20);

    outb(0x21,0x04);

    outb(0x21,0x01);



    // 开启IRQ0和IRQ1

    outb(
        0x21,
        0xfc
    );

}


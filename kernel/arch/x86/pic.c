#include "pic.h"


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



void outb(unsigned short port,unsigned char data)
{
    asm volatile(
        "outb %0,%1"
        :
        :"a"(data),
        "Nd"(port))
    ;
}

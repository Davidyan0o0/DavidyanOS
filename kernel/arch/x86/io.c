#include "kernel/arch/x86/io.h"


unsigned char inb(unsigned short port)
{

    unsigned char ret;


    asm volatile(
        "inb %1,%0"
        :"=a"(ret)
        :"Nd"(port)
    );


    return ret;

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



unsigned short inw(unsigned short port)
{

    unsigned short ret;


    asm volatile(
        "inw %1,%0"
        :"=a"(ret)
        :"Nd"(port)
    );


    return ret;

}



unsigned int inl(unsigned short port)
{

    unsigned int ret;


    asm volatile(
        "inl %1,%0"
        :"=a"(ret)
        :"Nd"(port)
    );


    return ret;

}



void outw(unsigned short port,unsigned short data)
{

    asm volatile(
        "outw %0,%1"
        :
        :"a"(data),
        "Nd"(port))
    ;

}



void outl(unsigned short port,unsigned int data)
{

    asm volatile(
        "outl %0,%1"
        :
        :"a"(data),
        "Nd"(port))
    ;

}



void insw(unsigned short port,void* addr,int count)
{

    asm volatile(
        "cld; rep insw"
        :"+D"(addr),"+c"(count)
        :"d"(port)
        :"memory"
    );

}



void outsw(unsigned short port,void* addr,int count)
{

    asm volatile(
        "cld; rep outsw"
        :"+S"(addr),"+c"(count)
        :"d"(port)
    );

}



void io_wait()
{

    outb(0x80,0);

}

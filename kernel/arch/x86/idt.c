#include "idt.h"


typedef struct
{

    unsigned short offset_low;

    unsigned short selector;

    unsigned char zero;

    unsigned char type_attr;

    unsigned short offset_high;


}__attribute__((packed))
IDT_ENTRY;



typedef struct
{

    unsigned short limit;

    unsigned int base;

}__attribute__((packed))
IDT_PTR;



IDT_ENTRY idt[256];


IDT_PTR idtp;



extern void irq1();



void idt_set_gate(
    int num,
    unsigned int handler
)
{

    idt[num].offset_low =
        handler & 0xffff;


    idt[num].selector =
        0x08;


    idt[num].zero=0;


    idt[num].type_attr=
        0x8E;


    idt[num].offset_high =
        (handler>>16)&0xffff;

}




void idt_load()
{

    asm volatile(
        "lidt (%0)"
        :
        :"r"(&idtp)
    );

}





void idt_init()
{

    int i;


    for(i=0;i<256;i++)
    {

        idt_set_gate(
            i,
            0
        );

    }


    idt_set_gate(
        33,
        (unsigned int)irq1
    );



    idtp.limit =
        sizeof(idt)-1;


    idtp.base =
        (unsigned int)&idt;


    idt_load();


}

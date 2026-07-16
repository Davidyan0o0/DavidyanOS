#include "idt.h"

// 中断描述符结构
typedef struct
{

    unsigned short offset_low;

    unsigned short selector;

    unsigned char zero;

    unsigned char type_attr;

    unsigned short offset_high;


}__attribute__((packed)) // __attribute__((packed))禁止编译器自动填充结构体对齐字节
IDT_ENTRY;


// CPU加载IDT用
typedef struct
{

    unsigned short limit;

    unsigned int base;

}__attribute__((packed))
IDT_PTR;



IDT_ENTRY idt[256]; // x86中中断号范围0~255


IDT_PTR idtp;


// 汇编函数

// 硬件中断
extern void irq0();
extern void irq1();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();


// 设置一个中断入口
void idt_set_gate(int num,unsigned int handler)
{

    idt[num].offset_low = handler & 0xffff;


    idt[num].selector = 0x08; // 发生中断后CPU跳到GDT第1个描述符


    idt[num].zero=0;


    idt[num].type_attr=
        0x8E;


    idt[num].offset_high =
        (handler>>16)&0xffff;

}



// 加载IDT生成lidt地址 CPU执行后IDTR寄存器->保存IDT位置
void idt_load()
{

    asm volatile(
        "lidt (%0)"
        :
        :"r"(&idtp)
    );

}




// 中断初始化
void idt_init()
{

    int i;

    // 先全部设置为空,否则某个未知中断发生,CPU跳到随机地址直接Triple Fault -> 重启
    for(i=0;i<256;i++)
    {
        idt_set_gate(i,0);
    }

    // 注册CPU异常
    idt_set_gate(0,(unsigned int)isr0);
    idt_set_gate(1,(unsigned int)isr1);
    idt_set_gate(2,(unsigned int)isr2);
    idt_set_gate(3,(unsigned int)isr3);
    idt_set_gate(4,(unsigned int)isr4);
    idt_set_gate(5,(unsigned int)isr5);
    idt_set_gate(6,(unsigned int)isr6);
    idt_set_gate(7,(unsigned int)isr7);
    idt_set_gate(8,(unsigned int)isr8);
    idt_set_gate(9,(unsigned int)isr9);
    idt_set_gate(10,(unsigned int)isr10);
    idt_set_gate(11,(unsigned int)isr11);
    idt_set_gate(12,(unsigned int)isr12);
    idt_set_gate(13,(unsigned int)isr13);
    idt_set_gate(14,(unsigned int)isr14);
    idt_set_gate(15,(unsigned int)isr15);
    idt_set_gate(16,(unsigned int)isr16);
    idt_set_gate(17,(unsigned int)isr17);
    idt_set_gate(18,(unsigned int)isr18);
    idt_set_gate(19,(unsigned int)isr19);
    idt_set_gate(20,(unsigned int)isr20);
    idt_set_gate(21,(unsigned int)isr21);
    idt_set_gate(22,(unsigned int)isr22);
    idt_set_gate(23,(unsigned int)isr23);
    idt_set_gate(24,(unsigned int)isr24);
    idt_set_gate(25,(unsigned int)isr25);
    idt_set_gate(26,(unsigned int)isr26);
    idt_set_gate(27,(unsigned int)isr27);
    idt_set_gate(28,(unsigned int)isr28);
    idt_set_gate(29,(unsigned int)isr29);
    idt_set_gate(30,(unsigned int)isr30);
    idt_set_gate(31,(unsigned int)isr31);

    // 注册硬件IRQ
    idt_set_gate(32,(unsigned int)irq0);
    idt_set_gate(33,(unsigned int)irq1);



    idtp.limit = sizeof(idt)-1;


    idtp.base = (unsigned int)&idt;


    idt_load();


}

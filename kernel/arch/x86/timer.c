#include "kernel/arch/x86/timer.h"

#include "kernel/arch/x86/pic.h"


#define PIT_BASE_FREQUENCY 1193182
#define TIMER_FREQUENCY 100


static unsigned int timer_ticks = 0;


void timer_init()
{

    unsigned int divisor = PIT_BASE_FREQUENCY/TIMER_FREQUENCY;


    outb(0x43,0x36);
    outb(0x40,(unsigned char)(divisor&0xFF));
    outb(0x40,(unsigned char)((divisor>>8)&0xFF));

}



void timer_handler()
{

    timer_ticks++;

}



unsigned int timer_get_ticks()
{

    return timer_ticks;

}



unsigned int timer_get_seconds()
{

    return timer_ticks/TIMER_FREQUENCY;

}

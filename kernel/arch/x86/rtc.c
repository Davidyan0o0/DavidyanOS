#include "kernel/arch/x86/rtc.h"

#include "kernel/arch/x86/io.h"


#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71


static unsigned char cmos_read(unsigned char reg)
{

    outb(CMOS_ADDRESS,reg);
    io_wait();
    return inb(CMOS_DATA);

}



static unsigned char rtc_is_updating()
{

    return cmos_read(0x0A)&0x80;

}



static unsigned int bcd_to_binary(unsigned int value)
{

    return (value&0x0F)+((value/16)*10);

}



void rtc_read_time(RTC_TIME* time)
{

    unsigned char status_b;


    while(rtc_is_updating())
    {

    }


    time->second = cmos_read(0x00);
    time->minute = cmos_read(0x02);
    time->hour = cmos_read(0x04);
    time->day = cmos_read(0x07);
    time->month = cmos_read(0x08);
    time->year = cmos_read(0x09);


    status_b = cmos_read(0x0B);


    if((status_b&0x04)==0)
    {

        time->second = bcd_to_binary(time->second);
        time->minute = bcd_to_binary(time->minute);
        time->hour = bcd_to_binary(time->hour&0x7F) | (time->hour&0x80);
        time->day = bcd_to_binary(time->day);
        time->month = bcd_to_binary(time->month);
        time->year = bcd_to_binary(time->year);

    }


    if((status_b&0x02)==0 && (time->hour&0x80))
    {

        time->hour = ((time->hour&0x7F)+12)%24;

    }


    time->year += 2000;

}

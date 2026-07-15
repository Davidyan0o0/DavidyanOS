#include "console.h"
#include "../drivers/video/vga.h"



void console_init()
{

    vga_init();

}



void print(char* str)
{

    vga_write(str);

}



void println(char* str)
{

    vga_write(str);

    vga_write("\n");

}



void print_uint(unsigned int value)
{

    char buffer[11];
    int index = 0;


    if(value==0)
    {

        vga_put_char('0');
        return;

    }


    while(value>0)
    {

        buffer[index] = '0' + (value%10);
        value = value/10;
        index++;

    }


    while(index>0)
    {

        index--;
        vga_put_char(buffer[index]);

    }

}

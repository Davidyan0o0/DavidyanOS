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
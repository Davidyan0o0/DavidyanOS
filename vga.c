#include "vga.h"


#define VGA_MEMORY 0xB8000


#define VGA_WIDTH 80
#define VGA_HEIGHT 25



unsigned short* vga_buffer =
    (unsigned short*)VGA_MEMORY;



int cursor_x = 0;
int cursor_y = 0;



// 颜色
#define WHITE_ON_BLACK 0x0F



void vga_clear()
{

    for(int y=0;y<VGA_HEIGHT;y++)
    {
        for(int x=0;x<VGA_WIDTH;x++)
        {

            int index=y*VGA_WIDTH+x;


            vga_buffer[index]
                = (WHITE_ON_BLACK<<8)
                | ' ';

        }
    }


    cursor_x=0;
    cursor_y=0;

}



void vga_init()
{
    vga_clear();
}



void vga_put_char(char c)
{


    if(c=='\n')
    {
        cursor_x=0;
        cursor_y++;
        return;
    }



    int index =
        cursor_y*VGA_WIDTH+cursor_x;



    vga_buffer[index]
        =
        (WHITE_ON_BLACK<<8)
        | c;



    cursor_x++;



    if(cursor_x>=VGA_WIDTH)
    {
        cursor_x=0;
        cursor_y++;
    }


}



void vga_write(char* str)
{

    while(*str)
    {

        vga_put_char(*str);

        str++;

    }

}
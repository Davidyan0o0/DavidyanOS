#include "vga.h"


#define VGA_MEMORY 0xB8000

#define VGA_WIDTH 80
#define VGA_HEIGHT 25


#define VGA_DEFAULT_COLOR 0x0F



unsigned short* vga_buffer = (unsigned short*)VGA_MEMORY;



int cursor_x = 0;
int cursor_y = 0;
static unsigned char vga_color = VGA_DEFAULT_COLOR;



static unsigned short vga_entry(char c)
{

    return (vga_color<<8) | c;

}

static unsigned short vga_entry_color(char c,unsigned char color)
{

    return (color<<8) | c;

}



static void outb(unsigned short port,unsigned char data)
{

    asm volatile(
        "outb %0,%1"
        :
        :"a"(data),
        "Nd"(port))
    ;

}



static void vga_update_cursor()
{

    unsigned short position =
        cursor_y*VGA_WIDTH+cursor_x;


    outb(0x3D4,0x0F);
    outb(0x3D5,(unsigned char)(position&0xFF));

    outb(0x3D4,0x0E);
    outb(0x3D5,(unsigned char)((position>>8)&0xFF));

}



static void vga_scroll()
{

    int x;
    int y;


    for(y=1;y<VGA_HEIGHT;y++)
    {

        for(x=0;x<VGA_WIDTH;x++)
        {

            vga_buffer[(y-1)*VGA_WIDTH+x] =
                vga_buffer[y*VGA_WIDTH+x];

        }

    }


    for(x=0;x<VGA_WIDTH;x++)
    {

        vga_buffer[(VGA_HEIGHT-1)*VGA_WIDTH+x] =
            vga_entry(' ');

    }


    cursor_y = VGA_HEIGHT-1;

}



static void vga_newline()
{

    cursor_x = 0;

    cursor_y++;


    if(cursor_y >= VGA_HEIGHT)
    {

        vga_scroll();

    }


    vga_update_cursor();

}




void vga_init()
{

    vga_clear();

}



void vga_clear()
{

    int x;
    int y;


    for(y=0;y<VGA_HEIGHT;y++)
    {

        for(x=0;x<VGA_WIDTH;x++)
        {

            int index =
                y*VGA_WIDTH+x;


            vga_buffer[index]
                =
                vga_entry(' ');

        }

    }


    cursor_x=0;
    cursor_y=0;

    vga_update_cursor();

}





void vga_put_char(char c)
{


    if(c=='\n')
    {

        vga_newline();

        return;

    }


    if(c=='\b')
    {

        vga_backspace();

        return;

    }



    int index =
        cursor_y*VGA_WIDTH
        +
        cursor_x;



    vga_buffer[index]
        =
        vga_entry(c);



    cursor_x++;



    if(cursor_x>=VGA_WIDTH)
    {

        vga_newline();

        return;

    }


    vga_update_cursor();


}





void vga_backspace()
{

    int index;


    if(cursor_x==0 && cursor_y==0)
    {

        return;

    }


    if(cursor_x==0)
    {

        cursor_y--;
        cursor_x = VGA_WIDTH-1;

    }
    else
    {

        cursor_x--;

    }


    index = cursor_y*VGA_WIDTH+cursor_x;

    vga_buffer[index] = vga_entry(' ');

    vga_update_cursor();

}





void vga_write(char* str)
{

    while(*str)
    {

        vga_put_char(*str);

        str++;

    }

}



void vga_set_color(unsigned char foreground,unsigned char background)
{

    vga_color = (background<<4) | (foreground&0x0F);

}



unsigned char vga_get_color()
{

    return vga_color;

}



void vga_put_at(int x,int y,char c,unsigned char color)
{

    if(x<0 || y<0 || x>=VGA_WIDTH || y>=VGA_HEIGHT)
    {

        return;

    }


    vga_buffer[y*VGA_WIDTH+x] = vga_entry_color(c,color);

}



void vga_fill_rect(int x,int y,int width,int height,char c,unsigned char color)
{

    int px;
    int py;


    for(py=y;py<y+height;py++)
    {

        for(px=x;px<x+width;px++)
        {

            vga_put_at(px,py,c,color);

        }

    }

}



void vga_draw_box(int x,int y,int width,int height,unsigned char color)
{

    int i;


    if(width<2 || height<2)
    {

        return;

    }


    vga_put_at(x,y,'+',color);
    vga_put_at(x+width-1,y,'+',color);
    vga_put_at(x,y+height-1,'+',color);
    vga_put_at(x+width-1,y+height-1,'+',color);

    for(i=1;i<width-1;i++)
    {

        vga_put_at(x+i,y,'-',color);
        vga_put_at(x+i,y+height-1,'-',color);

    }

    for(i=1;i<height-1;i++)
    {

        vga_put_at(x,y+i,'|',color);
        vga_put_at(x+width-1,y+i,'|',color);

    }

}

#include "vga.h"


#define VGA_MEMORY 0xB8000
#define VGA_GRAPHICS_MEMORY 0xA0000

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_GRAPHICS_WIDTH 320
#define VGA_GRAPHICS_HEIGHT 200
#define VGA_FONT_BYTES 8192


#define VGA_DEFAULT_COLOR 0x0F



unsigned short* vga_buffer = (unsigned short*)VGA_MEMORY;
static unsigned char* vga_graphics_buffer = (unsigned char*)VGA_GRAPHICS_MEMORY;
static unsigned char vga_font_backup[VGA_FONT_BYTES];



int cursor_x = 0;
int cursor_y = 0;
static unsigned char vga_color = VGA_DEFAULT_COLOR;
static int vga_graphics_mode = 0;
static int vga_font_saved = 0;



static unsigned char vga_320x200x256_regs[] =
{
    0x63,
    0x03,0x01,0x0F,0x00,0x0E,
    0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
    0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,
    0x9C,0x0E,0x8F,0x28,0x40,0x96,0xB9,0xA3,
    0xFF,
    0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,
    0xFF,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x41,0x00,0x0F,0x00,0x00
};



static unsigned char vga_80x25_text_regs[] =
{
    0x67,
    0x03,0x00,0x03,0x00,0x02,
    0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
    0x00,0x4F,0x0D,0x0E,0x00,0x00,0x00,0x50,
    0x9C,0x0E,0x8F,0x28,0x1F,0x96,0xB9,0xA3,
    0xFF,
    0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
    0xFF,
    0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
    0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x0C,0x00,0x0F,0x08,0x00
};



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



static unsigned char inb(unsigned short port)
{

    unsigned char value;

    asm volatile(
        "inb %1,%0"
        :"=a"(value)
        :"Nd"(port));

    return value;

}



static void vga_write_regs(unsigned char* regs)
{

    unsigned int i;

    outb(0x3C2,*regs);
    regs++;

    for(i=0;i<5;i++)
    {
        outb(0x3C4,(unsigned char)i);
        outb(0x3C5,regs[i]);
    }
    regs += 5;

    outb(0x3D4,0x03);
    outb(0x3D5,(unsigned char)(regs[0x03] | 0x80));
    outb(0x3D4,0x11);
    outb(0x3D5,(unsigned char)(regs[0x11] & 0x7F));

    for(i=0;i<25;i++)
    {
        outb(0x3D4,(unsigned char)i);
        outb(0x3D5,regs[i]);
    }
    regs += 25;

    for(i=0;i<9;i++)
    {
        outb(0x3CE,(unsigned char)i);
        outb(0x3CF,regs[i]);
    }
    regs += 9;

    for(i=0;i<21;i++)
    {
        inb(0x3DA);
        outb(0x3C0,(unsigned char)i);
        outb(0x3C0,regs[i]);
    }

    inb(0x3DA);
    outb(0x3C0,0x20);

}



static void vga_select_font_plane()
{

    outb(0x3C4,0x02);
    outb(0x3C5,0x04);
    outb(0x3C4,0x04);
    outb(0x3C5,0x06);

    outb(0x3CE,0x04);
    outb(0x3CF,0x02);
    outb(0x3CE,0x05);
    outb(0x3CF,0x00);
    outb(0x3CE,0x06);
    outb(0x3CF,0x04);

}



static void vga_save_font()
{

    unsigned int i;
    unsigned char* font_memory = (unsigned char*)VGA_GRAPHICS_MEMORY;

    if(vga_font_saved)
    {
        return;
    }

    vga_select_font_plane();

    for(i=0;i<VGA_FONT_BYTES;i++)
    {
        vga_font_backup[i] = font_memory[i];
    }

    vga_write_regs(vga_80x25_text_regs);
    vga_font_saved = 1;

}



static void vga_restore_font()
{

    unsigned int i;
    unsigned char* font_memory = (unsigned char*)VGA_GRAPHICS_MEMORY;

    if(!vga_font_saved)
    {
        return;
    }

    vga_select_font_plane();

    for(i=0;i<VGA_FONT_BYTES;i++)
    {
        font_memory[i] = vga_font_backup[i];
    }

    vga_write_regs(vga_80x25_text_regs);

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

    vga_graphics_mode = 0;
    vga_clear();

}



void vga_clear()
{

    int x;
    int y;


    if(vga_graphics_mode)
    {
        vga_clear_graphics(0);
        return;
    }


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



void vga_set_text_mode()
{

    vga_write_regs(vga_80x25_text_regs);
    vga_restore_font();
    vga_graphics_mode = 0;
    vga_clear();

}



void vga_set_graphics_mode()
{

    vga_save_font();
    vga_write_regs(vga_320x200x256_regs);
    vga_graphics_mode = 1;
    vga_clear_graphics(0);

}



int vga_is_graphics_mode()
{

    return vga_graphics_mode;

}



void vga_clear_graphics(unsigned char color)
{

    unsigned int i;

    for(i=0;i<VGA_GRAPHICS_WIDTH*VGA_GRAPHICS_HEIGHT;i++)
    {
        vga_graphics_buffer[i] = color;
    }

}



void vga_put_pixel(int x,int y,unsigned char color)
{

    if(x<0 || y<0 || x>=VGA_GRAPHICS_WIDTH || y>=VGA_GRAPHICS_HEIGHT)
    {
        return;
    }

    vga_graphics_buffer[y*VGA_GRAPHICS_WIDTH+x] = color;

}



void vga_fill_graphics_rect(int x,int y,int width,int height,unsigned char color)
{

    int px;
    int py;

    for(py=y;py<y+height;py++)
    {
        for(px=x;px<x+width;px++)
        {
            vga_put_pixel(px,py,color);
        }
    }

}



void vga_set_palette_color(unsigned char index,unsigned char r,unsigned char g,unsigned char b)
{

    outb(0x3C8,index);
    outb(0x3C9,(unsigned char)(r>>2));
    outb(0x3C9,(unsigned char)(g>>2));
    outb(0x3C9,(unsigned char)(b>>2));

}





void vga_put_char(char c)
{


    if(vga_graphics_mode)
    {
        return;
    }


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

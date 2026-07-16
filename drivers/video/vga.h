#ifndef VGA_H
#define VGA_H


void vga_init();

void vga_clear();

void vga_put_char(char c);

void vga_backspace();

void vga_write(char* str);

void vga_set_color(unsigned char foreground,unsigned char background);

unsigned char vga_get_color();

void vga_put_at(int x,int y,char c,unsigned char color);

void vga_fill_rect(int x,int y,int width,int height,char c,unsigned char color);

void vga_draw_box(int x,int y,int width,int height,unsigned char color);


#endif

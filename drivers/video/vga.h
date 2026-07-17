#ifndef VGA_H
#define VGA_H


void vga_init();

void vga_clear();

void vga_set_text_mode();

void vga_set_graphics_mode();

int vga_is_graphics_mode();

void vga_clear_graphics(unsigned char color);

void vga_put_pixel(int x,int y,unsigned char color);

void vga_fill_graphics_rect(int x,int y,int width,int height,unsigned char color);

void vga_set_palette_color(unsigned char index,unsigned char r,unsigned char g,unsigned char b);

void vga_put_char(char c);

void vga_backspace();

void vga_write(char* str);

void vga_set_color(unsigned char foreground,unsigned char background);

unsigned char vga_get_color();

void vga_put_at(int x,int y,char c,unsigned char color);

void vga_fill_rect(int x,int y,int width,int height,char c,unsigned char color);

void vga_draw_box(int x,int y,int width,int height,unsigned char color);


#endif

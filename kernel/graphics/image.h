#ifndef IMAGE_H
#define IMAGE_H

#define IMAGE_MAX_WIDTH 128
#define IMAGE_MAX_HEIGHT 128
#define IMAGE_MAX_PIXELS (IMAGE_MAX_WIDTH*IMAGE_MAX_HEIGHT)

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} IMAGE_PIXEL;

typedef struct
{
    unsigned int width;
    unsigned int height;
    IMAGE_PIXEL pixels[IMAGE_MAX_PIXELS];
} IMAGE;

void image_init();
int image_parse_ppm(char* text,IMAGE* image);
int image_load_file(char* name,IMAGE* image);
void image_make_demo(IMAGE* image);
void image_print_info(IMAGE* image);
void image_display(IMAGE* image,int x,int y,int scale);
void image_display_graphics(IMAGE* image,int x,int y,int scale);
void image_display_big_demo();

#endif

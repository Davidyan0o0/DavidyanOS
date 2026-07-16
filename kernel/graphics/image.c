#include "kernel/graphics/image.h"

#include "kernel/console.h"
#include "kernel/fs/ramfs.h"
#include "drivers/video/vga.h"

static unsigned int image_read_uint(char** text,unsigned int* value)
{
    unsigned int number = 0;
    unsigned int digits = 0;

    while(**text==' ' || **text=='\n' || **text=='\r' || **text=='\t')
    {
        (*text)++;
    }

    if(**text=='#')
    {
        while(**text && **text!='\n')
        {
            (*text)++;
        }

        return image_read_uint(text,value);
    }

    while(**text>='0' && **text<='9')
    {
        number = number*10 + (**text-'0');
        (*text)++;
        digits++;
    }

    *value = number;
    return digits!=0;
}

static unsigned char image_scale_channel(unsigned int value,unsigned int max)
{
    if(max==0 || value>max)
    {
        return 0;
    }

    return (unsigned char)((value*255)/max);
}

static unsigned char image_vga_color(IMAGE_PIXEL pixel)
{
    unsigned char color = 0;

    if(pixel.r>96) color |= 0x04;
    if(pixel.g>96) color |= 0x02;
    if(pixel.b>96) color |= 0x01;

    if(pixel.r>180 && pixel.g>180 && pixel.b>180)
    {
        color |= 0x08;
    }

    return color;
}

void image_init()
{
}

int image_parse_ppm(char* text,IMAGE* image)
{
    unsigned int width;
    unsigned int height;
    unsigned int max_value;
    unsigned int x;
    unsigned int y;
    unsigned int index;
    unsigned int r;
    unsigned int g;
    unsigned int b;

    if(text[0]!='P' || text[1]!='3')
    {
        return 0;
    }

    text += 2;
    if(!image_read_uint(&text,&width) || !image_read_uint(&text,&height) || !image_read_uint(&text,&max_value))
    {
        return 0;
    }

    if(width==0 || height==0 || width>IMAGE_MAX_WIDTH || height>IMAGE_MAX_HEIGHT || max_value==0)
    {
        return 0;
    }

    image->width = width;
    image->height = height;

    for(y=0;y<height;y++)
    {
        for(x=0;x<width;x++)
        {
            if(!image_read_uint(&text,&r) || !image_read_uint(&text,&g) || !image_read_uint(&text,&b))
            {
                return 0;
            }

            index = y*width+x;
            image->pixels[index].r = image_scale_channel(r,max_value);
            image->pixels[index].g = image_scale_channel(g,max_value);
            image->pixels[index].b = image_scale_channel(b,max_value);
        }
    }

    return 1;
}

int image_load_file(char* name,IMAGE* image)
{
    RAMFS_NODE* node = ramfs_find(name);

    if(node==0 || node->directory)
    {
        return 0;
    }

    return image_parse_ppm(node->data,image);
}

void image_make_demo(IMAGE* image)
{
    unsigned int x;
    unsigned int y;
    unsigned int index;

    image->width = 24;
    image->height = 12;

    for(y=0;y<image->height;y++)
    {
        for(x=0;x<image->width;x++)
        {
            index = y*image->width+x;
            image->pixels[index].r = (unsigned char)(x*10);
            image->pixels[index].g = (unsigned char)(y*20);
            image->pixels[index].b = (unsigned char)(255-(x*8));
        }
    }
}

void image_print_info(IMAGE* image)
{
    print("image ");
    print_uint(image->width);
    print("x");
    print_uint(image->height);
    print(" pixels=");
    print_uint(image->width*image->height);
    println("");
}

void image_display(IMAGE* image,int x,int y,int scale)
{
    unsigned int px;
    unsigned int py;
    unsigned int sx;
    unsigned int sy;
    unsigned int index;
    unsigned char color;

    if(scale<1)
    {
        scale = 1;
    }

    for(py=0;py<image->height;py++)
    {
        for(px=0;px<image->width;px++)
        {
            index = py*image->width+px;
            color = image_vga_color(image->pixels[index]);

            for(sy=0;sy<(unsigned int)scale;sy++)
            {
                for(sx=0;sx<(unsigned int)scale;sx++)
                {
                    vga_put_at(x+(int)(px*scale+sx),y+(int)(py*scale+sy),' ',(unsigned char)(color<<4 | 0x0F));
                }
            }
        }
    }
}

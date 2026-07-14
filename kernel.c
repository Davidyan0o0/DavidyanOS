#include "vga.h"

void kernel_main()
{

    char* video = (char*)0xb8000;


    char* msg = "Hello C Kernel!";


    int i=0;


    while(msg[i])
    {

        video[i*2]=msg[i];

        video[i*2+1]=0x0f;


        i++;
    }

    vga_init();


    vga_write(
        "Hello VGA Driver!\n"
    );


    vga_write(
        "Davidyan OS"
    );


    while(1)
    {

    }

}
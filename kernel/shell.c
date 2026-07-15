#include "kernel/shell.h"

#include "kernel/console.h"
#include "kernel/system.h"
#include "kernel/lib/string.h"
#include "kernel/arch/x86/timer.h"
#include "drivers/video/vga.h"


#define SHELL_INPUT_MAX 128


static char input_buffer[SHELL_INPUT_MAX];
static int input_length = 0;


static void shell_prompt()
{

    print("DavidyanOS> ");

}



static void shell_reset_input()
{

    input_length = 0;
    input_buffer[0] = 0;

}



static void shell_execute(char* command)
{

    if(strcmp(command,"")==0)
    {

        return;

    }


    if(strcmp(command,"help")==0)
    {

        println("Commands: help, clear, info, uptime, echo");
        return;

    }


    if(strcmp(command,"clear")==0)
    {

        vga_clear();
        return;

    }


    if(strcmp(command,"info")==0)
    {

        system_print_info();
        return;

    }


    if(strcmp(command,"uptime")==0)
    {

        print("Uptime: ");
        print_uint(timer_get_seconds());
        print("s (");
        print_uint(timer_get_ticks());
        println(" ticks)");
        return;

    }


    if(strncmp(command,"echo ",5)==0)
    {

        println(command+5);
        return;

    }


    print("Unknown command: ");
    println(command);

}



void shell_init()
{

    shell_reset_input();
    shell_prompt();

}



void shell_handle_key(char c)
{

    if(c=='\n')
    {

        vga_put_char('\n');
        input_buffer[input_length] = 0;
        shell_execute(input_buffer);
        shell_reset_input();
        shell_prompt();
        return;

    }


    if(c=='\b')
    {

        if(input_length>0)
        {

            input_length--;
            input_buffer[input_length] = 0;
            vga_backspace();

        }


        return;

    }


    if(input_length>=SHELL_INPUT_MAX-1)
    {

        return;

    }


    input_buffer[input_length] = c;
    input_length++;
    input_buffer[input_length] = 0;
    vga_put_char(c);

}

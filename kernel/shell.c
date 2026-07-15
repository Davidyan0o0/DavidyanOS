#include "kernel/shell.h"

#include "kernel/console.h"
#include "kernel/system.h"
#include "drivers/video/vga.h"


#define SHELL_INPUT_MAX 128


static char input_buffer[SHELL_INPUT_MAX];
static int input_length = 0;


static int string_equals(char* left,char* right)
{

    while(*left && *right)
    {

        if(*left!=*right)
        {

            return 0;

        }


        left++;
        right++;

    }


    return *left==0 && *right==0;

}



static int string_starts_with(char* text,char* prefix)
{

    while(*prefix)
    {

        if(*text!=*prefix)
        {

            return 0;

        }


        text++;
        prefix++;

    }


    return 1;

}



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

    if(string_equals(command,""))
    {

        return;

    }


    if(string_equals(command,"help"))
    {

        println("Commands: help, clear, info, echo");
        return;

    }


    if(string_equals(command,"clear"))
    {

        vga_clear();
        return;

    }


    if(string_equals(command,"info"))
    {

        system_print_info();
        return;

    }


    if(string_starts_with(command,"echo "))
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

#include "kernel/arch/x86/exception.h"

#include "kernel/console.h"


void exception_handler(unsigned int vector,unsigned int error_code)
{

    asm volatile("cli");

    println("");
    println("CPU Exception");
    print("  Vector: ");
    print_uint(vector);
    println("");
    print("  Error Code: ");
    print_uint(error_code);
    println("");
    println("System halted");

    while(1)
    {

        asm volatile("hlt");

    }
}

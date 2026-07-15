[BITS 32]

global _start
extern kernel_main


_start:


    cli


    ; 设置栈

    mov esp,0x90000


    call kernel_main



halt:

    hlt
    jmp halt

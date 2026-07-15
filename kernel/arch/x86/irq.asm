[BITS 32]


global irq0
global irq1


extern timer_handler
extern keyboard_handler


irq0:


    pusha


    call timer_handler


    popa


    mov al,0x20
    out 0x20,al


    iretd



irq1:


    pusha


    call keyboard_handler


    popa


    mov al,0x20
    out 0x20,al


    iretd

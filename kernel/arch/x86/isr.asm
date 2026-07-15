[BITS 32]


extern exception_handler


%macro ISR_NO_ERROR 1
global isr%1
isr%1:
    cli
    push dword 0
    push dword %1
    call exception_handler
.halt:
    hlt
    jmp .halt
%endmacro


%macro ISR_ERROR 1
global isr%1
isr%1:
    cli
    push dword %1
    call exception_handler
.halt:
    hlt
    jmp .halt
%endmacro


ISR_NO_ERROR 0
ISR_NO_ERROR 1
ISR_NO_ERROR 2
ISR_NO_ERROR 3
ISR_NO_ERROR 4
ISR_NO_ERROR 5
ISR_NO_ERROR 6
ISR_NO_ERROR 7
ISR_ERROR 8
ISR_NO_ERROR 9
ISR_ERROR 10
ISR_ERROR 11
ISR_ERROR 12
ISR_ERROR 13
ISR_ERROR 14
ISR_NO_ERROR 15
ISR_NO_ERROR 16
ISR_ERROR 17
ISR_NO_ERROR 18
ISR_NO_ERROR 19
ISR_NO_ERROR 20
ISR_ERROR 21
ISR_NO_ERROR 22
ISR_NO_ERROR 23
ISR_NO_ERROR 24
ISR_NO_ERROR 25
ISR_NO_ERROR 26
ISR_NO_ERROR 27
ISR_NO_ERROR 28
ISR_NO_ERROR 29
ISR_ERROR 30
ISR_NO_ERROR 31

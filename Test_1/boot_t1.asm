[BITS 16]

org 0x7c00

start:

    mov si,msg

print:
    lodsb

    cmp al,0
    je halt

    mov ah,0x0e
    int 0x10

    jmp print


halt:
    cli
    hlt


msg:
    db "Hello My OS!",0


times 510-($-$$) db 0

dw 0xaa55
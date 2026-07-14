[BITS 16]

org 0x7c00


start:

    cli

    xor ax,ax
    mov ds,ax
    mov es,ax

    mov ah,0x02
    mov al,4          ; 读取4个扇区

    mov ch,0
    mov cl,2          ; 从第二扇区开始

    mov dh,0
    mov dl,0x80       ; 硬盘


    mov bx,0x8000

    int 0x13

    jc disk_error


    jmp 0x0000:0x8000



disk_error:

    mov si,msg

.print:

    lodsb

    cmp al,0
    je $

    mov ah,0x0e
    int 0x10

    jmp .print



msg:
db "Stage2 Load Error",0



times 510-($-$$) db 0

dw 0xaa55
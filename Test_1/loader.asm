[BITS 16]

org 0x8000


start:

    cli


    ; 加载GDT

    lgdt [gdt_descriptor]



    ; 开启保护模式

    mov eax,cr0

    or eax,1

    mov cr0,eax



    ; 远跳转刷新CS

    jmp 0x08:protected_mode





; =====================
; GDT
; =====================

gdt_start:


; NULL

dq 0



; CODE段

gdt_code:

    dw 0xffff       ; limit

    dw 0x0000       ; base 0-15

    db 0x00         ; base 16-23

    db 10011010b    ; code

    db 11001111b    ; flags

    db 0x00         ; base 24-31





; DATA段

gdt_data:

    dw 0xffff

    dw 0x0000

    db 0x00

    db 10010010b

    db 11001111b

    db 0x00



gdt_end:



gdt_descriptor:

    dw gdt_end-gdt_start-1

    dd gdt_start





; =====================
; 32位保护模式
; =====================


[BITS 32]


protected_mode:


    ; 初始化数据段

    mov ax,0x10

    mov ds,ax

    mov es,ax

    mov ss,ax

    mov fs,ax

    mov gs,ax



    ; 设置栈

    mov esp,0x90000

    ; VGA文本模式显存

    mov edi,0xb8000+1920



    mov esi,msg



print:


    lodsb


    cmp al,0

    je halt



    mov [edi],al

    mov byte [edi+1],0x07


    add edi,2


    jmp print





halt:


    cli

    hlt

    jmp halt





msg:

db "Protected Mode OK!",0
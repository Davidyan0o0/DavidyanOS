; loader.asm

[BITS 16]

org 0x8000



start:

    cli


    lgdt [gdt_descriptor]


    ; 开启保护模式

    mov eax,cr0

    or eax,1

    mov cr0,eax



    ; 刷新CS

    jmp 0x08:protected_mode





; ==========================
; GDT
; ==========================


gdt_start:


; NULL段

gdt_null:
    dq 0



; Code Segment

gdt_code:

    dw 0xffff
    dw 0x0000

    db 0x00

    db 10011010b

    db 11001111b

    db 0x00



; Data Segment


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




; ==========================
; 32位保护模式
; ==========================


[BITS 32]


protected_mode:


    ; 设置数据段

    mov ax,0x10

    mov ds,ax
    mov es,ax
    mov ss,ax



    mov esi,msg



print:


    lodsb


    cmp al,0

    je halt



    ; VGA显存

    mov edi,0xb8000


    mov [edi],al


    add edi,2


    jmp print



halt:

    cli

    hlt




msg:

    db "Hello Protected Mode!",0
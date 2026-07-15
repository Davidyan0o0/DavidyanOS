[BITS 16]

org 0x8000

dw 0xDADA

start:


    cli
    cld

    xor ax,ax
    mov ds,ax
    mov es,ax
    mov [BOOT_DRIVE],dl

    sti

    mov al,'2'
    call print_char

    xor ax,ax
    mov dl,[BOOT_DRIVE]
    int 0x13


; ------------------
; 加载Kernel
; ------------------


    mov si,KERNEL_DAP

    mov ah,0x42

    mov dl,[BOOT_DRIVE]



    int 0x13


    jc disk_error



; ------------------
; 打印
; ------------------

    mov si,msg


print:

    lodsb

    cmp al,0

    je enter_pm


    mov ah,0x0e

    int 0x10

    jmp print



; ------------------
; GDT
; ------------------


enter_pm:


    cli

    lgdt [gdt_descriptor]


    mov eax,cr0

    or eax,1

    mov cr0,eax



    jmp 0x08:protected

BOOT_DRIVE:
    db 0

KERNEL_DAP:
    db 0x10
    db 0x00
    dw 32          ; kernel最大读取32扇区
    dw 0x0000
    dw 0x1000
    dq 5           ; kernel从磁盘第5个LBA扇区开始

disk_error:

    mov si,error


.err:

    lodsb

    cmp al,0
    je $

    mov ah,0x0e

    int 0x10

    jmp .err

print_char:
    mov ah,0x0e
    int 0x10
    ret





; =====================
; GDT
; =====================


gdt_start:


dq 0



; code

dw 0xffff

dw 0

db 0

db 10011010b

db 11001111b

db 0



; data


dw 0xffff

dw 0

db 0

db 10010010b

db 11001111b

db 0



gdt_end:



gdt_descriptor:

dw gdt_end-gdt_start-1

dd gdt_start





; =====================
; Protected Mode
; =====================


[BITS 32]


protected:


    mov ax,0x10


    mov ds,ax
    mov es,ax
    mov ss,ax



    mov esp,0x90000



    ; kernel加载位置
    jmp 0x08:0x10000





msg:

db "Kernel Load OK!",0


error:

db "Disk Error",0

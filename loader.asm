[BITS 16]

org 0x8000


start:


    cli


; ------------------
; 加载Kernel
; ------------------


    mov ah,0x02

    mov al,10        ; kernel大小10扇区

    mov ch,0

    mov cl,6         ; 第6扇区开始


    mov dh,0

    mov dl,0x80



    mov bx,0x1000

    mov es,bx

    xor bx,bx



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


    lgdt [gdt_descriptor]


    mov eax,cr0

    or eax,1

    mov cr0,eax



    jmp 0x08:protected



disk_error:

    mov si,error


.err:

    lodsb

    cmp al,0
    je $

    mov ah,0x0e

    int 0x10

    jmp .err





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
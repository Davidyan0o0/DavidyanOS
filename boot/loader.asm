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

    mov ah,0x41
    mov bx,0x55AA
    mov dl,[BOOT_DRIVE]
    int 0x13
    jc .no_edd
    cmp bx,0xAA55
    jne .no_edd
    test cx,1
    jz .no_edd
    mov byte [EDD_AVAILABLE],1

.no_edd:

    ; 分块加载Kernel，避免BIOS单次读扇区数限制
    mov word [KERNEL_REMAINING],512

load_kernel:
    mov ax,[KERNEL_REMAINING]
    cmp ax,0
    je kernel_loaded

    cmp ax,64
    jbe .use_remaining
    mov ax,64

.use_remaining:
    mov [KERNEL_DAP_COUNT],ax

    cmp byte [EDD_AVAILABLE],0
    je load_kernel_chs

    mov si,KERNEL_DAP
    mov ah,0x42
    mov dl,[BOOT_DRIVE]
    int 0x13

    jc load_kernel_chs

    mov ax,[KERNEL_DAP_COUNT]
    sub [KERNEL_REMAINING],ax

    mov bx,ax
    shl bx,5
    add [KERNEL_DAP_SEGMENT],bx

    add [KERNEL_DAP_LBA],ax
    adc word [KERNEL_DAP_LBA+2],0

    jmp load_kernel

load_kernel_chs:
    call init_chs_geometry

.next_sector:
    mov ax,[KERNEL_REMAINING]
    cmp ax,0
    je kernel_loaded

    call read_kernel_sector_chs
    jc disk_error

    dec word [KERNEL_REMAINING]
    add word [KERNEL_DAP_SEGMENT],0x20
    inc word [KERNEL_DAP_LBA]
    jmp .next_sector

kernel_loaded:

    ; 打印
    mov si,msg


print:
    lodsb
    cmp al,0
    je enter_pm
    mov ah,0x0e
    int 0x10
    jmp print


; GDT
enter_pm:

    cli

    lgdt [gdt_descriptor]

    mov eax,cr0

    or eax,1

    mov cr0,eax

    jmp 0x08:protected

BOOT_DRIVE:
    db 0

EDD_AVAILABLE:
    db 0

CHS_INITIALIZED:
    db 0

CHS_SECTORS:
    dw 63

CHS_HEADS:
    dw 255

KERNEL_DAP:
    db 0x10
    db 0x00
KERNEL_DAP_COUNT:
    dw 64          ; 每次最多读取64扇区
KERNEL_DAP_OFFSET:
    dw 0x0000
KERNEL_DAP_SEGMENT:
    dw 0x1000
KERNEL_DAP_LBA:
    dq 5           ; kernel从磁盘第5个LBA扇区开始

KERNEL_REMAINING:
    dw 0

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

init_chs_geometry:
    cmp byte [CHS_INITIALIZED],0
    jne .done

    mov byte [CHS_INITIALIZED],1
    xor ax,ax
    mov dl,[BOOT_DRIVE]
    int 0x13

    mov ah,0x08
    mov dl,[BOOT_DRIVE]
    int 0x13
    jc .done

    mov al,cl
    and al,0x3F
    jz .done
    xor ah,ah
    mov [CHS_SECTORS],ax

    mov al,dh
    inc al
    xor ah,ah
    mov [CHS_HEADS],ax

.done:
    ret

read_kernel_sector_chs:
    mov ax,[KERNEL_DAP_SEGMENT]
    mov es,ax
    xor bx,bx

    mov ax,[KERNEL_DAP_LBA]
    xor dx,dx
    div word [CHS_SECTORS]
    mov cl,dl
    inc cl

    xor dx,dx
    div word [CHS_HEADS]
    mov dh,dl
    mov ch,al
    shl ah,6
    or cl,ah

    mov ah,0x02
    mov al,1
    mov dl,[BOOT_DRIVE]
    int 0x13
    ret




; GDT
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

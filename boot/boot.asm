[BITS 16]

org 0x7c00 ;RAM address

start:

    cli ; 关闭中断
    cld

    ;初始化寄存器
    xor ax,ax
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,0x7c00

    mov [BOOT_DRIVE], dl ; 保存启动盘编号

    mov cx,ax
    mov dx,ax

    sti

    mov al,'B'
    call print_char

    xor ax,ax
    mov dl,[BOOT_DRIVE]
    int 0x13

    mov si,DAP
    mov ah,0x42          ; 优先使用扩展LBA读取
    mov dl,[BOOT_DRIVE]
    int 0x13
    jnc check_loader

    xor ax,ax
    mov dl,[BOOT_DRIVE]
    int 0x13

chs_read:
    mov ah,0x02          ; 读取扇区(BIOS CHS)
    mov al,4             ; 读取4个扇区
    mov ch,0             ; 柱面号
    mov cl,2             ; 扇区号
    mov dh,0             ; 磁头号
    mov dl,[BOOT_DRIVE]
    mov bx,0x8000        ; 加载到RAM地址

    int 0x13             ; BIOS 磁盘中断

    jc disk_error

check_loader:
    cmp word [0x8000],0xDADA
    jne loader_signature_error

    mov al,'L'
    call print_char

    mov dl,[BOOT_DRIVE]
    jmp 0x0000:0x8002


disk_error:
    mov si,msg
    .print:
        lodsb
        cmp al,0
        je $
        mov ah,0x0e ; 设置 BIOS 视频服务功能号
        int 0x10    ; 显示中断
        jmp .print

loader_signature_error:
    mov si,sig_msg
    .print:
        lodsb
        cmp al,0
        je $
        mov ah,0x0e
        int 0x10
        jmp .print

print_char:
    mov ah,0x0e
    int 0x10
    ret

align 4

; Disk Address Packet
DAP:

    db 0x10        ; DAP大小
    db 0x00        ; 保留

    dw 4           ; 读取4个扇区

    dw 0x8000      ; offset
    dw 0x0000      ; segment

    dq 1

msg:
    db "Loader Load Error",0

sig_msg:
    db "Bad Loader",0

BOOT_DRIVE:
    db 0


times 510-($-$$) db 0

dw 0xaa55

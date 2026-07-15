[BITS 16]

org 0x7c00 ;RAM address

start:

    cli ; 关闭中断

    ;初始化寄存器
    xor ax,ax
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,0x7c00

    mov [BOOT_DRIVE], dl ; 保存启动盘编号

    mov cx,ax
    mov dx,ax

    ;磁盘功能
    ;mov ah,0x02         ; 读取扇区(BIOS 功能)
    ;mov al,4            ; 读取4个扇区(BIOS 参数)
    ;mov ch,0            ; 柱面号
    ;mov cl,2            ; 扇区号
    ;mov dh,0            ; 磁头号
    ;mov dl,[BOOT_DRIVE] ; 硬盘驱动器号
    ;mov dl,0x80
    ;mov bx,0x8000       ; 加载到RAM地址


    ; LBA读取
    mov si,DAP
    mov ah,0x42          ; 扩展读取
    mov dl,[BOOT_DRIVE]

    int 0x13             ; BIOS 磁盘中断

    jc disk_error

    mov dl,[BOOT_DRIVE]
    jmp 0x0000:0x8000


disk_error:
    mov si,msg
    .print:
        lodsb
        cmp al,0
        je $
        mov ah,0x0e ; 设置 BIOS 视频服务功能号
        int 0x10    ; 显示中断
        jmp .print

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

BOOT_DRIVE:
    db 0


times 510-($-$$) db 0

dw 0xaa55

[BITS 16]

org 0x7c00 ;RAM address

start:

    cli ; 关闭CPU硬件中断(软件中断不受cli影响)
    cld ; 清方向标志DF(地址自增还是自减)

    ;初始化段寄存器
    xor ax,ax
    mov ds,ax
    mov es,ax
    mov ss,ax      ; 栈地址
    mov sp,0x7c00  ; 栈顶

    mov [BOOT_DRIVE], dl ; 保存启动盘编号

    sti ;开启中断，部分真机BIOS磁盘服务依赖中断

    mov al,'B'
    call print_char

    xor ax,ax
    mov ds,ax
    mov es,ax

    xor ax,ax            ; BIOS磁盘复位(AH=0 复位磁盘)
    mov dl,[BOOT_DRIVE]  ; 恢复启动盘编号
    int 0x13             ; BIOS磁盘服务

    mov ah,0x41          ; 检测扩展LBA读取支持
    mov bx,0x55AA
    mov dl,[BOOT_DRIVE]
    int 0x13
    jc chs_reset
    cmp bx,0xAA55
    jne chs_reset
    test cx,1
    jz chs_reset

    mov si,DAP
    mov ah,0x42          ; 优先使用扩展LBA读取(功能号AH=42h)
    mov dl,[BOOT_DRIVE]
    int 0x13
    jnc check_loader

chs_reset:
    xor ax,ax
    mov dl,[BOOT_DRIVE]
    int 0x13

chs_read:
    xor ax,ax
    mov es,ax
    mov ah,0x02          ; 读取扇区(BIOS CHS)
    mov al,4             ; 读取4个扇区
    mov ch,0             ; 柱面号
    mov cl,2             ; 扇区号
    mov dh,0             ; 磁头号
    mov dl,[BOOT_DRIVE]
    mov bx,0x8000        ; 加载到RAM地址

    int 0x13             ; BIOS 磁盘中断

    mov [DISK_STATUS],ah
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
        je .status
        mov ah,0x0e ; 设置 BIOS 视频服务功能号
        int 0x10    ; 显示中断
        jmp .print

.status:
    mov al,' '
    call print_char
    mov al,[DISK_STATUS]
    call print_hex8
    jmp $

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

print_hex8:
    push ax
    shr al,4
    call print_hex_nibble
    pop ax
    and al,0x0F
    call print_hex_nibble
    ret

print_hex_nibble:
    cmp al,10
    jb .digit
    add al,'A'-10
    jmp print_char
.digit:
    add al,'0'
    jmp print_char

align 4

; Disk Address Packet
DAP:

    db 0x10        ; DAP大小(16字节)
    db 0x00        ; 保留

    dw 4           ; 读取4个扇区

    dw 0x8000      ; offset
    dw 0x0000      ; segment

    dq 1           ; LBA起始扇区(第2个扇区)

msg:
    db "Loader Load Error",0

sig_msg:
    db "Bad Loader",0

BOOT_DRIVE:
    db 0

DISK_STATUS:
    db 0


times 510-($-$$) db 0

dw 0xaa55

; boot.asm
[BITS 16]

org 0x7c00


start:

    cli

    xor ax, ax
    mov ds, ax
    mov es, ax


    ; 保存启动盘编号
    mov [BOOT_DRIVE], dl


    ; 读取第二阶段
    mov ah, 0x02      ; BIOS读扇区
    mov al, 4         ; 读取4个扇区

    mov ch, 0         ; 柱面
    mov cl, 2         ; 从第2扇区开始
    mov dh, 0         ; 磁头

    mov dl,[BOOT_DRIVE]


    ; 加载地址
    mov bx,0x8000


    int 0x13

    jc disk_error
 
    mov si,success_msg
    jmp print_ok

print_ok:
   lodsb
   cmp al,0
   je continue

   mov ah,0x0e
   int 0x10
   jmp print_ok
   
   
continue:
    ; 跳转Stage2
    jmp 0x0000:0x8000


disk_error:

    mov si,error_msg

.print:

    lodsb

    cmp al,0
    je $

    mov ah,0x0e
    int 0x10

    jmp .print



error_msg:
    db "Disk Error!",0

success_msg:
    db "Read OK!",0

BOOT_DRIVE:
    db 0



times 510-($-$$) db 0

dw 0xaa55
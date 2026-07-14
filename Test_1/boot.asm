[BITS 16]

org 0x7c00


start:

    cli

    xor ax,ax
    mov ds,ax
    mov es,ax


    ; 保存启动磁盘编号
    mov [BOOT_DRIVE],dl


    ; 显示启动信息
    mov si,boot_msg
    call print



    ; =====================
    ; 读取 Stage2
    ; =====================

    mov ah,0x02        ; BIOS读取扇区
    mov al,4           ; 读取4个扇区

    mov ch,0
    mov cl,2           ; 第2扇区

    mov dh,0

    mov dl,[BOOT_DRIVE]


    ; ES:BX = 0000:8000

    mov bx,0x8000


    int 0x13


    jc disk_error



    mov si,ok_msg
    call print



    ; 跳到Stage2

    jmp 0x0000:0x8000





; =====================
; BIOS打印
; =====================

print:

.next:

    lodsb

    cmp al,0
    je .done


    mov ah,0x0e
    int 0x10


    jmp .next


.done:

    ret





disk_error:

    mov si,error_msg
    call print

    cli
    hlt





boot_msg:

db "Booting...",13,10,0


ok_msg:

db "Load stage2 OK!",13,10,0


error_msg:

db "Disk Error!",0



BOOT_DRIVE:

db 0



times 510-($-$$) db 0


dw 0xaa55
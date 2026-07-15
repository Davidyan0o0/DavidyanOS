[BITS 32]

org 0x10000



start:


    mov edi,0xb8000

    mov edi,0xb8000+1920

    mov esi,msg



print:


    lodsb


    test al,al

    jz halt



    mov [edi],al

    mov byte [edi+1],0x0f


    add edi,2


    jmp print



halt:


    cli

    hlt

    jmp halt





msg:

db "Hello My Kernel!",0
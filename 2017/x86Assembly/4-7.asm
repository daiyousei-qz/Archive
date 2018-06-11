data segment
ends

stack segment
    dw   128  dup(0)
ends

code segment
start:
    mov ax, data
    mov ds, ax
    mov es, ax
                                         
    xor ah, ah    ; clear ah
    mov al, 8     ; load immediate number
    
    mov dx, dx    ; take copy
    shl dx, 1     ; dx <- x*2
    shl ax, 2     ; ax <- x*4
    add ax, dx    ; ax <- x*2 + x*4 = x*6
    
    
    mov ax, 4c00h ; exit to operating system.
    int 21h    
ends

end start ; set entry point and stop the assembler.

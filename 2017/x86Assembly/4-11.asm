data segment
    error db "Input error$"
ends

stack segment
    dw   256  dup(0)
ends

code segment
start:
; set segment registers:
                mov ax, data
                mov ds, ax
                mov es, ax
                
                jmp begin 
recovery:       mov ax, 10
                call put
                mov ax, 13
                call put
                mov ah, 09h
                int 21h
                mov ax, 10
                call put
                mov ax, 13
                call put
                
begin:          ; digit        
                call get
                call testdigit
                cmp al, 0ffh
                jz recovery
                mov bx, ax
                
                ; +
                call get
                cmp al, '+'
                jnz recovery
                
                ; digit
                call get
                call testdigit
                cmp al, 0ffh
                jz recovery
                
                ; do calculation
                add ax, bx
                mov dx, ax
                
                ; print '='
                mov ax, '='
                call put
                             
                ; put int
                mov ax, dx
                call putint  
                
                mov ax, 4c00h ; exit to operating system.
                int 21h    
ends
    
put             proc
                push dx
                push ax
                
                mov dx, ax
                mov ah, 2h
                int 21h
                
                pop ax
                pop dx
                ret
put             endp 

; function putint(int) -> int
; write an integer
putint          proc
                push dx
                push cx
                push bx
                
                cmp ax, 10
                jl putint_dig
                
                ; ax <- ax/10
                ; dx <- ax%10
                xor dx, dx
                mov bx, 10
                div bx
                call putint
                mov cx, ax
                mov ax, dx
                
putint_dig:     add ax, '0'
                call put
                
putint_ret:     pop bx
                pop cx
                pop dx
                ret
putint          endp

get             proc
                mov ah, 1h
                int 21h
                
                xor ah, ah
                ret
get             endp
    
; function testdigit(word ascii) -> word result
; returns identity if it's an ascii code for a digit, -1 otherwise
testdigit       proc
                ; test digit
                cmp ax, 48
                jl testdigit_err
                cmp ax, 57
                jg testdigit_err
                ; finalize
                sub ax, 48       
                jmp testdigit_scc
                
testdigit_err:  mov ax, 0ffffH
testdigit_scc:  ret
testdigit       endp


end start ; set entry point and stop the assembler.
                           
data segment
ends

stack segment
    dw   128  dup(0)
ends

code segment  
macro shift X, Y
    mov ax, X
    mov cl, Y
    shl ax, cl
    mov X, ax
endm

; a correct version with no side effect should be:
; but this version does not compile on emu8086
;macro shift X, Y 
;      IFIDN X,ax
;          shl ax, Y
;      ELSE
;          push ax
;          push cx
;          mov ax, X
;          mov cl, Y
;          shl ax, cl
;          mov Y, ax
;          pop cx
;          pop ax
;      ENDIF
;endm shift

start:
      mov ax, data
      mov ds, ax
      mov es, ax
              
      ; test code
      mov ax, 2
      shift ax, 2 
            
      mov ax, 4c00h ; exit to operating system.
      int 21h    
ends

end start ; set entry point and stop the assembler.

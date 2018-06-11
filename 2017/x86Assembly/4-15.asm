data segment        
    buffer dw 24, 60, 78, 11, 10, 58, 13, 66, 1, 15, 21, 92, 34, 40, 29, 35, 37, 37, 87, 31, 68, 44, 18, 10, 37, 19, 84, 92, 91, 78, 44, 63, 15, 50, 70, 33, 3, 81, 13, 74, 1, 18, 64, 9, 21, 61, 75, 76, 55, 4, 7, 80, 17, 42, 91, 79, 41, 68, 74, 90, 73, 20, 75, 22, 71, 57, 77, 84, 69, 96, 31, 55, 24, 12, 30, 49, 80, 9, 89, 75, 44, 8, 37, 87, 71, 63, 46, 72, 59, 90, 92, 34, 7, 22, 0, 81, 43, 41, 38, 63
ends

stack segment
    dw   256  dup(0)
ends

; an implementation of merge sort
code segment
start:      ; set segment registers:
            mov ax, data
            mov ds, ax
            mov es, ax
        
            lea ax, [buffer]
            lea bx, [buffer+200]
            call sort                   
            
            ; exit to operating system.
            mov ax, 4c00h
            int 21h  
            
; function qsort(word-ptr begin, word-ptr end) -> unit  
; apply quick sort to data in [begin, end)
sort       proc
           ; backup call site 
           push di
           push cx
           push bx
           push ax 
 
           ; range check
           mov cx, bx
           sub cx, ax
           shr cx, 1
           
           ; test special cases
           cmp cx, 2
           ; zero or one element
           jl return_s
  
           ; backup ax as function partition would return
           mov cx, ax 
            
           ; perform partition
           call partition
           
           ; sort tail
           add ax, 2  
           call sort
           
           ; sort head
           mov bx, ax
           sub bx, 2
           mov ax, cx
           call sort
           
return_s:  ; recover call site
           pop ax
           pop bx
           pop cx
           pop di  
           ret
sort       endp  

; function partition(word-ptr begin, word-ptr end) -> word-ptr new_position  
; do partition in [begin, end), first element is assumed as the pivot
partition  proc
           ; backup call site
           push di
           push dx
           push cx
           push bx
            
           ; registers table
           ; ax: left
           ; bx: right
           ; cx: counter
           ; dx: pivot
           
           ; load pivot
           mov di, ax      
           mov dx, [di]
           
           ; prepare counter
           mov cx, bx
           sub cx, ax
           shr cx, 1
           dec cx
           
           ; prepare right
           sub bx, 2
           
stage1:    mov di, ax   ; while(*left > pivot)
           cmp [di], dx 
           jle stage2
            
           add ax, 2    ; ++left
           dec cx       ; --counter
           jmp stage1
                  
stage2:    cmp [bx], dx ; while(*right < pivot)
           jge stage3
           
           sub bx, 2    ; --right
           dec cx       ; --counter
           jmp stage2
           
stage3:    cmp cx, 0    ; if(counter != 0)
           jz return_p  ; return on negative branch
                                             
           push dx      ; backup context
           push cx 
           
           mov di, ax   ; swap(*left, *right)
           mov cx, [di]
           mov dx, [bx]
           mov [di], dx
           mov [bx], cx
           
           add ax, 2    ; ++left
           
           mov di, ax   ; swap(*left, right)
           mov cx, [di]
           mov dx, [bx]
           mov [di], dx
           mov [bx], cx
           
           pop cx       ; recover context
           pop dx         
           
           dec cx       ; --counter
           
           jmp stage1   ; continue            
           
return_p:  ; NOTE ax that stores left pointer is the return value
           ; recover call site
           pop bx
           pop cx
           pop dx
           pop di 
           ret
partition  endp

ends

end start ; set entry point and stop the assembler.
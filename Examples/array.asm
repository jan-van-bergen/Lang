; Generated by Lang compiler

extern GetCommandLineA
extern ExitProcess

section .code
global _start
_start:
    call GetCommandLineA
    mov rcx, rax
    call main
    mov ecx, eax
    call ExitProcess

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 272 ; reserve stack space for 3 locals
    
    ; let arr: u32[64];
    mov QWORD [rbp + -272], 0 ; zero initialize 'arr'
    
    ; let i: i32;
    mov DWORD [rbp + -16], 0 ; zero initialize 'i'
    
    ; while (i < 64)
    L_loop0:
    movsx rbx, DWORD [rbp + -16]
    mov r10, 64
    cmp rbx, r10
    setl bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit0
        ; *(arr + i * sizeof(u32)) = i * i
        movsx rbx, DWORD [rbp + -16]
        mov r10, 4 ; sizeof 'u32'
        imul rbx, r10
        lea r10, QWORD [rbp + -272] ; get address of 'arr'
        add r10, rbx
        movsx rbx, DWORD [rbp + -16]
        movsx r11, DWORD [rbp + -16]
        imul rbx, r11
        mov DWORD [r10], ebx
        
        ; i++
        lea rbx, QWORD [rbp + -16] ; get address of 'i'
        mov r10, rbx
        movsx rbx, DWORD [rbx]
        mov r11, rbx
        inc r11
        mov DWORD [r10], r11d
        
    jmp L_loop0
    L_exit0:
    
    ; let sum: i32;
    mov DWORD [rbp + -12], 0 ; zero initialize 'sum'
    
    ; i = 0
    lea rbx, QWORD [rbp + -16] ; get address of 'i'
    mov r10, 0
    mov DWORD [rbx], r10d
    
    ; while (i < 64)
    L_loop1:
    movsx rbx, DWORD [rbp + -16]
    mov r10, 64
    cmp rbx, r10
    setl bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit1
        ; sum = sum + *(arr + i * sizeof(u32))
        movsx rbx, DWORD [rbp + -16]
        mov r10, 4 ; sizeof 'u32'
        imul rbx, r10
        lea r10, QWORD [rbp + -272] ; get address of 'arr'
        add r10, rbx
        mov r10d, DWORD [r10]
        movsx rbx, DWORD [rbp + -12]
        add rbx, r10
        lea r10, QWORD [rbp + -12] ; get address of 'sum'
        mov DWORD [r10], ebx
        
        ; i++
        lea rbx, QWORD [rbp + -16] ; get address of 'i'
        mov r10, rbx
        movsx rbx, DWORD [rbx]
        mov r11, rbx
        inc r11
        mov DWORD [r10], r11d
        
    jmp L_loop1
    L_exit1:
    
    ; return sum
    movsx rbx, DWORD [rbp + -12]
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


section .data

; Generated by Lang compiler

GLOBAL main

SECTION .code
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 32 ; reserve stack space for 5 locals
    lea rbx, QWORD [rbp + -32] ; get address of 'a'
    mov r10, 55
    mov DWORD [rbx], r10d
    
    lea rbx, QWORD [rbp + -32] ; get address of 'a'
    mov r10, rbx
    movsx rbx, DWORD [rbx]
    mov r11, rbx
    inc r11
    mov DWORD [r10], r11d
    lea r10, QWORD [rbp + -28] ; get address of 'b'
    mov DWORD [r10], ebx
    
    lea rbx, QWORD [rbp + -32] ; get address of 'a'
    mov r10, rbx
    movsx rbx, DWORD [rbx]
    inc rbx
    mov DWORD [r10], ebx
    lea r10, QWORD [rbp + -24] ; get address of 'c'
    mov DWORD [r10], ebx
    
    lea rbx, QWORD [rbp + -32] ; get address of 'a'
    mov r10, rbx
    movsx rbx, DWORD [rbx]
    dec rbx
    mov DWORD [r10], ebx
    lea r10, QWORD [rbp + -20] ; get address of 'd'
    mov DWORD [r10], ebx
    
    lea rbx, QWORD [rbp + -32] ; get address of 'a'
    mov r10, rbx
    movsx rbx, DWORD [rbx]
    mov r11, rbx
    dec r11
    mov DWORD [r10], r11d
    lea r10, QWORD [rbp + -16] ; get address of 'e'
    mov DWORD [r10], ebx
    
    movsx rbx, DWORD [rbp + -16]
    movsx r10, DWORD [rbp + -20]
    sub rbx, r10
    movsx r10, DWORD [rbp + -24]
    add rbx, r10
    movsx r10, DWORD [rbp + -28]
    sub rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

; Generated by Lang compiler

GLOBAL main

SECTION .code
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for locals
    mov rbx, 48
    mov DWORD [rbp + -4], ebx; initialize a
    mov rbx, 3
    mov DWORD [rbp + -8], ebx; initialize b
    movsx rbx, DWORD [rbp + -4] ; get value of a
    movsx r10, DWORD [rbp + -8] ; get value of b
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rax
    mov DWORD [rbp + -12], ebx; initialize c
    movsx rbx, DWORD [rbp + -12] ; get value of c
    mov r10, 16
    cmp rbx, r10
    je L0
    mov rbx, 1
    jmp L1
    L0:
    mov rbx, 0
    L1:
    cmp rbx, 0
    je L_exit2
        mov rbx, 0
        mov rax, rbx ; return via rax
        jmp L_function_main_exit
    L_exit2:
    lea rbx, QWORD [rbp + -8] ; get address of b
    mov r10, 4
    mov DWORD [rbx], r10d
    movsx rbx, DWORD [rbp + -12] ; get value of c
    movsx r10, DWORD [rbp + -8] ; get value of b
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rax
    lea r10, QWORD [rbp + -12] ; get address of c
    mov DWORD [r10], ebx
    movsx rbx, DWORD [rbp + -12] ; get value of c
    mov r10, 4
    cmp rbx, r10
    je L3
    mov rbx, 1
    jmp L4
    L3:
    mov rbx, 0
    L4:
    cmp rbx, 0
    je L_exit5
        mov rbx, 0
        mov rax, rbx ; return via rax
        jmp L_function_main_exit
    L_exit5:
    movsx rbx, DWORD [rbp + -12] ; get value of c
    movsx r10, DWORD [rbp + -12] ; get value of c
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rax
    lea r10, QWORD [rbp + -12] ; get address of c
    mov DWORD [r10], ebx
    movsx rbx, DWORD [rbp + -12] ; get value of c
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    
SECTION .data

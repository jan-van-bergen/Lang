; Generated by Lang compiler

GLOBAL main

SECTION .code
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for locals
    mov rbx, 321
    mov r10, 43
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rdx
    mov QWORD [rbp + -8], rbx; initialize a
    mov rbx, 3
    mov r10, 5
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rdx
    mov QWORD [rbp + -16], rbx; initialize b
    mov rbx, QWORD [rbp + -8] ; get value of a
    mov r10, QWORD [rbp + -16] ; get value of b
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rdx
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    
SECTION .data

; Generated by Lang compiler

GLOBAL main

SECTION .code
abc:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    sub rsp, 32 ; reserve stack space for locals
    lea rbx, QWORD [rbp + 16] ; addrof a
    mov QWORD [rbp + -32], rbx; initialize ptr_a
    lea rbx, QWORD [rbp + 24] ; addrof b
    mov QWORD [rbp + -24], rbx; initialize ptr_b
    lea rbx, QWORD [rbp + -32] ; addrof ptr_a
    mov QWORD [rbp + -16], rbx; initialize ptr_ptr
    mov rbx, QWORD [rbp + -16] ; get value of ptr_ptr
    mov r10, QWORD [rbp + -24] ; get value of ptr_b
    mov QWORD [rbx], r10
    mov rbx, QWORD [rbp + -16] ; get value of ptr_ptr
    mov rbx, QWORD [rbx]
    movsx rbx, DWORD [rbx]
    mov rax, rbx ; return via rax
    jmp L_function_abc_exit
    xor rax, rax ; Default return value 0
    L_function_abc_exit:
    mov rsp, rbp
    pop rbp
    ret
    
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 0 ; reserve stack space for locals
    sub rsp, 32 ; reserve space for call arguments
    mov rbx, 1
    mov rcx, rbx ; arg 0
    mov rbx, 2
    mov rdx, rbx ; arg 1
    call abc
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    
SECTION .data

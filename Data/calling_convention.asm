; Generated by Lang compiler

GLOBAL main

SECTION .code
calling_convention:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    mov DWORD [rbp + 32], r8d ; push arg 2 
    mov DWORD [rbp + 40], r9d ; push arg 3 
    movsx rbx, DWORD [rbp + 16] ; get value of 'rcx'
    movsx r10, DWORD [rbp + 24] ; get value of 'rdx'
    add rbx, r10
    movsx r10, DWORD [rbp + 32] ; get value of 'r8'
    add rbx, r10
    movsx r10, DWORD [rbp + 40] ; get value of 'r9'
    add rbx, r10
    movsx r10, DWORD [rbp + 48] ; get value of 'stack0'
    add rbx, r10
    movsx r10, DWORD [rbp + 52] ; get value of 'stack1'
    add rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_calling_convention_exit
    xor rax, rax ; Default return value 0
    L_function_calling_convention_exit:
    mov rsp, rbp
    pop rbp
    ret
    
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 48 ; reserve shadow space and 6 arguments
    mov rbx, 1
    mov rcx, rbx ; arg 0
    mov rbx, 2
    mov rdx, rbx ; arg 1
    mov rbx, 3
    mov r8, rbx ; arg 2
    mov rbx, 4
    mov r9, rbx ; arg 3
    mov rbx, 5
    mov QWORD [rsp + 32], rbx ; arg 4
    mov rbx, 6
    mov QWORD [rsp + 36], rbx ; arg 5
    call calling_convention
    add rsp, 48 ; pop arguments
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    
SECTION .data

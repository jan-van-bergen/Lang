; Generated by Lang compiler

GLOBAL main

SECTION .code
bla:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    sub rsp, 0 ; reserve stack space for locals
    mov rbx, QWORD [rbp + 16] ; get value of ptr
    mov r10, 4321
    mov DWORD [rbx], r10d
    xor rax, rax ; Default return value 0
    L_function_bla_exit:
    mov rsp, rbp
    pop rbp
    ret
    
deref:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    sub rsp, 16 ; reserve stack space for locals
    mov DWORD [rbp + -4], 0; zero initialize local
    mov rbx, QWORD [rbp + 16] ; get value of ptr
    mov r10, 21
    mov DWORD [rbx], r10d
    lea rbx, QWORD [rbp + -4] ; get address of local
    mov r10, 1234
    mov DWORD [rbx], r10d
    sub rsp, 32 ; reserve space for call arguments
    lea rbx, QWORD [rbp + -4] ; addrof local
    mov rcx, rbx ; arg 0
    call bla
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    xor rax, rax ; Default return value 0
    L_function_deref_exit:
    mov rsp, rbp
    pop rbp
    ret
    
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for locals
    mov rbx, 42
    mov DWORD [rbp + -4], ebx; initialize a
    sub rsp, 32 ; reserve space for call arguments
    lea rbx, QWORD [rbp + -4] ; addrof a
    mov rcx, rbx ; arg 0
    call deref
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    movsx rbx, DWORD [rbp + -4] ; get value of a
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    
SECTION .data

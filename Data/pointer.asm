; Generated by Lang compiler

GLOBAL main

SECTION .code
bla:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    mov rbx, QWORD [rbp + 16]
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
    sub rsp, 16 ; reserve stack space for 1 locals
    mov DWORD [rbp + -16], 0 ; zero initialize 'local'
    
    mov rbx, QWORD [rbp + 16]
    mov r10, 21
    mov DWORD [rbx], r10d
    
    lea rbx, QWORD [rbp + -16] ; get address of 'local'
    mov r10, 1234
    mov DWORD [rbx], r10d
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    lea rbx, QWORD [rbp + -16] ; get address of 'local'
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
    sub rsp, 32 ; reserve stack space for 3 locals
    mov rbx, 42
    lea r10, QWORD [rbp + -32] ; get address of 'a'
    mov DWORD [r10], ebx
    
    mov QWORD [rbp + -24], 0 ; zero initialize 'p'
    
    lea rbx, QWORD [rbp + -32] ; get address of 'a'
    lea r10, QWORD [rbp + -24] ; get address of 'p'
    mov QWORD [r10], rbx
    
    mov QWORD [rbp + -16], 0 ; zero initialize 'p2'
    
    lea rbx, QWORD [rbp + -16] ; get address of 'p2'
    mov r10, QWORD [rbp + -24]
    mov QWORD [rbx], r10
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, QWORD [rbp + -16]
    mov rcx, rbx ; arg 0
    call deref
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    movsx rbx, DWORD [rbp + -32]
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

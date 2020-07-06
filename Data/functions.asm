; Generated by Lang compiler

GLOBAL main

SECTION .code
one:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    movsx rbx, DWORD [rbp + 16]
    mov r10, 1
    add rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_one_exit
    
    xor rax, rax ; Default return value 0
    L_function_one_exit:
    mov rsp, rbp
    pop rbp
    ret
    

two:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    sub rsp, 16 ; reserve stack space for 1 locals
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + 16]
    mov rcx, rbx ; arg 0
    call one
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -16] ; get address of 'local'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + 24]
    mov rcx, rbx ; arg 0
    call one
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    movsx r10, DWORD [rbp + -16]
    add rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_two_exit
    
    xor rax, rax ; Default return value 0
    L_function_two_exit:
    mov rsp, rbp
    pop rbp
    ret
    

recursive:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    movsx rbx, DWORD [rbp + 16]
    mov r10, 0
    cmp rbx, r10
    je L0
    mov rbx, 0
    jmp L1
    L0:
    mov rbx, 1
    L1:
    cmp rbx, 0
    je L_exit2
        movsx rbx, DWORD [rbp + 24]
        mov rax, rbx ; return via rax
        jmp L_function_recursive_exit
        
    L_exit2:
    
    movsx rbx, DWORD [rbp + 24]
    mov r10, 2
    add rbx, r10
    lea r10, QWORD [rbp + 24] ; get address of 'b'
    mov DWORD [r10], ebx
    
    movsx rbx, DWORD [rbp + 16]
    mov r10, 1
    sub rbx, r10
    lea r10, QWORD [rbp + 16] ; get address of 'a'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 2 arguments
    movsx rbx, DWORD [rbp + 16]
    mov rcx, rbx ; arg 0
    movsx rbx, DWORD [rbp + 24]
    mov rdx, rbx ; arg 1
    call recursive
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    jmp L_function_recursive_exit
    
    xor rax, rax ; Default return value 0
    L_function_recursive_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 2 locals
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, 1
    mov rcx, rbx ; arg 0
    mov rbx, 2
    mov rdx, rbx ; arg 1
    call two
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -16] ; get address of 'bla'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + -16]
    mov rcx, rbx ; arg 0
    call one
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -12] ; get address of 'tmp'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 2 arguments
    push rcx ; preserve
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + -16]
    mov rcx, rbx ; arg 0
    call one
    add rsp, 32 ; pop arguments
    pop rcx ; restore
    mov rbx, rax ; get return value
    mov rcx, rbx ; arg 0
    mov rbx, 0
    mov rdx, rbx ; arg 1
    call recursive
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

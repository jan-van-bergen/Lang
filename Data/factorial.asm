; Generated by Lang compiler

GLOBAL main

SECTION .code
factorial_recursive:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
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
        mov rbx, 1
        mov rax, rbx ; return via rax
        jmp L_function_factorial_recursive_exit
        
    L_exit2:
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + 16]
    mov r10, 1
    sub rbx, r10
    mov rcx, rbx ; arg 0
    call factorial_recursive
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    movsx r10, DWORD [rbp + 16]
    imul r10, rbx
    mov rax, r10 ; return via rax
    jmp L_function_factorial_recursive_exit
    
    xor rax, rax ; Default return value 0
    L_function_factorial_recursive_exit:
    mov rsp, rbp
    pop rbp
    ret
    

factorial_loop:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    sub rsp, 16 ; reserve stack space for 1 locals
    lea rbx, QWORD [rbp + -16] ; get address of 'result'
    mov r10, 1
    mov DWORD [rbx], r10d
    
    L_loop3:
    movsx rbx, DWORD [rbp + 16]
    mov r10, 0
    cmp rbx, r10
    jg L4
    mov rbx, 0
    jmp L5
    L4:
    mov rbx, 1
    L5:
    cmp rbx, 0
    je L_exit3
        movsx rbx, DWORD [rbp + -16]
        movsx r10, DWORD [rbp + 16]
        imul rbx, r10
        lea r10, QWORD [rbp + -16] ; get address of 'result'
        mov DWORD [r10], ebx
        
        movsx rbx, DWORD [rbp + 16]
        mov r10, 1
        sub rbx, r10
        lea r10, QWORD [rbp + 16] ; get address of 'n'
        mov DWORD [r10], ebx
        
    jmp L_loop3
    L_exit3:
    
    movsx rbx, DWORD [rbp + -16]
    mov rax, rbx ; return via rax
    jmp L_function_factorial_loop_exit
    
    xor rax, rax ; Default return value 0
    L_function_factorial_loop_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 3 locals
    lea rbx, QWORD [rbp + -16] ; get address of 'arg'
    mov r10, 5
    mov DWORD [rbx], r10d
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + -16]
    mov rcx, rbx ; arg 0
    call factorial_recursive
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -12] ; get address of 'a'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + -16]
    mov rcx, rbx ; arg 0
    call factorial_loop
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -8] ; get address of 'b'
    mov DWORD [r10], ebx
    
    movsx rbx, DWORD [rbp + -12]
    movsx r10, DWORD [rbp + -8]
    cmp rbx, r10
    je L6
    mov rbx, 0
    jmp L7
    L6:
    mov rbx, 1
    L7:
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

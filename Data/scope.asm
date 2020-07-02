; Generated by Lang compiler

GLOBAL main

SECTION .code
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 32 ; reserve stack space for 5 locals
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, 1
    mov rcx, rbx ; arg 0
    mov rbx, 5
    mov rdx, rbx ; arg 1
    call test
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -32] ; get address of 'a'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, 0
    mov rcx, rbx ; arg 0
    mov rbx, 5
    mov rdx, rbx ; arg 1
    call test
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -28] ; get address of 'b'
    mov DWORD [r10], ebx
    
    movsx rbx, DWORD [rbp + -32]
    mov r10, 5
    cmp rbx, r10
    je L0
    mov rbx, 1
    jmp L1
    L0:
    mov rbx, 0
    L1:
    movsx r10, DWORD [rbp + -28]
    mov r11, 3
    cmp r10, r11
    je L2
    mov r10, 1
    jmp L3
    L2:
    mov r10, 0
    L3:
    test rbx, rbx
    jne L_lor_true_4
    test r10, r10
    jne L_lor_true_4
    mov rbx, 0
    jmp L_lor_exit_4
    L_lor_true_4:
    mov rbx, 1
    L_lor_exit_4:
    cmp rbx, 0
    je L_exit5
        mov rbx, -1
        mov rax, rbx ; return via rax
        jmp L_function_main_exit
        
    L_exit5:
    
    mov rbx, 3
    lea r10, QWORD [rbp + -24] ; get address of 'common_name'
    mov DWORD [r10], ebx
    
    mov rbx, 0
    lea r10, QWORD [rbp + -20] ; get address of 'outter'
    mov DWORD [r10], ebx
    
    mov rbx, 1
    cmp rbx, 0
    je L_exit6
        mov DWORD [rbp + -16], 0 ; zero initialize 'common_name'
        
        lea rbx, QWORD [rbp + -16] ; get address of 'common_name'
        mov r10, 5
        mov DWORD [rbx], r10d
        
        lea rbx, QWORD [rbp + -20] ; get address of 'outter'
        movsx r10, DWORD [rbp + -16]
        mov DWORD [rbx], r10d
        
    L_exit6:
    
    movsx rbx, DWORD [rbp + -20]
    mov r10, 5
    cmp rbx, r10
    je L7
    mov rbx, 1
    jmp L8
    L7:
    mov rbx, 0
    L8:
    cmp rbx, 0
    je L_exit9
        mov rbx, -1
        mov rax, rbx ; return via rax
        jmp L_function_main_exit
        
    L_exit9:
    
    movsx rbx, DWORD [rbp + -24]
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    

test:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov BYTE [rbp + 16], cl ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    sub rsp, 16 ; reserve stack space for 1 locals
    movzx rbx, BYTE [rbp + 16]
    cmp rbx, 0
    je L_else10
        movsx rbx, DWORD [rbp + 24]
        mov rax, rbx ; return via rax
        jmp L_function_test_exit
        
    jmp L_exit10
    L_else10:
        mov rbx, 3
        lea r10, QWORD [rbp + -16] ; get address of 'arg'
        mov DWORD [r10], ebx
        
        movsx rbx, DWORD [rbp + -16]
        mov rax, rbx ; return via rax
        jmp L_function_test_exit
        
    L_exit10:
    
    xor rax, rax ; Default return value 0
    L_function_test_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

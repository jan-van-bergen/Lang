; Generated by Lang compiler

GLOBAL main

SECTION .code
EXTERN ExitProcess

fail:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, -1
    mov rcx, rbx ; arg 1
    call ExitProcess
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    xor rax, rax ; Default return value 0
    L_function_fail_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 1 locals
    mov rbx, 0
    test rbx, rbx
    je L_land_false_0
    push rbx ; preserve
    sub rsp, 32 ; reserve shadow space and 0 arguments
    call fail
    add rsp, 32 ; pop arguments
    pop rbx ; restore
    mov r10, rax ; get return value
    test r10, r10
    je L_land_false_0
    mov rbx, 1
    jmp L_land_exit_0
    L_land_false_0:
    mov rbx, 0
    L_land_exit_0:
    lea r10, QWORD [rbp + -16] ; get address of 'stat'
    mov BYTE [r10], bl
    
    mov rbx, 1
    test rbx, rbx
    jne L_lor_true_1
    push rbx ; preserve
    sub rsp, 32 ; reserve shadow space and 0 arguments
    call fail
    add rsp, 32 ; pop arguments
    pop rbx ; restore
    mov r10, rax ; get return value
    test r10, r10
    jne L_lor_true_1
    mov rbx, 0
    jmp L_lor_exit_1
    L_lor_true_1:
    mov rbx, 1
    L_lor_exit_1:
    cmp rbx, 0
    je L_exit2
    L_exit2:
    
    mov rbx, 0
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

; Generated by Lang compiler

GLOBAL main

SECTION .code
EXTERN ExitProcess

assert:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov BYTE [rbp + 16], cl ; push arg 0 
    movzx rbx, BYTE [rbp + 16]
    test rbx, rbx
    jne L_lnot_false_0
    mov rbx, 1
    jmp L_lnot_exit_0
    L_lnot_false_0:
    mov rbx, 0
    L_lnot_exit_0:
    cmp rbx, 0
    je L_exit1
        sub rsp, 32 ; reserve shadow space and 1 arguments
        mov rbx, 1
        mov rcx, rbx ; arg 1
        call ExitProcess
        add rsp, 32 ; pop arguments
        mov rbx, rax ; get return value
        
    L_exit1:
    
    xor rax, rax ; Default return value 0
    L_function_assert_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 1 locals
    mov BYTE [rbp + -16], 0 ; zero initialize 'a'
    
    mov rbx, 1
    test rbx, rbx
    jne L_lor_true_2
    mov r10, 0
    test r10, r10
    jne L_lor_true_2
    mov rbx, 0
    jmp L_lor_exit_2
    L_lor_true_2:
    mov rbx, 1
    L_lor_exit_2:
    lea r10, QWORD [rbp + -16] ; get address of 'a'
    mov BYTE [r10], bl
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movzx rbx, BYTE [rbp + -16]
    mov r10, 1
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    mov rbx, 0
    test rbx, rbx
    je L_land_false_3
    mov r10, 1
    test r10, r10
    je L_land_false_3
    mov rbx, 1
    jmp L_land_exit_3
    L_land_false_3:
    mov rbx, 0
    L_land_exit_3:
    lea r10, QWORD [rbp + -16] ; get address of 'a'
    mov BYTE [r10], bl
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movzx rbx, BYTE [rbp + -16]
    mov r10, 0
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    movzx rbx, BYTE [rbp + -16]
    test rbx, rbx
    jne L_lor_true_4
    movzx r10, BYTE [rbp + -16]
    test r10, r10
    jne L_lnot_false_5
    mov r10, 1
    jmp L_lnot_exit_5
    L_lnot_false_5:
    mov r10, 0
    L_lnot_exit_5:
    test r10, r10
    jne L_lor_true_4
    mov rbx, 0
    jmp L_lor_exit_4
    L_lor_true_4:
    mov rbx, 1
    L_lor_exit_4:
    lea r10, QWORD [rbp + -16] ; get address of 'a'
    mov BYTE [r10], bl
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movzx rbx, BYTE [rbp + -16]
    mov r10, 1
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    mov rbx, 0
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

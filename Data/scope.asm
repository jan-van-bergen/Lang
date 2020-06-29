; Generated by Lang compiler

GLOBAL main

SECTION .code
test:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov BYTE [rbp + 16], cl ; push arg 0 
    mov QWORD [rbp + 17], rdx ; push arg 1 
    sub rsp, 16 ; reserve stack space for locals
    movzx rbx, BYTE [rbp + 16] ; get value of ret_arg
    cmp rbx, 0
    je L_else0
        mov rbx, QWORD [rbp + 17] ; get value of arg
        mov rax, rbx ; return via rax
        jmp L_function_test_exit
    jmp L_exit0
    L_else0:
        mov rbx, 3
        mov QWORD [rbp + -8], rbx; initialize arg
        mov rbx, QWORD [rbp + -8] ; get value of arg
        mov rax, rbx ; return via rax
        jmp L_function_test_exit
    L_exit0:
    xor rax, rax ; Default return value 0
    L_function_test_exit:
    mov rsp, rbp
    pop rbp
    ret
    
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 48 ; reserve stack space for locals
    sub rsp, 32 ; shadow space
    mov rbx, 1
    mov rcx, rbx ; arg 0
    mov rbx, 5
    mov rdx, rbx ; arg 1
    call test
    mov rbx, rax ; get return value
    mov QWORD [rbp + -8], rbx; initialize a
    sub rsp, 32 ; shadow space
    mov rbx, 0
    mov rcx, rbx ; arg 0
    mov rbx, 5
    mov rdx, rbx ; arg 1
    call test
    mov rbx, rax ; get return value
    mov QWORD [rbp + -16], rbx; initialize b
    mov rbx, QWORD [rbp + -8] ; get value of a
    mov r10, 5
    cmp rbx, r10
    je L1
    mov rbx, 1
    jmp L2
    L1:
    mov rbx, 0
    L2:
    mov r10, QWORD [rbp + -16] ; get value of b
    mov r11, 3
    cmp r10, r11
    je L3
    mov r10, 1
    jmp L4
    L3:
    mov r10, 0
    L4:
    test rbx, rbx
    jne L_lor_true_5
    test r10, r10
    jne L_lor_true_5
    mov rbx, 0
    jmp L_lor_exit_5
    L_lor_true_5:
    mov rbx, 1
    L_lor_exit_5:
    cmp rbx, 0
    je L_exit6
        mov rbx, -1
        mov rax, rbx ; return via rax
        jmp L_function_main_exit
    L_exit6:
    mov rbx, 3
    mov QWORD [rbp + -24], rbx; initialize common_name
    mov rbx, 0
    mov QWORD [rbp + -32], rbx; initialize outter
    mov rbx, 1
    cmp rbx, 0
    je L_exit7
        mov QWORD [rbp + -40], 0; zero initialize common_name
        lea rbx, QWORD [rbp + -40] ; get address of common_name
        mov r10, 5
        mov QWORD [rbx], r10
        lea rbx, QWORD [rbp + -32] ; get address of outter
        mov r10, QWORD [rbp + -40] ; get value of common_name
        mov QWORD [rbx], r10
    L_exit7:
    mov rbx, QWORD [rbp + -32] ; get value of outter
    mov r10, 5
    cmp rbx, r10
    je L8
    mov rbx, 1
    jmp L9
    L8:
    mov rbx, 0
    L9:
    cmp rbx, 0
    je L_exit10
        mov rbx, -1
        mov rax, rbx ; return via rax
        jmp L_function_main_exit
    L_exit10:
    mov rbx, QWORD [rbp + -24] ; get value of common_name
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    
SECTION .data

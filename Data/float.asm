; Generated by Lang compiler

GLOBAL main

SECTION .code
square_f:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    mov QWORD [rbp + 24], rdx ; push arg 1 
    mov rbx, QWORD [rbp + 16]
    movss xmm4, DWORD [rbx]
    mov rbx, QWORD [rbp + 16]
    movss xmm5, DWORD [rbx]
    mulss xmm4, xmm5
    mov rbx, QWORD [rbp + 24]
    movss DWORD [rbx], xmm4
    
    xor rax, rax ; Default return value 0
    L_function_square_f_exit:
    mov rsp, rbp
    pop rbp
    ret
    

square_d:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    mov QWORD [rbp + 24], rdx ; push arg 1 
    mov rbx, QWORD [rbp + 16]
    movsd xmm4, QWORD [rbx]
    mov rbx, QWORD [rbp + 16]
    movsd xmm5, QWORD [rbx]
    mulsd xmm4, xmm5
    mov rbx, QWORD [rbp + 24]
    movsd QWORD [rbx], xmm4
    
    xor rax, rax ; Default return value 0
    L_function_square_d_exit:
    mov rsp, rbp
    pop rbp
    ret
    

round_f:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    mov rbx, QWORD [rbp + 16]
    movss xmm4, DWORD [rbx]
    movss xmm5, DWORD [REL lit_flt_0]
    addss xmm4, xmm5
    cvttss2si rbx, xmm4
    mov rax, rbx ; return via rax
    jmp L_function_round_f_exit
    
    xor rax, rax ; Default return value 0
    L_function_round_f_exit:
    mov rsp, rbp
    pop rbp
    ret
    

round_d:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    mov rbx, QWORD [rbp + 16]
    movsd xmm4, QWORD [rbx]
    movsd xmm5, QWORD [REL lit_flt_1]
    addsd xmm4, xmm5
    cvttsd2si rbx, xmm4
    mov rax, rbx ; return via rax
    jmp L_function_round_d_exit
    
    xor rax, rax ; Default return value 0
    L_function_round_d_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 32 ; reserve stack space for 5 locals
    lea rbx, QWORD [rbp + -32] ; get address of 'a'
    movss xmm4, DWORD [REL lit_flt_2]
    movss DWORD [rbx], xmm4
    
    lea rbx, QWORD [rbp + -28] ; get address of 'b'
    movss xmm4, DWORD [REL lit_flt_3]
    movss DWORD [rbx], xmm4
    
    movss xmm5, DWORD [rbp + -32]
    movss xmm4, DWORD [REL lit_flt_4]
    mulss xmm5, xmm4
    movss xmm6, DWORD [rbp + -28]
    movss xmm4, DWORD [REL lit_flt_5]
    divss xmm6, xmm4
    addss xmm5, xmm6
    lea rbx, QWORD [rbp + -24] ; get address of 'f'
    movss DWORD [rbx], xmm5
    
    lea rbx, QWORD [rbp + -16] ; get address of 'd'
    movsd xmm4, QWORD [REL lit_flt_6]
    movsd QWORD [rbx], xmm4
    
    sub rsp, 32 ; reserve shadow space and 2 arguments
    lea rbx, QWORD [rbp + -24] ; get address of 'f'
    mov rcx, rbx ; arg 1
    lea rbx, QWORD [rbp + -24] ; get address of 'f'
    mov rdx, rbx ; arg 2
    call square_f
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    sub rsp, 32 ; reserve shadow space and 2 arguments
    lea rbx, QWORD [rbp + -16] ; get address of 'd'
    mov rcx, rbx ; arg 1
    lea rbx, QWORD [rbp + -16] ; get address of 'd'
    mov rdx, rbx ; arg 2
    call square_d
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    movss xmm5, DWORD [rbp + -24]
    cvtss2sd xmm5, xmm5
    movsd xmm6, QWORD [rbp + -16]
    addsd xmm6, xmm5
    lea rbx, QWORD [rbp + -8] ; get address of 'result'
    movsd QWORD [rbx], xmm6
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    lea rbx, QWORD [rbp + -8] ; get address of 'result'
    mov rcx, rbx ; arg 1
    call round_d
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
lit_flt_0 dq 3f000000h ; 0.500000f
lit_flt_1 dq 3fe0000000000000h ; 0.500000
lit_flt_2 dq 40a00000h ; 5.000000f
lit_flt_3 dq 40400000h ; 3.000000f
lit_flt_4 dq 40400000h ; 3.000000f
lit_flt_5 dq 40000000h ; 2.000000f
lit_flt_6 dq 4010000000000000h ; 4.000000

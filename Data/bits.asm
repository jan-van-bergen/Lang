; Generated by Lang compiler

GLOBAL main

SECTION .code
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 3 locals
    mov DWORD [rbp + -16], 0 ; zero initialize 'a'
    
    mov DWORD [rbp + -12], 0 ; zero initialize 'b'
    
    mov DWORD [rbp + -8], 0 ; zero initialize 'c'
    
    lea rbx, QWORD [rbp + -16] ; get address of 'a'
    mov r10, 4278190080
    mov DWORD [rbx], r10d
    
    lea rbx, QWORD [rbp + -12] ; get address of 'b'
    mov r10, 255
    mov DWORD [rbx], r10d
    
    mov ebx, DWORD [rbp + -16]
    mov r10d, DWORD [rbp + -12]
    or rbx, r10
    lea r10, QWORD [rbp + -8] ; get address of 'c'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov ebx, DWORD [rbp + -8]
    mov r10, 4278190335
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    mov ebx, DWORD [rbp + -16]
    not rbx
    mov r10d, DWORD [rbp + -12]
    and rbx, r10
    lea r10, QWORD [rbp + -8] ; get address of 'c'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov ebx, DWORD [rbp + -8]
    mov r10, 255
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    lea rbx, QWORD [rbp + -16] ; get address of 'a'
    mov r10, 16776960
    mov DWORD [rbx], r10d
    
    mov ebx, DWORD [rbp + -12]
    mov r10, 8
    mov rcx, r10
    shl rbx, cl
    lea r10, QWORD [rbp + -12] ; get address of 'b'
    mov DWORD [r10], ebx
    
    mov ebx, DWORD [rbp + -16]
    mov r10d, DWORD [rbp + -12]
    xor rbx, r10
    mov r10d, DWORD [rbp + -8]
    or rbx, r10
    lea r10, QWORD [rbp + -8] ; get address of 'c'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov ebx, DWORD [rbp + -8]
    mov r10, 16711935
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    mov ebx, DWORD [rbp + -8]
    mov r10, 16
    mov rcx, r10
    shr rbx, cl
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    

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
    


SECTION .data

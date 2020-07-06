; Generated by Lang compiler

GLOBAL main

SECTION .code
fun:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    sub rsp, 16 ; reserve stack space for 1 locals
    lea rbx, QWORD [rbp + -16] ; get address of 'i'
    movsx r10, DWORD [rbp + 16]
    mov DWORD [rbx], r10d
    
    movsx rbx, DWORD [rbp + -16]
    mov r10, 2
    add rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_fun_exit
    
    xor rax, rax ; Default return value 0
    L_function_fun_exit:
    mov rsp, rbp
    pop rbp
    ret
    

otherfun:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    movsx rbx, DWORD [rbp + 16]
    mov r10, 1
    add rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_otherfun_exit
    
    xor rax, rax ; Default return value 0
    L_function_otherfun_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 32 ; reserve shadow space and 1 arguments
    push rcx ; preserve
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, 2
    mov rcx, rbx ; arg 0
    call otherfun
    add rsp, 32 ; pop arguments
    pop rcx ; restore
    mov rbx, rax ; get return value
    push rbx ; preserve
    push rcx ; preserve
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov r10, 3
    mov rcx, r10 ; arg 0
    call otherfun
    add rsp, 32 ; pop arguments
    pop rcx ; restore
    pop rbx ; restore
    mov r10, rax ; get return value
    add rbx, r10
    mov rcx, rbx ; arg 0
    call fun
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

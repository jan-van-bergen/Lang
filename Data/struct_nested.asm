; Generated by Lang compiler

GLOBAL main

SECTION .code
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 48 ; reserve stack space for locals
    lea rdi, QWORD [rbp + -48] ; zero initialize test
    xor rax, rax
    mov ecx, 24
    rep stosb
    lea rbx, QWORD [rbp + -48] ; get address of test
    add rbx, 0 ; member offset 'a'
    mov r10, 3
    mov BYTE [rbx], r10b
    lea rbx, QWORD [rbp + -48] ; get address of test
    add rbx, 8 ; member offset 'n'
    add rbx, 0 ; member offset 'b'
    mov r10, 4
    mov DWORD [rbx], r10d
    lea rbx, QWORD [rbp + -48] ; get address of test
    add rbx, 8 ; member offset 'n'
    add rbx, 8 ; member offset 'c'
    mov r10, 5
    mov QWORD [rbx], r10
    lea rdi, QWORD [rbp + -24] ; zero initialize nest
    xor rax, rax
    mov ecx, 16
    rep stosb
    lea rbx, QWORD [rbp + -48] ; get address of test
    add rbx, 8 ; member offset 'n'
    lea r10, QWORD [rbp + -24] ; get address of nest
    mov rdi, r10
    mov rsi, rbx
    mov ecx, 16
    rep movsb
    lea rbx, QWORD [rbp + -24] ; get address of nest
    add rbx, 0 ; member offset 'b'
    movsx rbx, DWORD [rbx]
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    
SECTION .data

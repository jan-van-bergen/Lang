; Generated by Lang compiler

GLOBAL main

SECTION .code
make_structure:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    sub rsp, 0 ; reserve stack space for locals
    mov rbx, QWORD [rbp + 16] ; get value of s
    add rbx, 0 ; member offset 'a'
    mov r10, 1
    mov DWORD [rbx], r10d
    mov rbx, QWORD [rbp + 16] ; get value of s
    add rbx, 4 ; member offset 'b'
    mov r10, 2
    mov DWORD [rbx], r10d
    mov rbx, QWORD [rbp + 16] ; get value of s
    mov rax, rbx ; return via rax
    jmp L_function_make_structure_exit
    xor rax, rax ; Default return value 0
    L_function_make_structure_exit:
    mov rsp, rbp
    pop rbp
    ret
    
main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for locals
    lea rdi, QWORD [rbp + -16] ; zero initialize ab
    xor rax, rax
    mov ecx, 8
    rep stosb
    sub rsp, 32 ; reserve space for call arguments
    lea rbx, QWORD [rbp + -16] ; addrof ab
    mov rcx, rbx ; arg 0
    call make_structure
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea rbx, QWORD [rbp + -16] ; get address of ab
    add rbx, 4 ; member offset 'b'
    movsx rbx, DWORD [rbx]
    lea r10, QWORD [rbp + -16] ; get address of ab
    add r10, 0 ; member offset 'a'
    movsx r10, DWORD [r10]
    add r10, rbx
    mov rax, r10 ; return via rax
    jmp L_function_main_exit
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    
SECTION .data
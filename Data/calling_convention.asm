GLOBAL main

SECTION .code
calling_convention:
    mov QWORD [rsp + 1 * 8], rcx
    mov QWORD [rsp + 2 * 8], rdx
    mov QWORD [rsp + 3 * 8], r8
    mov QWORD [rsp + 4 * 8], r9
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    mov rbx, QWORD [rsp + 2 * 8] ; get rcx
    mov r10, QWORD [rsp + 3 * 8] ; get rdx
    add rbx, r10
    mov r10, QWORD [rsp + 4 * 8] ; get r8
    add rbx, r10
    mov r10, QWORD [rsp + 5 * 8] ; get r9
    add rbx, r10
    mov r10, QWORD [rsp + 6 * 8] ; get stack0
    add rbx, r10
    mov r10, QWORD [rsp + 7 * 8] ; get stack1
    add rbx, r10
    mov rax, rbx ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
main:
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    sub rsp, 32 + 2 * 8 ; shadow space + spill arguments
    mov rbx, 1
    mov rcx, rbx ; arg 0
    mov rbx, 2
    mov rdx, rbx ; arg 1
    mov rbx, 3
    mov r8, rbx ; arg 2
    mov rbx, 4
    mov r9, rbx ; arg 3
    mov rbx, 5
    mov QWORD [RSP + 4 * 8], rbx ; arg 4
    mov rbx, 6
    mov QWORD [RSP + 5 * 8], rbx ; arg 5
    call calling_convention
    add rsp, 6 * 8
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data

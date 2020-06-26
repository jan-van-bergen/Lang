GLOBAL main

SECTION .code
abc:
    mov QWORD [rsp + 1 * 8], rcx
    mov QWORD [rsp + 2 * 8], rdx
    sub rsp, 3 * 8; 3 vars
    mov QWORD [rsp + 0 * 8], 0 ; zero initialize ptr_a
    mov QWORD [rsp + 1 * 8], 0 ; zero initialize ptr_b
    lea rbx, QWORD [RSP + 4 * 8] ; addrof a
    lea r10, QWORD [rsp + 0 * 8] ; addr of ptr_a
    mov QWORD [r10], rbx
    lea rbx, QWORD [RSP + 5 * 8] ; addrof b
    lea r10, QWORD [rsp + 1 * 8] ; addr of ptr_b
    mov QWORD [r10], rbx
    mov QWORD [rsp + 2 * 8], 0 ; zero initialize ptr_ptr
    lea rbx, QWORD [RSP + 0 * 8] ; addrof ptr_a
    lea r10, QWORD [rsp + 2 * 8] ; addr of ptr_ptr
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 1 * 8] ; get ptr_b
    mov r10, QWORD [rsp + 2 * 8] ; get ptr_ptr
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 2 * 8] ; get ptr_ptr
    mov rbx, QWORD [rbx]
    mov rbx, QWORD [rbx]
    mov rax, rbx ; return via rax
    add rsp, 24
    ret
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
main:
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    sub rsp, 32 ; shadow space
    mov rbx, 1
    mov rcx, rbx ; arg 0
    mov rbx, 2
    mov rdx, rbx ; arg 1
    call abc
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data

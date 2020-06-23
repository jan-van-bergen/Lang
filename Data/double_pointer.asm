GLOBAL main

SECTION .code
abc:
    mov QWORD [rsp + 1 * 8], rcx
    mov QWORD [rsp + 2 * 8], rdx
    sub rsp, 4 * 8 + 8; 4 vars + alignment
    lea rax, QWORD [RSP + 6 * 8] ; addrof a
    mov QWORD [rsp + 0 * 8], rax ; set ptr_a
    lea rax, QWORD [RSP + 7 * 8] ; addrof b
    mov QWORD [rsp + 1 * 8], rax ; set ptr_b
    lea rax, QWORD [RSP + 0 * 8] ; addrof ptr_a
    mov QWORD [rsp + 2 * 8], rax ; set ptr_ptr
    mov rax, QWORD [rsp + 2 * 8]
    mov rbx, QWORD [rsp + 1 * 8] ; get ptr_b
    mov QWORD [rax], rbx ; set ptr ptr_ptr
    mov rax, QWORD [RSP + 2 * 8] ; deref ptr_ptr
    mov rax, QWORD [rax]
    mov QWORD [rsp + 3 * 8], rax ; set deref
    mov rax, QWORD [RSP + 3 * 8] ; deref deref
    mov rax, QWORD [rax]
    add rsp, 40
    ret
    ; Default return
    add rsp, 40
    xor rax, rax
    ret
    
main:
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    sub rsp, 32 ; shadow space
    mov rax, 1
    mov rcx, rax ; arg 0
    mov rax, 2
    mov rdx, rax ; arg 1
    call abc
    add rsp, 4 * 8
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data

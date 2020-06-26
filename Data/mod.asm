GLOBAL main

SECTION .code
main:
    sub rsp, 2 * 8 + 8; 2 vars + alignment
    mov QWORD [rsp + 0 * 8], 0 ; zero initialize a
    mov QWORD [rsp + 1 * 8], 0 ; zero initialize b
    mov rbx, 321
    mov r10, 43
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rdx
    lea r10, QWORD [rsp + 0 * 8] ; addr of a
    mov QWORD [r10], rbx
    mov rbx, 3
    mov r10, 5
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rdx
    lea r10, QWORD [rsp + 1 * 8] ; addr of b
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 0 * 8] ; get a
    mov r10, QWORD [rsp + 1 * 8] ; get b
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rdx
    mov rax, rbx ; return via rax
    add rsp, 24
    ret
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
SECTION .data

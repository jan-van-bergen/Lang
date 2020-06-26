GLOBAL main

SECTION .code
main:
    sub rsp, 3 * 8; 3 vars
    mov QWORD [rsp + 0 * 8], 0 ; zero initialize a
    mov QWORD [rsp + 1 * 8], 0 ; zero initialize b
    mov QWORD [rsp + 2 * 8], 0 ; zero initialize c
    mov rbx, 48
    lea r10, QWORD [rsp + 0 * 8] ; addr of a
    mov QWORD [r10], rbx
    mov rbx, 3
    lea r10, QWORD [rsp + 1 * 8] ; addr of b
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 0 * 8] ; get a
    mov r10, QWORD [rsp + 1 * 8] ; get b
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rax
    lea r10, QWORD [rsp + 2 * 8] ; addr of c
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 2 * 8] ; get c
    mov r10, 16
    cmp rbx, r10
    je L0
    mov rbx, 1
    jmp L1
    L0:
    mov rbx, 0
    L1:
    cmp rbx, 0
    je L_exit2
        mov rbx, 0
        mov rax, rbx ; return via rax
        add rsp, 24
        ret
    L_exit2:
    mov rbx, 4
    lea r10, QWORD [rsp + 1 * 8] ; addr of b
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 2 * 8] ; get c
    mov r10, QWORD [rsp + 1 * 8] ; get b
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rax
    lea r10, QWORD [rsp + 2 * 8] ; addr of c
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 2 * 8] ; get c
    mov r10, 4
    cmp rbx, r10
    je L3
    mov rbx, 1
    jmp L4
    L3:
    mov rbx, 0
    L4:
    cmp rbx, 0
    je L_exit5
        mov rbx, 0
        mov rax, rbx ; return via rax
        add rsp, 24
        ret
    L_exit5:
    mov rbx, QWORD [rsp + 2 * 8] ; get c
    mov r10, QWORD [rsp + 2 * 8] ; get c
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rax
    lea r10, QWORD [rsp + 2 * 8] ; addr of c
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 2 * 8] ; get c
    mov rax, rbx ; return via rax
    add rsp, 24
    ret
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
SECTION .data

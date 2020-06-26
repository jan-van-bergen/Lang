GLOBAL main

SECTION .code
main:
    sub rsp, 2 * 8 + 8; 2 vars + alignment
    mov rbx, 1
    mov QWORD [rsp + 0 * 8], rbx; initialize a
    mov QWORD [rsp + 1 * 8], 0; zero initialize b
    mov rbx, QWORD [rsp + 0 * 8] ; get a
    mov r10, 0
    cmp rbx, r10
    jle L0
    mov rbx, 1
    jmp L1
    L0:
    mov rbx, 0
    L1:
    cmp rbx, 0
    je L_else2
        mov rbx, 3
        lea r10, QWORD [rsp + 1 * 8] ; addr of b
        mov QWORD [r10], rbx
    jmp L_exit2
    L_else2:
        mov rbx, 2
        lea r10, QWORD [rsp + 1 * 8] ; addr of b
        mov QWORD [r10], rbx
    L_exit2:
    mov rbx, QWORD [rsp + 1 * 8] ; get b
    mov rax, rbx ; return via rax
    add rsp, 24
    ret
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
SECTION .data
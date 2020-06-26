GLOBAL main

SECTION .code
main:
    sub rsp, 3 * 8; 3 vars
    mov QWORD [rsp + 0 * 8], 0; zero initialize a
    mov QWORD [rsp + 1 * 8], 0; zero initialize b
    lea rbx, QWORD [rsp + 0 * 8] ; addrof a
    mov QWORD [rsp + 2 * 8], rbx; initialize ptr
    mov rbx, 3
    mov r10, QWORD [rsp + 2 * 8] ; get ptr
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 0 * 8] ; get a
    mov r10, 3
    cmp rbx, r10
    je L0
    mov rbx, 1
    jmp L1
    L0:
    mov rbx, 0
    L1:
    cmp rbx, 0
    je L_exit2
        mov rbx, -1
        mov rax, rbx ; return via rax
        add rsp, 24
        ret
    L_exit2:
    mov rbx, 5
    mov r10, QWORD [rsp + 2 * 8] ; get ptr
    mov r11, 8
    add r10, r11
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 1 * 8] ; get b
    mov rax, rbx ; return via rax
    add rsp, 24
    ret
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
SECTION .data
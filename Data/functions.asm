GLOBAL main

SECTION .code
one:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    mov rbx, QWORD [rsp + 2 * 8] ; get arg
    mov r10, 1
    add rbx, r10
    mov rax, rbx ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
two:
    mov QWORD [rsp + 1 * 8], rcx
    mov QWORD [rsp + 2 * 8], rdx
    sub rsp, 1 * 8; 1 vars
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 6 * 8] ; get a
    mov rcx, rbx ; arg 0
    call one
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov QWORD [rsp + 0 * 8], rbx; initialize local
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 7 * 8] ; get b
    mov rcx, rbx ; arg 0
    call one
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov r10, QWORD [rsp + 0 * 8] ; get local
    add rbx, r10
    mov rax, rbx ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
recursive:
    mov QWORD [rsp + 1 * 8], rcx
    mov QWORD [rsp + 2 * 8], rdx
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    mov rbx, QWORD [rsp + 2 * 8] ; get a
    mov r10, 0
    cmp rbx, r10
    jne L0
    mov rbx, 1
    jmp L1
    L0:
    mov rbx, 0
    L1:
    cmp rbx, 0
    je L_exit2
        mov rbx, QWORD [rsp + 3 * 8] ; get b
        mov rax, rbx ; return via rax
        add rsp, 8
        ret
    L_exit2:
    mov rbx, QWORD [rsp + 3 * 8] ; get b
    mov r10, 2
    add rbx, r10
    lea r10, QWORD [rsp + 3 * 8] ; addr of b
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 2 * 8] ; get a
    mov r10, 1
    sub rbx, r10
    lea r10, QWORD [rsp + 2 * 8] ; addr of a
    mov QWORD [r10], rbx
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 6 * 8] ; get a
    mov rcx, rbx ; arg 0
    mov rbx, QWORD [rsp + 7 * 8] ; get b
    mov rdx, rbx ; arg 1
    call recursive
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
main:
    sub rsp, 2 * 8 + 8; 2 vars + alignment
    sub rsp, 32 ; shadow space
    mov rbx, 1
    mov rcx, rbx ; arg 0
    mov rbx, 2
    mov rdx, rbx ; arg 1
    call two
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov QWORD [rsp + 0 * 8], rbx; initialize bla
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 4 * 8] ; get bla
    mov rcx, rbx ; arg 0
    call one
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov QWORD [rsp + 1 * 8], rbx; initialize tmp
    sub rsp, 32 ; shadow space
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 8 * 8] ; get bla
    mov rcx, rbx ; arg 0
    call one
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov rcx, rbx ; arg 0
    mov rbx, 0
    mov rdx, rbx ; arg 1
    call recursive
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    add rsp, 24
    ret
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
SECTION .data

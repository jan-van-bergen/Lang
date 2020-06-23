GLOBAL main

SECTION .code
one:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    mov rax, QWORD [rsp + 2 * 8] ; get arg
    mov rbx, 1
    add rax, rbx
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
    mov rax, QWORD [rsp + 6 * 8] ; get a
    mov rcx, rax ; arg 0
    call one
    add rsp, 4 * 8
    mov QWORD [rsp + 0 * 8], rax ; set local
    mov rax, QWORD [rsp + 0 * 8] ; get local
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 7 * 8] ; get b
    mov rcx, rbx ; arg 0
    mov r10, rax ; save rax
    call one
    add rsp, 4 * 8
    mov rbx, rax
    mov rax, r10 ; restore rax
    add rax, rbx
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
    mov rax, QWORD [rsp + 2 * 8] ; get a
    mov rbx, 0
    cmp rax, rbx
    jne L0
    mov rax, 1
    jmp L1
    L0:
    mov rax, 0
    L1:
    cmp rax, 0
    je L_exit2
        mov rax, QWORD [rsp + 3 * 8] ; get b
        add rsp, 8
        ret
    L_exit2:
    mov rax, QWORD [rsp + 3 * 8] ; get b
    mov rbx, 2
    add rax, rbx
    mov QWORD [rsp + 3 * 8], rax ; set b
    mov rax, QWORD [rsp + 2 * 8] ; get a
    mov rbx, 1
    sub rax, rbx
    mov QWORD [rsp + 2 * 8], rax ; set a
    sub rsp, 32 ; shadow space
    mov rax, QWORD [rsp + 6 * 8] ; get a
    mov rcx, rax ; arg 0
    mov rax, QWORD [rsp + 7 * 8] ; get b
    mov rdx, rax ; arg 1
    call recursive
    add rsp, 4 * 8
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
main:
    sub rsp, 1 * 8; 1 vars
    sub rsp, 32 ; shadow space
    mov rax, 1
    mov rcx, rax ; arg 0
    mov rax, 2
    mov rdx, rax ; arg 1
    call two
    add rsp, 4 * 8
    mov QWORD [rsp + 0 * 8], rax ; set bla
    sub rsp, 32 ; shadow space
    sub rsp, 32 ; shadow space
    mov rax, QWORD [rsp + 8 * 8] ; get bla
    mov rcx, rax ; arg 0
    call one
    add rsp, 4 * 8
    mov rcx, rax ; arg 0
    mov rax, 0
    mov rdx, rax ; arg 1
    call recursive
    add rsp, 4 * 8
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data

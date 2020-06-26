GLOBAL main

SECTION .code
factorial_recursive:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    mov rbx, QWORD [rsp + 2 * 8] ; get n
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
        mov rbx, 1
        mov rax, rbx ; return via rax
        add rsp, 8
        ret
    L_exit2:
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 6 * 8] ; get n
    mov r10, 1
    sub rbx, r10
    mov rcx, rbx ; arg 0
    call factorial_recursive
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov r10, QWORD [rsp + 2 * 8] ; get n
    imul r10, rbx
    mov rax, r10 ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
factorial_loop:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 1 * 8; 1 vars
    mov QWORD [rsp + 0 * 8], 0 ; zero initialize result
    mov rbx, 1
    lea r10, QWORD [rsp + 0 * 8] ; addr of result
    mov QWORD [r10], rbx
    L_loop3:
    mov rbx, QWORD [rsp + 2 * 8] ; get n
    mov r10, 0
    cmp rbx, r10
    jle L4
    mov rbx, 1
    jmp L5
    L4:
    mov rbx, 0
    L5:
    cmp rbx, 0
    je L_exit3
        mov rbx, QWORD [rsp + 0 * 8] ; get result
        mov r10, QWORD [rsp + 2 * 8] ; get n
        imul rbx, r10
        lea r10, QWORD [rsp + 0 * 8] ; addr of result
        mov QWORD [r10], rbx
        mov rbx, QWORD [rsp + 2 * 8] ; get n
        mov r10, 1
        sub rbx, r10
        lea r10, QWORD [rsp + 2 * 8] ; addr of n
        mov QWORD [r10], rbx
    jmp L_loop3
    L_exit3:
    mov rbx, QWORD [rsp + 0 * 8] ; get result
    mov rax, rbx ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
main:
    sub rsp, 3 * 8; 3 vars
    mov QWORD [rsp + 0 * 8], 0 ; zero initialize arg
    mov rbx, 5
    lea r10, QWORD [rsp + 0 * 8] ; addr of arg
    mov QWORD [r10], rbx
    mov QWORD [rsp + 1 * 8], 0 ; zero initialize a
    mov QWORD [rsp + 2 * 8], 0 ; zero initialize b
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 4 * 8] ; get arg
    mov rcx, rbx ; arg 0
    call factorial_recursive
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    lea r10, QWORD [rsp + 1 * 8] ; addr of a
    mov QWORD [r10], rbx
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 4 * 8] ; get arg
    mov rcx, rbx ; arg 0
    call factorial_loop
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    lea r10, QWORD [rsp + 2 * 8] ; addr of b
    mov QWORD [r10], rbx
    mov rbx, QWORD [rsp + 1 * 8] ; get a
    mov r10, QWORD [rsp + 2 * 8] ; get b
    cmp rbx, r10
    jne L6
    mov rbx, 1
    jmp L7
    L6:
    mov rbx, 0
    L7:
    mov rax, rbx ; return via rax
    add rsp, 24
    ret
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
SECTION .data

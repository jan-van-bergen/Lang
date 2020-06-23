GLOBAL main

SECTION .code
factorial_recursive:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    mov rax, QWORD [rsp + 2 * 8] ; get n
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
        mov rax, 1
        add rsp, 8
        ret
    L_exit2:
    sub rsp, 32 ; shadow space
    mov rax, QWORD [rsp + 6 * 8] ; get n
    mov rbx, 1
    sub rax, rbx
    mov rcx, rax ; arg 0
    call factorial_recursive
    add rsp, 4 * 8
    mov rbx, QWORD [rsp + 2 * 8] ; get n
    imul rax, rbx
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
factorial_loop:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 1 * 8; 1 vars
    mov QWORD [rsp + 0 * 8], 1 ; set result
    L_loop3:
    mov rax, QWORD [rsp + 2 * 8] ; get n
    mov rbx, 0
    cmp rax, rbx
    jle L4
    mov rax, 1
    jmp L5
    L4:
    mov rax, 0
    L5:
    cmp rax, 0
    je L_exit3
        mov rax, QWORD [rsp + 0 * 8] ; get result
        mov rbx, QWORD [rsp + 2 * 8] ; get n
        imul rax, rbx
        mov QWORD [rsp + 0 * 8], rax ; set result
        mov rax, QWORD [rsp + 2 * 8] ; get n
        mov rbx, 1
        sub rax, rbx
        mov QWORD [rsp + 2 * 8], rax ; set n
    jmp L_loop3
    L_exit3:
    mov rax, QWORD [rsp + 0 * 8] ; get result
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
main:
    sub rsp, 3 * 8; 3 vars
    mov QWORD [rsp + 0 * 8], 5 ; set arg
    sub rsp, 32 ; shadow space
    mov rax, QWORD [rsp + 4 * 8] ; get arg
    mov rcx, rax ; arg 0
    call factorial_recursive
    add rsp, 4 * 8
    mov QWORD [rsp + 1 * 8], rax ; set a
    sub rsp, 32 ; shadow space
    mov rax, QWORD [rsp + 4 * 8] ; get arg
    mov rcx, rax ; arg 0
    call factorial_loop
    add rsp, 4 * 8
    mov QWORD [rsp + 2 * 8], rax ; set b
    mov rax, QWORD [rsp + 1 * 8] ; get a
    mov rbx, QWORD [rsp + 2 * 8] ; get b
    cmp rax, rbx
    jne L6
    mov rax, 1
    jmp L7
    L6:
    mov rax, 0
    L7:
    add rsp, 24
    ret
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
SECTION .data

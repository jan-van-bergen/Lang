GLOBAL main

SECTION .code
EXTERN ExitProcess

assert:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    mov rbx, QWORD [rsp + 2 * 8] ; get expression
    test rbx, rbx
    jne L_lnot_false_0
    mov rbx, 1
    jmp L_lnot_exit_0
    L_lnot_false_0:
    mov rbx, 0
    L_lnot_exit_0:
    cmp rbx, 0
    je L_exit1
        sub rsp, 32 ; shadow space
        mov rbx, 1
        mov rcx, rbx ; arg 0
        call ExitProcess
        add rsp, 4 * 8
        mov rbx, rax ; get return value
    L_exit1:
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
main:
    sub rsp, 1 * 8; 1 vars
    mov QWORD [rsp + 0 * 8], 0; zero initialize a
    mov rbx, 1
    mov r10, 0
    test rbx, rbx
    jne L_lor_true_2
    test r10, r10
    jne L_lor_true_2
    mov rbx, 0
    jmp L_lor_exit_2
    L_lor_true_2:
    mov rbx, 1
    L_lor_exit_2:
    lea r10, QWORD [rsp + 0 * 8] ; addr of a
    mov QWORD [r10], rbx
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 4 * 8] ; get a
    mov r10, 1
    cmp rbx, r10
    jne L3
    mov rbx, 1
    jmp L4
    L3:
    mov rbx, 0
    L4:
    mov rcx, rbx ; arg 0
    call assert
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov rbx, 0
    mov r10, 1
    test rbx, rbx
    je L_land_false_5
    test r10, r10
    je L_land_false_5
    mov rbx, 1
    jmp L_land_exit_5
    L_land_false_5:
    mov rbx, 0
    L_land_exit_5:
    lea r10, QWORD [rsp + 0 * 8] ; addr of a
    mov QWORD [r10], rbx
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 4 * 8] ; get a
    mov r10, 0
    cmp rbx, r10
    jne L6
    mov rbx, 1
    jmp L7
    L6:
    mov rbx, 0
    L7:
    mov rcx, rbx ; arg 0
    call assert
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov rbx, QWORD [rsp + 0 * 8] ; get a
    test rbx, rbx
    jne L_lnot_false_8
    mov rbx, 1
    jmp L_lnot_exit_8
    L_lnot_false_8:
    mov rbx, 0
    L_lnot_exit_8:
    mov r10, QWORD [rsp + 0 * 8] ; get a
    test r10, r10
    jne L_lor_true_9
    test rbx, rbx
    jne L_lor_true_9
    mov r10, 0
    jmp L_lor_exit_9
    L_lor_true_9:
    mov r10, 1
    L_lor_exit_9:
    lea rbx, QWORD [rsp + 0 * 8] ; addr of a
    mov QWORD [rbx], r10
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 4 * 8] ; get a
    mov r10, 1
    cmp rbx, r10
    jne L10
    mov rbx, 1
    jmp L11
    L10:
    mov rbx, 0
    L11:
    mov rcx, rbx ; arg 0
    call assert
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov rbx, 0
    mov rax, rbx ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data
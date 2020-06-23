GLOBAL main

SECTION .code
main:
    sub rsp, 2 * 8 + 8; 2 vars + alignment
    mov QWORD [rsp + 0 * 8], 1 ; set a
    mov rax, QWORD [rsp + 0 * 8] ; get a
    mov rbx, 0
    cmp rax, rbx
    jle L0
    mov rax, 1
    jmp L1
    L0:
    mov rax, 0
    L1:
    cmp rax, 0
    je L_else2
        mov QWORD [rsp + 1 * 8], 3 ; set b
    jmp L_exit2
    L_else2:
        mov QWORD [rsp + 1 * 8], 2 ; set b
    L_exit2:
    mov rax, QWORD [rsp + 1 * 8] ; get b
    add rsp, 24
    ret
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
SECTION .data

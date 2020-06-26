GLOBAL main

SECTION .code
main:
    sub rsp, 1 * 8; 1 vars
    mov QWORD [rsp + 0 * 8], 0 ; zero initialize a
    mov rbx, QWORD [rsp + 0 * 8] ; get a
    mov r10, 1
    add rbx, r10
    mov r10, QWORD [rsp + 0 * 8] ; get a
    add r10, rbx
    mov rax, r10 ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data

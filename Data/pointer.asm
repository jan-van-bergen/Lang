GLOBAL main

SECTION .code
bla:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    mov rax, QWORD [rsp + 2 * 8]
    mov QWORD [rax], 4321 ; set ptr ptr
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
deref:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 1 * 8; 1 vars
    mov rax, QWORD [rsp + 2 * 8]
    mov QWORD [rax], 21 ; set ptr ptr
    mov QWORD [rsp + 0 * 8], 1234 ; set local
    sub rsp, 32 ; shadow space
    lea rax, QWORD [RSP + 4 * 8] ; addrof local
    mov rcx, rax ; arg 0
    call bla
    add rsp, 4 * 8
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
main:
    sub rsp, 1 * 8; 1 vars
    mov QWORD [rsp + 0 * 8], 42 ; set a
    sub rsp, 32 ; shadow space
    lea rax, QWORD [RSP + 4 * 8] ; addrof a
    mov rcx, rax ; arg 0
    call deref
    add rsp, 4 * 8
    mov rax, QWORD [rsp + 0 * 8] ; get a
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data

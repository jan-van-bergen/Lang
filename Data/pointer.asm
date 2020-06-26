GLOBAL main

SECTION .code
bla:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    mov rbx, 4321
    mov r10, QWORD [rsp + 2 * 8] ; get ptr
    mov QWORD [r10], rbx
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
deref:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 1 * 8; 1 vars
    mov QWORD [rsp + 0 * 8], 0; zero initialize local
    mov rbx, 21
    mov r10, QWORD [rsp + 2 * 8] ; get ptr
    mov QWORD [r10], rbx
    mov rbx, 1234
    lea r10, QWORD [rsp + 0 * 8] ; addr of local
    mov QWORD [r10], rbx
    sub rsp, 32 ; shadow space
    lea rbx, QWORD [rsp + 4 * 8] ; addrof local
    mov rcx, rbx ; arg 0
    call bla
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
main:
    sub rsp, 1 * 8; 1 vars
    mov rbx, 42
    mov QWORD [rsp + 0 * 8], rbx; initialize a
    sub rsp, 32 ; shadow space
    lea rbx, QWORD [rsp + 4 * 8] ; addrof a
    mov rcx, rbx ; arg 0
    call deref
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov rbx, QWORD [rsp + 0 * 8] ; get a
    mov rax, rbx ; return via rax
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data

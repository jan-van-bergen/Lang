GLOBAL main

SECTION .code
deref:
    mov QWORD [rsp + 1 * 8], rcx
    mov rax, QWORD [rsp + 1 * 8]
    mov QWORD [rax], 21 ; set ptr ptr
    ; Default return
    xor rax, rax
    ret
    
main:
    mov QWORD [rsp + 1 * 8], 42 ; set a
    sub rsp, 32 + 0
    lea rax, QWORD [RSP + 5 * 8] ; addrof a
    mov rcx, rax ; arg 0
    call deref
    add rsp, 32 + 0
    mov rax, QWORD [rsp + 1 * 8] ; get a
    ret
    ; Default return
    xor rax, rax
    ret
    
SECTION .data

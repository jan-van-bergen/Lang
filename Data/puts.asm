GLOBAL main

SECTION .code
EXTERN puts

main:
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    sub rsp, 32 ; shadow space
    lea rbx, [REL str_lit_0]
    mov rcx, rbx ; arg 0
    call puts
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data
str_lit_0 db "bla", 0

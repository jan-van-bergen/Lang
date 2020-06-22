GLOBAL main

SECTION .code
calling_convention:
    mov QWORD [rsp + 1 * 8], rcx
    mov QWORD [rsp + 2 * 8], rdx
    mov QWORD [rsp + 3 * 8], r8
    mov QWORD [rsp + 4 * 8], r9
    mov rax, QWORD [rsp + 1 * 8] ; get rcx
    mov rbx, QWORD [rsp + 2 * 8] ; get rdx
    add rax, rbx
    mov rbx, QWORD [rsp + 3 * 8] ; get r8
    add rax, rbx
    mov rbx, QWORD [rsp + 4 * 8] ; get r9
    add rax, rbx
    mov rbx, QWORD [rsp + 5 * 8] ; get stack0
    add rax, rbx
    mov rbx, QWORD [rsp + 6 * 8] ; get stack1
    add rax, rbx
    ret
    ; Default return
    xor rax, rax
    ret
    
main:
    sub rsp, 32 + 16
    mov rax, 1
    mov rcx, rax ; arg 0
    mov rax, 2
    mov rdx, rax ; arg 1
    mov rax, 3
    mov r8, rax ; arg 2
    mov rax, 4
    mov r9, rax ; arg 3
    mov rax, 5
    mov QWORD [RSP + 4 * 8], rax ; arg 4
    mov rax, 6
    mov QWORD [RSP + 5 * 8], rax ; arg 5
    call calling_convention
    add rsp, 32 + 16
    ret
    ; Default return
    xor rax, rax
    ret
    
SECTION .data

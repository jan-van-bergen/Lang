EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC

GLOBAL main

SECTION .data
hello_msg db "Hello world", 0
info_msg  db "Info", 0

SECTION .bss
alignb 8
SECTION .text

one:
    push rcx
    mov rax, QWORD [rsp + 0 * 8] ; get arg
    mov rbx, 1
    add rax, rbx
    add rsp, 8
    ret
two:
    push rcx
    push rdx
    mov rax, QWORD [rsp + 1 * 8] ; get a
    mov rcx, rax ; arg 0
    call one
    mov rbx, QWORD [rsp + 0 * 8] ; get b
    mov rcx, rbx ; arg 0
    mov r10, rax ; save rax
    call one
    mov rbx, rax
    mov rax, r10 ; restore rax
    add rax, rbx
    add rsp, 16
    ret
recursive:
    push rcx
    push rdx
    mov rax, QWORD [rsp + 1 * 8] ; get a
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
        mov rax, QWORD [rsp + 0 * 8] ; get b
        add rsp, 16
        ret
    L_exit2:
    mov rax, QWORD [rsp + 0 * 8] ; get b
    mov rbx, 2
    add rax, rbx
    mov QWORD [rsp + 0 * 8], rax ; set b
    mov rax, QWORD [rsp + 1 * 8] ; get a
    mov rbx, 1
    sub rax, rbx
    mov QWORD [rsp + 1 * 8], rax ; set a
    mov rax, QWORD [rsp + 1 * 8] ; get a
    mov rcx, rax ; arg 0
    mov rax, QWORD [rsp + 0 * 8] ; get b
    mov rdx, rax ; arg 1
    call recursive
    add rsp, 16
    ret
main:
    sub rsp, 8
    mov rax, 1
    mov rcx, rax ; arg 0
    mov rax, 2
    mov rdx, rax ; arg 1
    call two
    mov QWORD [rsp + 0 * 8], rax ; set bla
    mov rax, QWORD [rsp + 0 * 8] ; get bla
    mov rcx, rax ; arg 0
    mov rax, 0
    mov rdx, rax ; arg 1
    call recursive
    add rsp, 8
    ret

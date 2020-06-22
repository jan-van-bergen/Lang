EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC

GLOBAL main

SECTION .data
hello_msg db "Hello world", 0
info_msg  db "Info", 0

SECTION .bss
alignb 8
SECTION .text

main:
    sub rsp, 16
    mov QWORD [rsp + 0 * 8], 1
    mov rax, QWORD [rsp + 0 * 8]
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
        mov QWORD [rsp + 1 * 8], 3
    jmp L_exit2
    L_else2:
        mov QWORD [rsp + 1 * 8], 2
    L_exit2:
    mov rax, QWORD [rsp + 1 * 8]
    add rsp, 16
    ret

EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC

GLOBAL main

SECTION .data
hello_msg db "Hello world", 0
info_msg  db "Info", 0

SECTION .bss
alignb 8
a resd 1
b resq 1
c resq 1
d resq 1
i resq 1
j resq 1

SECTION .text

main:
    mov QWORD [REL a], 1
    mov rax, QWORD [REL a]
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
        mov QWORD [REL b], 3
    jmp L_exit2
    L_else2:
        mov QWORD [REL b], 2
    L_exit2:
    mov rax, QWORD [REL b]
    ret

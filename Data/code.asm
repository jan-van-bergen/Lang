EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC

GLOBAL main

SECTION .data
hello_msg db "Hello world", 0
info_msg  db "Info", 0

SECTION .bss
alignb 8
a resd 1
b resd 1
c resd 1
d resd 1
i resd 1
j resd 1

SECTION .text

main:
    mov DWORD [REL a], 0
    mov eax, DWORD [REL a]
    mov ebx, 0
    cmp eax, ebx
    jle L0
    mov eax, 1
    jmp L1
    L0:
    mov eax, 0
    L1:
    cmp eax, 0
    je L_else2
        mov DWORD [REL b], 3
    jmp L_exit2
    L_else2:
        mov DWORD [REL b], 2
    L_exit2:
    mov eax, DWORD [REL b]
    ret

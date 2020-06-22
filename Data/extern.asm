EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC

GLOBAL main

SECTION .data
hello_msg db "Hello world", 0
info_msg  db "Info", 0

SECTION .bss
alignb 8

SECTION .code
EXTERN GetStdHandle

EXTERN WriteFile

main:
    sub rsp, 16
    sub rsp, 32 + 0
    mov rax, 11
    neg rax
    mov rcx, rax ; arg 0
    call GetStdHandle
    add rsp, 32 + 0
    mov QWORD [rsp + 1 * 8], rax ; set std_handle
    sub rsp, 32 + 16
    mov rax, QWORD [rsp + 7 * 8] ; get std_handle
    mov rcx, rax ; arg 0
    lea rax, [REL hello_msg]
    mov rdx, rax ; arg 1
    mov rax, 12
    mov r8, rax ; arg 2
    lea rax, QWORD [RSP + 6 * 8] ; addrof bytes_written
    mov r9, rax ; arg 3
    mov rax, 0
    mov QWORD [RSP + 4 * 8], rax ; arg 4
    call WriteFile
    add rsp, 32 + 16
    mov rax, 0
    add rsp, 16
    ret


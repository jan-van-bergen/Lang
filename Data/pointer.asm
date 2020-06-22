EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC

GLOBAL main

SECTION .data
hello_msg db "Hello world", 0
info_msg  db "Info", 0

SECTION .bss
alignb 8

SECTION .code
use_ptr:
    push rcx
    mov rax, QWORD [RSP + 0 * 8] ; deref ptr
    mov rax, QWORD [rax]
    add rsp, 8
    ret

main:
    sub rsp, 8
    mov QWORD [rsp + 0 * 8], 42 ; set a
    lea rax, QWORD [RSP + 0 * 8] ; addrof a
    mov rcx, rax ; arg 0
    call use_ptr
    add rsp, 8
    ret


EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC

GLOBAL main

SECTION .code
EXTERN GetStdHandle

EXTERN WriteFile

test:
    push rcx
    push rdx
    push r8
    push r9
    mov rax, QWORD [rsp + 3 * 8] ; get a
    mov rbx, QWORD [rsp + 2 * 8] ; get b
    add rax, rbx
    mov rbx, QWORD [rsp + 1 * 8] ; get c
    add rax, rbx
    mov rbx, QWORD [rsp + 0 * 8] ; get d
    add rax, rbx
    add rsp, 32
    ret

main:
    sub rsp, 24
    sub rsp, 32 + 0
    mov rax, 11
    neg rax
    mov rcx, rax ; arg 0
    call GetStdHandle
    add rsp, 32 + 0
    mov QWORD [rsp + 2 * 8], rax ; set std_handle
    sub rsp, 32 + 0
    mov rax, 10
    mov rcx, rax ; arg 0
    mov rax, 1
    mov rdx, rax ; arg 1
    mov rax, 1
    mov r8, rax ; arg 2
    mov rax, 1
    mov r9, rax ; arg 3
    call test
    add rsp, 32 + 0
    mov QWORD [rsp + 1 * 8], rax ; set str_len
    sub rsp, 32 + 16
    mov rax, QWORD [rsp + 8 * 8] ; get std_handle
    mov rcx, rax ; arg 0
    lea rax, [REL str_lit_0]
    mov rdx, rax ; arg 1
    mov rax, QWORD [rsp + 7 * 8] ; get str_len
    mov r8, rax ; arg 2
    lea rax, QWORD [RSP + 6 * 8] ; addrof bytes_written
    mov r9, rax ; arg 3
    mov rax, 0
    mov QWORD [RSP + 4 * 8], rax ; arg 4
    call WriteFile
    add rsp, 32 + 16
    mov rax, QWORD [rsp + 0 * 8] ; get bytes_written
    add rsp, 24
    ret

SECTION .data
str_lit_0 db "Hallo wereld!", 0

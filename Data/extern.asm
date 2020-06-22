GLOBAL main

SECTION .code
EXTERN GetStdHandle

EXTERN WriteFile

EXTERN lstrlenA

main:
    sub rsp, 40
    mov rax, 11
    neg rax
    mov QWORD [rsp + 4 * 8], rax ; set STD_OUTPUT_HANDLE
    sub rsp, 32 + 0
    mov rax, QWORD [rsp + 8 * 8] ; get STD_OUTPUT_HANDLE
    mov rcx, rax ; arg 0
    call GetStdHandle
    add rsp, 32 + 0
    mov QWORD [rsp + 3 * 8], rax ; set std_handle
    lea rax, [REL str_lit_0]
    mov QWORD [rsp + 2 * 8], rax ; set str
    sub rsp, 32 + 0
    mov rax, QWORD [rsp + 6 * 8] ; get str
    mov rcx, rax ; arg 0
    call lstrlenA
    add rsp, 32 + 0
    mov QWORD [rsp + 1 * 8], rax ; set str_len
    sub rsp, 32 + 16
    mov rax, QWORD [rsp + 9 * 8] ; get std_handle
    mov rcx, rax ; arg 0
    mov rax, QWORD [rsp + 8 * 8] ; get str
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
    add rsp, 40
    ret
    ; Default return
    add rsp, 40
    xor rax, rax
    ret
    
SECTION .data
str_lit_0 db "Hallo wereld!", 0

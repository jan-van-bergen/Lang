GLOBAL main

SECTION .code
EXTERN GetStdHandle

EXTERN WriteFile

EXTERN lstrlenA

main:
    mov rax, 11
    neg rax
    mov QWORD [rsp + 1 * 8], rax ; set STD_OUTPUT_HANDLE
    sub rsp, 32 + 0
    mov rax, QWORD [rsp + 5 * 8] ; get STD_OUTPUT_HANDLE
    mov rcx, rax ; arg 0
    call GetStdHandle
    add rsp, 32 + 0
    mov QWORD [rsp + 2 * 8], rax ; set std_handle
    sub rsp, 32 + 16
    mov rax, QWORD [rsp + 8 * 8] ; get std_handle
    mov rcx, rax ; arg 0
    lea rax, [REL str_lit_0]
    mov rdx, rax ; arg 1
    mov rax, 13
    mov r8, rax ; arg 2
    lea rax, QWORD [RSP + 9 * 8] ; addrof bytes_written
    mov r9, rax ; arg 3
    mov rax, 0
    mov QWORD [RSP + 4 * 8], rax ; arg 4
    call WriteFile
    add rsp, 32 + 16
    mov rax, QWORD [rsp + 3 * 8] ; get bytes_written
    ret
    ; Default return
    xor rax, rax
    ret
    
SECTION .data
str_lit_0 db "Hallo wereld!", 0

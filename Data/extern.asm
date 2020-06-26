GLOBAL main

SECTION .code
EXTERN GetStdHandle

EXTERN WriteFile

EXTERN strlen

main:
    sub rsp, 4 * 8 + 8; 4 vars + alignment
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [REL STD_OUTPUT_HANDLE] ; get STD_OUTPUT_HANDLE
    mov rcx, rbx ; arg 0
    call GetStdHandle
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov QWORD [rsp + 0 * 8], rbx; initialize std_handle
    lea rbx, [REL str_lit_1]
    mov QWORD [rsp + 1 * 8], rbx; initialize str
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 5 * 8] ; get str
    mov rcx, rbx ; arg 0
    call strlen
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    mov QWORD [rsp + 2 * 8], rbx; initialize str_len
    mov QWORD [rsp + 3 * 8], 0; zero initialize bytes_written
    sub rsp, 32 + 1 * 8 + 8 ; shadow space + spill arguments + alignment
    mov rbx, QWORD [rsp + 6 * 8] ; get std_handle
    mov rcx, rbx ; arg 0
    mov rbx, QWORD [rsp + 7 * 8] ; get str
    mov rdx, rbx ; arg 1
    mov rbx, QWORD [rsp + 8 * 8] ; get str_len
    mov r8, rbx ; arg 2
    lea rbx, QWORD [rsp + 9 * 8] ; addrof bytes_written
    mov r9, rbx ; arg 3
    mov rbx, 0
    mov QWORD [RSP + 4 * 8], rbx ; arg 4
    call WriteFile
    add rsp, 6 * 8
    mov rbx, rax ; get return value
    mov rbx, QWORD [rsp + 3 * 8] ; get bytes_written
    mov rax, rbx ; return via rax
    add rsp, 40
    ret
    ; Default return
    add rsp, 40
    xor rax, rax
    ret
    
SECTION .data
STD_OUTPUT_HANDLE dq -11
str_lit_1 db "Hallo wereld!", 0

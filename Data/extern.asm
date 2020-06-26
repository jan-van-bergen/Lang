GLOBAL main

SECTION .code
EXTERN GetStdHandle

EXTERN WriteFile

EXTERN strlen

main:
    sub rsp, 5 * 8; 5 vars
    mov QWORD [rsp + 0 * 8], 0 ; zero initialize STD_OUTPUT_HANDLE
    mov rbx, 11
    neg rbx
    lea r10, QWORD [rsp + 0 * 8] ; addr of STD_OUTPUT_HANDLE
    mov QWORD [r10], rbx
    mov QWORD [rsp + 1 * 8], 0 ; zero initialize std_handle
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 4 * 8] ; get STD_OUTPUT_HANDLE
    mov rcx, rbx ; arg 0
    call GetStdHandle
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    lea r10, QWORD [rsp + 1 * 8] ; addr of std_handle
    mov QWORD [r10], rbx
    mov QWORD [rsp + 2 * 8], 0 ; zero initialize str
    lea rbx, [REL str_lit_0]
    lea r10, QWORD [rsp + 2 * 8] ; addr of str
    mov QWORD [r10], rbx
    mov QWORD [rsp + 3 * 8], 0 ; zero initialize str_len
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 6 * 8] ; get str
    mov rcx, rbx ; arg 0
    call strlen
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    lea r10, QWORD [rsp + 3 * 8] ; addr of str_len
    mov QWORD [r10], rbx
    mov QWORD [rsp + 4 * 8], 0 ; zero initialize bytes_written
    sub rsp, 32 + 1 * 8 + 8 ; shadow space + spill arguments + alignment
    mov rbx, QWORD [rsp + 7 * 8] ; get std_handle
    mov rcx, rbx ; arg 0
    mov rbx, QWORD [rsp + 8 * 8] ; get str
    mov rdx, rbx ; arg 1
    mov rbx, QWORD [rsp + 9 * 8] ; get str_len
    mov r8, rbx ; arg 2
    lea rbx, QWORD [RSP + 10 * 8] ; addrof bytes_written
    mov r9, rbx ; arg 3
    mov rbx, 0
    mov QWORD [RSP + 4 * 8], rbx ; arg 4
    call WriteFile
    add rsp, 6 * 8
    mov rbx, rax ; get return value
    mov rbx, QWORD [rsp + 4 * 8] ; get bytes_written
    mov rax, rbx ; return via rax
    add rsp, 40
    ret
    ; Default return
    add rsp, 40
    xor rax, rax
    ret
    
SECTION .data
str_lit_0 db "Hallo wereld!", 0

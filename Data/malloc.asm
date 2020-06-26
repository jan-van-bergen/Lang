GLOBAL main

SECTION .code
EXTERN GetStdHandle

EXTERN WriteFile

EXTERN _malloc

main:
    sub rsp, 6 * 8 + 8; 6 vars + alignment
    sub rsp, 32 ; shadow space
    mov rax, 11
    neg rax
    mov rcx, rax ; arg 0
    call GetStdHandle
    add rsp, 4 * 8
    mov QWORD [rsp + 0 * 8], rax ; set std_handle
    mov QWORD [rsp + 1 * 8], 16 ; set str_len
    sub rsp, 32 ; shadow space
    mov rax, QWORD [rsp + 5 * 8] ; get str_len
    mov rcx, rax ; arg 0
    call _malloc
    add rsp, 4 * 8
    mov QWORD [rsp + 2 * 8], rax ; set mem
    mov QWORD [rsp + 3 * 8], 0 ; set i
    L_loop0:
    mov rax, QWORD [rsp + 3 * 8] ; get i
    mov rbx, QWORD [rsp + 1 * 8] ; get str_len
    cmp rax, rbx
    jge L1
    mov rax, 1
    jmp L2
    L1:
    mov rax, 0
    L2:
    cmp rax, 0
    je L_exit0
        mov rax, QWORD [rsp + 2 * 8] ; get mem
        mov rbx, QWORD [rsp + 3 * 8] ; get i
        add rax, rbx
        mov QWORD [rsp + 4 * 8], rax ; set tmp
        mov rax, QWORD [rsp + 4 * 8]
        mov QWORD [rax], 1684234849 ; set ptr tmp
        mov rax, QWORD [rsp + 3 * 8] ; get i
        mov rbx, 4
        add rax, rbx
        mov QWORD [rsp + 3 * 8], rax ; set i
    jmp L_loop0
    L_exit0:
    sub rsp, 32 + 1 * 8 + 8 ; shadow space + spill arguments + alignment
    mov rax, QWORD [rsp + 6 * 8] ; get std_handle
    mov rcx, rax ; arg 0
    mov rax, QWORD [rsp + 8 * 8] ; get mem
    mov rdx, rax ; arg 1
    mov rax, QWORD [rsp + 7 * 8] ; get str_len
    mov r8, rax ; arg 2
    lea rax, QWORD [RSP + 11 * 8] ; addrof bytes_written
    mov r9, rax ; arg 3
    mov rax, 0
    mov QWORD [RSP + 4 * 8], rax ; arg 4
    call WriteFile
    add rsp, 6 * 8
    mov rax, QWORD [rsp + 5 * 8] ; get bytes_written
    add rsp, 56
    ret
    ; Default return
    add rsp, 56
    xor rax, rax
    ret
    
SECTION .data

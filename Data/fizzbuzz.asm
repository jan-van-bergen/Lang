GLOBAL main

SECTION .code
EXTERN GetStdHandle

EXTERN WriteFile

EXTERN strlen

print:
    mov QWORD [rsp + 1 * 8], rcx
    mov QWORD [rsp + 2 * 8], rdx
    sub rsp, 3 * 8; 3 vars
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
    mov QWORD [rsp + 2 * 8], 0 ; zero initialize bytes_written
    sub rsp, 32 + 1 * 8 + 8 ; shadow space + spill arguments + alignment
    mov rbx, QWORD [rsp + 7 * 8] ; get std_handle
    mov rcx, rbx ; arg 0
    mov rbx, QWORD [rsp + 10 * 8] ; get str
    mov rdx, rbx ; arg 1
    mov rbx, QWORD [rsp + 11 * 8] ; get str_len
    mov r8, rbx ; arg 2
    lea rbx, QWORD [RSP + 8 * 8] ; addrof bytes_written
    mov r9, rbx ; arg 3
    mov rbx, 0
    mov QWORD [RSP + 4 * 8], rbx ; arg 4
    call WriteFile
    add rsp, 6 * 8
    mov rbx, rax ; get return value
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
print_num:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 5 * 8; 5 vars
    mov QWORD [rsp + 0 * 8], 0 ; zero initialize num_str
    lea rbx, [REL str_lit_0]
    lea r10, QWORD [rsp + 0 * 8] ; addr of num_str
    mov QWORD [r10], rbx
    mov QWORD [rsp + 1 * 8], 0 ; zero initialize idx
    mov rbx, 0
    lea r10, QWORD [rsp + 1 * 8] ; addr of idx
    mov QWORD [r10], rbx
    L_loop0:
    mov rbx, QWORD [rsp + 6 * 8] ; get num
    mov r10, 0
    cmp rbx, r10
    jle L1
    mov rbx, 1
    jmp L2
    L1:
    mov rbx, 0
    L2:
    cmp rbx, 0
    je L_exit0
        mov QWORD [rsp + 2 * 8], 0 ; zero initialize digit
        mov rbx, QWORD [rsp + 6 * 8] ; get num
        mov r10, 10
        mov rax, rbx
        cdq
        idiv r10
        mov rbx, rdx
        lea r10, QWORD [rsp + 2 * 8] ; addr of digit
        mov QWORD [r10], rbx
        mov rbx, QWORD [rsp + 2 * 8] ; get digit
        mov r10, 48
        add rbx, r10
        mov r10, QWORD [rsp + 0 * 8] ; get num_str
        mov r11, QWORD [rsp + 1 * 8] ; get idx
        add r10, r11
        mov QWORD [r10], rbx
        mov rbx, QWORD [rsp + 6 * 8] ; get num
        mov r10, 10
        mov rax, rbx
        cdq
        idiv r10
        mov rbx, rax
        lea r10, QWORD [rsp + 6 * 8] ; addr of num
        mov QWORD [r10], rbx
        mov rbx, QWORD [rsp + 1 * 8] ; get idx
        mov r10, 1
        add rbx, r10
        lea r10, QWORD [rsp + 1 * 8] ; addr of idx
        mov QWORD [r10], rbx
    jmp L_loop0
    L_exit0:
    mov QWORD [rsp + 3 * 8], 0 ; zero initialize i
    mov rbx, 0
    lea r10, QWORD [rsp + 3 * 8] ; addr of i
    mov QWORD [r10], rbx
    L_loop3:
    mov rbx, QWORD [rsp + 1 * 8] ; get idx
    mov r10, 2
    mov rax, rbx
    cdq
    idiv r10
    mov rbx, rax
    mov r10, QWORD [rsp + 3 * 8] ; get i
    cmp r10, rbx
    jge L4
    mov r10, 1
    jmp L5
    L4:
    mov r10, 0
    L5:
    cmp r10, 0
    je L_exit3
        mov QWORD [rsp + 4 * 8], 0 ; zero initialize tmp
        mov rbx, QWORD [rsp + 0 * 8] ; get num_str
        mov r10, QWORD [rsp + 3 * 8] ; get i
        add rbx, r10
        mov rbx, QWORD [rbx]
        lea r10, QWORD [rsp + 4 * 8] ; addr of tmp
        mov QWORD [r10], rbx
        mov rbx, QWORD [rsp + 0 * 8] ; get num_str
        mov r10, QWORD [rsp + 1 * 8] ; get idx
        add rbx, r10
        mov r10, QWORD [rsp + 3 * 8] ; get i
        sub rbx, r10
        mov r10, 1
        sub rbx, r10
        mov rbx, QWORD [rbx]
        mov r10, QWORD [rsp + 0 * 8] ; get num_str
        mov r11, QWORD [rsp + 3 * 8] ; get i
        add r10, r11
        mov QWORD [r10], rbx
        mov rbx, QWORD [rsp + 4 * 8] ; get tmp
        mov r10, QWORD [rsp + 0 * 8] ; get num_str
        mov r11, QWORD [rsp + 1 * 8] ; get idx
        add r10, r11
        mov r11, QWORD [rsp + 3 * 8] ; get i
        sub r10, r11
        mov r11, 1
        sub r10, r11
        mov QWORD [r10], rbx
        mov rbx, QWORD [rsp + 3 * 8] ; get i
        mov r10, 1
        add rbx, r10
        lea r10, QWORD [rsp + 3 * 8] ; addr of i
        mov QWORD [r10], rbx
    jmp L_loop3
    L_exit3:
    mov rbx, 32
    mov r10, QWORD [rsp + 0 * 8] ; get num_str
    mov r11, QWORD [rsp + 1 * 8] ; get idx
    add r10, r11
    mov QWORD [r10], rbx
    sub rsp, 32 ; shadow space
    mov rbx, QWORD [rsp + 4 * 8] ; get num_str
    mov rcx, rbx ; arg 0
    mov rbx, QWORD [rsp + 5 * 8] ; get idx
    mov r10, 1
    add rbx, r10
    mov rdx, rbx ; arg 1
    call print
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    ; Default return
    add rsp, 40
    xor rax, rax
    ret
    
fizzbuzz:
    mov QWORD [rsp + 1 * 8], rcx
    sub rsp, 3 * 8; 3 vars
    mov QWORD [rsp + 0 * 8], 0 ; zero initialize i
    mov rbx, 1
    lea r10, QWORD [rsp + 0 * 8] ; addr of i
    mov QWORD [r10], rbx
    L_loop6:
    mov rbx, QWORD [rsp + 0 * 8] ; get i
    mov r10, QWORD [rsp + 4 * 8] ; get n
    cmp rbx, r10
    jg L7
    mov rbx, 1
    jmp L8
    L7:
    mov rbx, 0
    L8:
    cmp rbx, 0
    je L_exit6
        mov QWORD [rsp + 1 * 8], 0 ; zero initialize divisible_by_3
        mov rbx, QWORD [rsp + 0 * 8] ; get i
        mov r10, 3
        mov rax, rbx
        cdq
        idiv r10
        mov rbx, rdx
        mov r10, 0
        cmp rbx, r10
        jne L9
        mov rbx, 1
        jmp L10
        L9:
        mov rbx, 0
        L10:
        lea r10, QWORD [rsp + 1 * 8] ; addr of divisible_by_3
        mov QWORD [r10], rbx
        mov QWORD [rsp + 2 * 8], 0 ; zero initialize divisible_by_5
        mov rbx, QWORD [rsp + 0 * 8] ; get i
        mov r10, 5
        mov rax, rbx
        cdq
        idiv r10
        mov rbx, rdx
        mov r10, 0
        cmp rbx, r10
        jne L11
        mov rbx, 1
        jmp L12
        L11:
        mov rbx, 0
        L12:
        lea r10, QWORD [rsp + 2 * 8] ; addr of divisible_by_5
        mov QWORD [r10], rbx
        mov rbx, QWORD [rsp + 1 * 8] ; get divisible_by_3
        cmp rbx, 0
        je L_else13
            mov rbx, QWORD [rsp + 2 * 8] ; get divisible_by_5
            cmp rbx, 0
            je L_else14
                sub rsp, 32 ; shadow space
                lea rbx, [REL str_lit_1]
                mov rcx, rbx ; arg 0
                mov rbx, 9
                mov rdx, rbx ; arg 1
                call print
                add rsp, 4 * 8
                mov rbx, rax ; get return value
            jmp L_exit14
            L_else14:
                sub rsp, 32 ; shadow space
                lea rbx, [REL str_lit_2]
                mov rcx, rbx ; arg 0
                mov rbx, 5
                mov rdx, rbx ; arg 1
                call print
                add rsp, 4 * 8
                mov rbx, rax ; get return value
            L_exit14:
        jmp L_exit13
        L_else13:
            mov rbx, QWORD [rsp + 2 * 8] ; get divisible_by_5
            cmp rbx, 0
            je L_else15
                sub rsp, 32 ; shadow space
                lea rbx, [REL str_lit_3]
                mov rcx, rbx ; arg 0
                mov rbx, 5
                mov rdx, rbx ; arg 1
                call print
                add rsp, 4 * 8
                mov rbx, rax ; get return value
            jmp L_exit15
            L_else15:
                sub rsp, 32 ; shadow space
                mov rbx, QWORD [rsp + 4 * 8] ; get i
                mov rcx, rbx ; arg 0
                call print_num
                add rsp, 4 * 8
                mov rbx, rax ; get return value
            L_exit15:
        L_exit13:
        mov rbx, QWORD [rsp + 0 * 8] ; get i
        mov r10, 1
        add rbx, r10
        lea r10, QWORD [rsp + 0 * 8] ; addr of i
        mov QWORD [r10], rbx
    jmp L_loop6
    L_exit6:
    ; Default return
    add rsp, 24
    xor rax, rax
    ret
    
main:
    sub rsp, 0 * 8 + 8; 0 vars + alignment
    sub rsp, 32 ; shadow space
    mov rbx, 20
    mov rcx, rbx ; arg 0
    call fizzbuzz
    add rsp, 4 * 8
    mov rbx, rax ; get return value
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data
str_lit_0 db "         ", 0
str_lit_1 db "fizzbuzz ", 0
str_lit_2 db "buzz ", 0
str_lit_3 db "fizz ", 0

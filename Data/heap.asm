; Generated by Lang compiler

GLOBAL main

SECTION .code
EXTERN GetProcessHeap

EXTERN HeapAlloc

EXTERN HeapFree

EXTERN GetStdHandle

EXTERN WriteFile

EXTERN ExitProcess



malloc:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov rbx, QWORD [REL heap]
    mov r10, QWORD [REL NULL]
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit0
        sub rsp, 32 ; reserve shadow space and 0 arguments
        call GetProcessHeap
        add rsp, 32 ; pop arguments
        mov rbx, rax ; get return value
        lea r10, QWORD [REL heap] ; get address of 'heap'
        mov QWORD [r10], rbx
        
    L_exit0:
    
    sub rsp, 32 ; reserve shadow space and 3 arguments
    mov rbx, QWORD [REL heap]
    mov rcx, rbx ; arg 1
    mov rbx, 0
    mov rdx, rbx ; arg 2
    movsx rbx, DWORD [rbp + 16]
    mov r8, rbx ; arg 3
    call HeapAlloc
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    jmp L_function_malloc_exit
    
    xor rax, rax ; Default return value 0
    L_function_malloc_exit:
    mov rsp, rbp
    pop rbp
    ret
    

free:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, QWORD [rbp + 16]
    mov r10, QWORD [REL NULL]
    cmp rbx, r10
    setne bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, QWORD [REL heap]
    mov r10, QWORD [REL NULL]
    cmp rbx, r10
    setne bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    sub rsp, 32 ; reserve shadow space and 3 arguments
    mov rbx, QWORD [REL heap]
    mov rcx, rbx ; arg 1
    mov rbx, 0
    mov rdx, rbx ; arg 2
    mov rbx, QWORD [rbp + 16]
    mov r8, rbx ; arg 3
    call HeapFree
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    xor rax, rax ; Default return value 0
    L_function_free_exit:
    mov rsp, rbp
    pop rbp
    ret
    

assert:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov BYTE [rbp + 16], cl ; push arg 0 
    movzx rbx, BYTE [rbp + 16]
    xor rbx, -1
    and rbx, 1
    cmp rbx, 0
    je L_exit2
        sub rsp, 32 ; reserve shadow space and 1 arguments
        mov rbx, 1
        mov rcx, rbx ; arg 1
        call ExitProcess
        add rsp, 32 ; pop arguments
        mov rbx, rax ; get return value
        
    L_exit2:
    
    xor rax, rax ; Default return value 0
    L_function_assert_exit:
    mov rsp, rbp
    pop rbp
    ret
    


print:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    sub rsp, 16 ; reserve stack space for 2 locals
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [REL STD_OUTPUT_HANDLE]
    mov rcx, rbx ; arg 1
    call GetStdHandle
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -16] ; get address of 'std_handle'
    mov QWORD [r10], rbx
    
    mov DWORD [rbp + -8], 0 ; zero initialize 'bytes_written'
    
    sub rsp, 48 ; reserve shadow space and 5 arguments
    mov rbx, QWORD [rbp + -16]
    mov rcx, rbx ; arg 1
    mov rbx, QWORD [rbp + 16]
    mov rdx, rbx ; arg 2
    movsx rbx, DWORD [rbp + 24]
    mov r8, rbx ; arg 3
    lea rbx, QWORD [rbp + -8] ; get address of 'bytes_written'
    mov r9, rbx ; arg 4
    mov rbx, 0
    mov DWORD [rsp + 32], ebx ; arg 5
    call WriteFile
    add rsp, 48 ; pop arguments
    mov rbx, rax ; get return value
    
    xor rax, rax ; Default return value 0
    L_function_print_exit:
    mov rsp, rbp
    pop rbp
    ret
    

strlen:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    sub rsp, 16 ; reserve stack space for 1 locals
    lea rbx, QWORD [rbp + -16] ; get address of 'len'
    mov r10, 0
    mov DWORD [rbx], r10d
    
    L_loop3:
    mov rbx, QWORD [rbp + 16]
    movsx r10, DWORD [rbp + -16]
    add rbx, r10
    movzx rbx, BYTE [rbx]
    mov r10, 0
    cmp rbx, r10
    setne bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit3
        movsx rbx, DWORD [rbp + -16]
        mov r10, 1
        add rbx, r10
        lea r10, QWORD [rbp + -16] ; get address of 'len'
        mov DWORD [r10], ebx
        
    jmp L_loop3
    L_exit3:
    
    movsx rbx, DWORD [rbp + -16]
    mov rax, rbx ; return via rax
    jmp L_function_strlen_exit
    
    xor rax, rax ; Default return value 0
    L_function_strlen_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 32 ; reserve stack space for 3 locals
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, 16
    mov rcx, rbx ; arg 1
    call malloc
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -32] ; get address of 'mem'
    mov QWORD [r10], rbx
    
    mov rbx, QWORD [rbp + -32]
    mov r10, 0
    add rbx, r10
    mov r10, 66
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -32]
    mov r10, 1
    add rbx, r10
    mov r10, 114
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -32]
    mov r10, 2
    add rbx, r10
    mov r10, 117
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -32]
    mov r10, 3
    add rbx, r10
    mov r10, 104
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -32]
    mov r10, 4
    add rbx, r10
    mov r10, 84
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -32]
    mov r10, 5
    add rbx, r10
    mov r10, 101
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -32]
    mov r10, 6
    add rbx, r10
    mov r10, 115
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -32]
    mov r10, 7
    add rbx, r10
    mov r10, 116
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -32]
    mov r10, 8
    add rbx, r10
    mov r10, 0
    mov BYTE [rbx], r10b
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, QWORD [rbp + -32]
    mov rcx, rbx ; arg 1
    call strlen
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -24] ; get address of 'str_len'
    mov DWORD [r10], ebx
    
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, QWORD [rbp + -32]
    mov rcx, rbx ; arg 1
    movsx rbx, DWORD [rbp + -24]
    mov rdx, rbx ; arg 2
    call print
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, QWORD [rbp + -32]
    mov rcx, rbx ; arg 1
    call free
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, 16
    mov rcx, rbx ; arg 1
    call malloc
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -16] ; get address of 'mem2'
    mov QWORD [r10], rbx
    
    mov rbx, QWORD [rbp + -16]
    mov r10, 0
    add rbx, r10
    mov r10, 1
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -16]
    mov r10, 1
    add rbx, r10
    mov r10, 2
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -16]
    mov r10, 2
    add rbx, r10
    mov r10, 3
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -16]
    mov r10, 3
    add rbx, r10
    mov r10, 4
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -16]
    mov r10, 4
    add rbx, r10
    mov r10, 0
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -16]
    mov r10, 5
    add rbx, r10
    mov r10, 0
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -16]
    mov r10, 6
    add rbx, r10
    mov r10, 0
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -16]
    mov r10, 7
    add rbx, r10
    mov r10, 0
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -16]
    movsx rbx, DWORD [rbx]
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data
NULL dq 0
heap dq 0
STD_OUTPUT_HANDLE dq -11

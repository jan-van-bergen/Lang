; Generated by Lang compiler

GLOBAL main

SECTION .code

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 4 locals
    lea rbx, QWORD [rbp + -16] ; get address of 'num_primes'
    mov r10, 0
    mov DWORD [rbx], r10d
    
    lea rbx, QWORD [rbp + -12] ; get address of 'i'
    mov r10, 2
    mov DWORD [rbx], r10d
    
    L_loop0:
    mov rbx, 1
    cmp rbx, 0
    je L_exit0
        lea rbx, QWORD [rbp + -8] ; get address of 'i_is_prime'
        mov r10, 1
        mov BYTE [rbx], r10b
        
        lea rbx, QWORD [rbp + -4] ; get address of 'j'
        mov r10, 2
        mov DWORD [rbx], r10d
        
        L_loop1:
        movsx rbx, DWORD [rbp + -4]
        movsx r10, DWORD [rbp + -12]
        cmp rbx, r10
        setl bl
        and bl, 1
        movzx rbx, bl
        cmp rbx, 0
        je L_exit1
            movsx rbx, DWORD [rbp + -12]
            movsx r10, DWORD [rbp + -4]
            mov rax, rbx
            cqo
            idiv r10
            mov rbx, rdx
            mov r10, 0
            cmp rbx, r10
            sete bl
            and bl, 1
            movzx rbx, bl
            cmp rbx, 0
            je L_exit2
                lea rbx, QWORD [rbp + -8] ; get address of 'i_is_prime'
                mov r10, 0
                mov BYTE [rbx], r10b
                
                jmp L_exit1
                
            L_exit2:
            
            movsx rbx, DWORD [rbp + -4]
            mov r10, 1
            add rbx, r10
            lea r10, QWORD [rbp + -4] ; get address of 'j'
            mov DWORD [r10], ebx
            
        jmp L_loop1
        L_exit1:
        
        movzx rbx, BYTE [rbp + -8]
        cmp rbx, 0
        je L_exit3
            sub rsp, 32 ; reserve shadow space and 1 arguments
            movsx rbx, DWORD [rbp + -12]
            mov rcx, rbx ; arg 1
            call print_num
            add rsp, 32 ; pop arguments
            mov rbx, rax ; get return value
            
            movsx rbx, DWORD [rbp + -16]
            mov r10, 1
            add rbx, r10
            lea r10, QWORD [rbp + -16] ; get address of 'num_primes'
            mov DWORD [r10], ebx
            
            movsx rbx, DWORD [rbp + -16]
            movsx r10, DWORD [REL N]
            cmp rbx, r10
            sete bl
            and bl, 1
            movzx rbx, bl
            cmp rbx, 0
            je L_exit4
                jmp L_exit0
                
            L_exit4:
            
        L_exit3:
        
        lea rbx, QWORD [rbp + -12] ; get address of 'i'
        mov r10, rbx
        movsx rbx, DWORD [rbx]
        mov r11, rbx
        inc r11
        mov DWORD [r10], r11d
        
    jmp L_loop0
    L_exit0:
    
    movsx rbx, DWORD [rbp + -12]
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    

EXTERN GetStdHandle

EXTERN WriteFile

print:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    sub rsp, 16 ; reserve stack space for 2 locals
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, -11
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
    

print_num:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    sub rsp, 32 ; reserve stack space for 5 locals
    lea rbx, QWORD [rbp + -32] ; get address of 'num_str'
    lea r10, [REL lit_str_1]
    mov QWORD [rbx], r10
    
    lea rbx, QWORD [rbp + -24] ; get address of 'idx'
    mov r10, 0
    mov DWORD [rbx], r10d
    
    L_loop5:
    movsx rbx, DWORD [rbp + 16]
    mov r10, 0
    cmp rbx, r10
    setg bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit5
        movsx rbx, DWORD [rbp + 16]
        mov r10, 10
        mov rax, rbx
        cqo
        idiv r10
        mov rbx, rdx
        lea r10, QWORD [rbp + -20] ; get address of 'digit'
        mov BYTE [r10], bl
        
        mov rbx, QWORD [rbp + -32]
        movsx r10, DWORD [rbp + -24]
        add rbx, r10
        movzx r10, BYTE [rbp + -20]
        mov r11, 48
        add r10, r11
        mov BYTE [rbx], r10b
        
        movsx rbx, DWORD [rbp + 16]
        mov r10, 10
        mov rax, rbx
        cqo
        idiv r10
        mov rbx, rax
        lea r10, QWORD [rbp + 16] ; get address of 'num'
        mov DWORD [r10], ebx
        
        movsx rbx, DWORD [rbp + -24]
        mov r10, 1
        add rbx, r10
        lea r10, QWORD [rbp + -24] ; get address of 'idx'
        mov DWORD [r10], ebx
        
    jmp L_loop5
    L_exit5:
    
    lea rbx, QWORD [rbp + -16] ; get address of 'i'
    mov r10, 0
    mov DWORD [rbx], r10d
    
    L_loop6:
    movsx rbx, DWORD [rbp + -24]
    mov r10, 2
    mov rax, rbx
    cqo
    idiv r10
    mov rbx, rax
    movsx r10, DWORD [rbp + -16]
    cmp r10, rbx
    setl r10b
    and r10b, 1
    movzx r10, r10b
    cmp r10, 0
    je L_exit6
        mov rbx, QWORD [rbp + -32]
        movsx r10, DWORD [rbp + -16]
        add rbx, r10
        movzx rbx, BYTE [rbx]
        lea r10, QWORD [rbp + -12] ; get address of 'tmp'
        mov BYTE [r10], bl
        
        mov rbx, QWORD [rbp + -32]
        movsx r10, DWORD [rbp + -24]
        add rbx, r10
        movsx r10, DWORD [rbp + -16]
        sub rbx, r10
        mov r10, 1
        sub rbx, r10
        movzx rbx, BYTE [rbx]
        mov r10, QWORD [rbp + -32]
        movsx r11, DWORD [rbp + -16]
        add r10, r11
        mov BYTE [r10], bl
        
        mov rbx, QWORD [rbp + -32]
        movsx r10, DWORD [rbp + -24]
        add rbx, r10
        movsx r10, DWORD [rbp + -16]
        sub rbx, r10
        mov r10, 1
        sub rbx, r10
        movzx r10, BYTE [rbp + -12]
        mov BYTE [rbx], r10b
        
        movsx rbx, DWORD [rbp + -16]
        mov r10, 1
        add rbx, r10
        lea r10, QWORD [rbp + -16] ; get address of 'i'
        mov DWORD [r10], ebx
        
    jmp L_loop6
    L_exit6:
    
    mov rbx, QWORD [rbp + -32]
    movsx r10, DWORD [rbp + -24]
    add rbx, r10
    mov r10, 44
    mov BYTE [rbx], r10b
    
    mov rbx, QWORD [rbp + -32]
    movsx r10, DWORD [rbp + -24]
    add rbx, r10
    mov r10, 1
    add rbx, r10
    mov r10, 32
    mov BYTE [rbx], r10b
    
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, QWORD [rbp + -32]
    mov rcx, rbx ; arg 1
    movsx rbx, DWORD [rbp + -24]
    mov r10, 2
    add rbx, r10
    mov rdx, rbx ; arg 2
    call print
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    xor rax, rax ; Default return value 0
    L_function_print_num_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data
N dq 100
lit_str_1 db "         ", 0
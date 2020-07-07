; Generated by Lang compiler

GLOBAL main

SECTION .code
EXTERN GetStdHandle

EXTERN WriteFile

EXTERN strlen


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
    mov QWORD [rsp + 32], rbx ; arg 5
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
    
    L_loop0:
    movsx rbx, DWORD [rbp + 16]
    mov r10, 0
    cmp rbx, r10
    setg bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit0
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
        
    jmp L_loop0
    L_exit0:
    
    lea rbx, QWORD [rbp + -16] ; get address of 'i'
    mov r10, 0
    mov DWORD [rbx], r10d
    
    L_loop1:
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
    je L_exit1
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
        
        lea rbx, QWORD [rbp + -16] ; get address of 'i'
        mov r10, rbx
        movsx rbx, DWORD [rbx]
        mov r11, rbx
        inc r11
        mov DWORD [r10], r11d
        
    jmp L_loop1
    L_exit1:
    
    mov rbx, QWORD [rbp + -32]
    movsx r10, DWORD [rbp + -24]
    add rbx, r10
    mov r10, 32
    mov BYTE [rbx], r10b
    
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, QWORD [rbp + -32]
    mov rcx, rbx ; arg 1
    movsx rbx, DWORD [rbp + -24]
    mov r10, 1
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
    

fizzbuzz:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    sub rsp, 16 ; reserve stack space for 3 locals
    lea rbx, QWORD [rbp + -16] ; get address of 'i'
    mov r10, 1
    mov DWORD [rbx], r10d
    
    L_loop2:
    movsx rbx, DWORD [rbp + -16]
    movsx r10, DWORD [rbp + 16]
    cmp rbx, r10
    setle bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit2
        movsx rbx, DWORD [rbp + -16]
        mov r10, 3
        mov rax, rbx
        cqo
        idiv r10
        mov rbx, rdx
        mov r10, 0
        cmp rbx, r10
        sete bl
        and bl, 1
        movzx rbx, bl
        lea r10, QWORD [rbp + -12] ; get address of 'divisible_by_3'
        mov BYTE [r10], bl
        
        movsx rbx, DWORD [rbp + -16]
        mov r10, 5
        mov rax, rbx
        cqo
        idiv r10
        mov rbx, rdx
        mov r10, 0
        cmp rbx, r10
        sete bl
        and bl, 1
        movzx rbx, bl
        lea r10, QWORD [rbp + -11] ; get address of 'divisible_by_5'
        mov BYTE [r10], bl
        
        movzx rbx, BYTE [rbp + -11]
        test rbx, rbx
        je L_land_false_3
        movzx r10, BYTE [rbp + -12]
        test r10, r10
        je L_land_false_3
        mov rbx, 1
        jmp L_land_exit_3
        L_land_false_3:
        mov rbx, 0
        L_land_exit_3:
        cmp rbx, 0
        je L_else4
            sub rsp, 32 ; reserve shadow space and 2 arguments
            lea rbx, [REL lit_str_2]
            mov rcx, rbx ; arg 1
            mov rbx, 9
            mov rdx, rbx ; arg 2
            call print
            add rsp, 32 ; pop arguments
            mov rbx, rax ; get return value
            
        jmp L_exit4
        L_else4:
            movzx rbx, BYTE [rbp + -11]
            cmp rbx, 0
            je L_else5
                sub rsp, 32 ; reserve shadow space and 2 arguments
                lea rbx, [REL lit_str_3]
                mov rcx, rbx ; arg 1
                mov rbx, 5
                mov rdx, rbx ; arg 2
                call print
                add rsp, 32 ; pop arguments
                mov rbx, rax ; get return value
                
            jmp L_exit5
            L_else5:
                movzx rbx, BYTE [rbp + -12]
                cmp rbx, 0
                je L_else6
                    sub rsp, 32 ; reserve shadow space and 2 arguments
                    lea rbx, [REL lit_str_4]
                    mov rcx, rbx ; arg 1
                    mov rbx, 5
                    mov rdx, rbx ; arg 2
                    call print
                    add rsp, 32 ; pop arguments
                    mov rbx, rax ; get return value
                    
                jmp L_exit6
                L_else6:
                    sub rsp, 32 ; reserve shadow space and 1 arguments
                    movsx rbx, DWORD [rbp + -16]
                    mov rcx, rbx ; arg 1
                    call print_num
                    add rsp, 32 ; pop arguments
                    mov rbx, rax ; get return value
                    
                L_exit6:
                
            L_exit5:
            
        L_exit4:
        
        lea rbx, QWORD [rbp + -16] ; get address of 'i'
        mov r10, rbx
        movsx rbx, DWORD [rbx]
        mov r11, rbx
        inc r11
        mov DWORD [r10], r11d
        
    jmp L_loop2
    L_exit2:
    
    xor rax, rax ; Default return value 0
    L_function_fizzbuzz_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, 20
    mov rcx, rbx ; arg 1
    call fizzbuzz
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data
STD_OUTPUT_HANDLE dq -11
lit_str_1 db "         ", 0
lit_str_2 db "fizzbuzz ", 0
lit_str_3 db "fizz ", 0
lit_str_4 db "buzz ", 0

; Generated by Lang compiler

extern GetCommandLineA
extern ExitProcess

section .code
global _start
_start:
    call GetCommandLineA
    mov rcx, rax
    call main
    mov ecx, eax
    call ExitProcess

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 32 ; reserve stack space for 5 locals
    
    ; let a: i32; a = test(true, 5);
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, 1
    mov rcx, rbx ; arg 1
    mov rbx, 5
    mov rdx, rbx ; arg 2
    call test
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -32] ; get address of 'a'
    mov DWORD [r10], ebx
    
    ; let b: i32; b = test(false, 5);
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, 0
    mov rcx, rbx ; arg 1
    mov rbx, 5
    mov rdx, rbx ; arg 2
    call test
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -28] ; get address of 'b'
    mov DWORD [r10], ebx
    
    ; if (a != 5 || b != 3)
    movsx rbx, DWORD [rbp + -32]
    mov r10, 5
    cmp rbx, r10
    setne bl
    and bl, 1
    movzx rbx, bl
    test rbx, rbx
    jne L_lor_true_0 ; short circuit '||'
    movsx r10, DWORD [rbp + -28]
    mov r11, 3
    cmp r10, r11
    setne r10b
    and r10b, 1
    movzx r10, r10b
    test r10, r10
    jne L_lor_true_0
    mov rbx, 0
    jmp L_lor_exit_0
    L_lor_true_0:
    mov rbx, 1
    L_lor_exit_0:
    cmp rbx, 0
    je L_exit1
        ; return -1
        mov rbx, -1
        mov rax, rbx ; return via rax
        jmp L_function_main_exit
        
    L_exit1:
    
    ; let common_name: i32; common_name = 3;
    lea rbx, QWORD [rbp + -24] ; get address of 'common_name'
    mov r10, 3
    mov DWORD [rbx], r10d
    
    ; let outter: i32; outter = 0;
    lea rbx, QWORD [rbp + -20] ; get address of 'outter'
    mov r10, 0
    mov DWORD [rbx], r10d
    
    ; if (true)
    mov rbx, 1
    cmp rbx, 0
    je L_exit2
        ; let common_name: i32;
        mov DWORD [rbp + -16], 0 ; zero initialize 'common_name'
        
        ; common_name = 5
        lea rbx, QWORD [rbp + -16] ; get address of 'common_name'
        mov r10, 5
        mov DWORD [rbx], r10d
        
        ; outter = common_name
        lea rbx, QWORD [rbp + -20] ; get address of 'outter'
        movsx r10, DWORD [rbp + -16]
        mov DWORD [rbx], r10d
        
    L_exit2:
    
    ; if (outter != 5)
    movsx rbx, DWORD [rbp + -20]
    mov r10, 5
    cmp rbx, r10
    setne bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit3
        ; return -1
        mov rbx, -1
        mov rax, rbx ; return via rax
        jmp L_function_main_exit
        
    L_exit3:
    
    ; return common_name
    movsx rbx, DWORD [rbp + -24]
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    

test:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov BYTE [rbp + 16], cl ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    sub rsp, 16 ; reserve stack space for 1 locals
    
    ; if (ret_arg)
    movzx rbx, BYTE [rbp + 16]
    cmp rbx, 0
    ; else
    je L_else4
        ; return arg
        movsx rbx, DWORD [rbp + 24]
        mov rax, rbx ; return via rax
        jmp L_function_test_exit
        
    jmp L_exit4
    L_else4:
        ; let arg: i32; arg = 3;
        lea rbx, QWORD [rbp + -16] ; get address of 'arg'
        mov r10, 3
        mov DWORD [rbx], r10d
        
        ; return arg
        movsx rbx, DWORD [rbp + -16]
        mov rax, rbx ; return via rax
        jmp L_function_test_exit
        
    L_exit4:
    
    xor rax, rax ; Default return value 0
    L_function_test_exit:
    mov rsp, rbp
    pop rbp
    ret
    


section .data

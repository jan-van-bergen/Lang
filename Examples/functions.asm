; Generated by Lang compiler

GLOBAL _start

extern GetCommandLineA
extern ExitProcess

SECTION .code
_start:
    call GetCommandLineA
    mov r10, rax
    xor rcx, rcx
    sub rsp, 8 * 64 ; Max 64 command line args
    mov rdx, rsp
    arg_loop_top:
    mov bl, BYTE [rax]
    test bl, bl
    jz arg_loop_exit
    cmp bl, ' '
    jne arg_loop_next
    cmp r10, rax
    je skip
    mov BYTE [rax], 0
    mov QWORD [rdx], r10
    add rdx, 8
    inc rcx
    skip:
    mov r10, rax
    inc r10
    arg_loop_next:
    inc rax
    jmp arg_loop_top
    arg_loop_exit:
    mov al, BYTE [r10]
    cmp al, ' '
    je args_done
    cmp al, 0
    je args_done
    mov QWORD [rdx], r10
    inc rcx
    args_done:
    mov rdx, rsp
    sub rsp, 32
    call main
    mov ecx, eax
    call ExitProcess

one:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    
    ; return arg + 1
    movsx rbx, DWORD [rbp + 16]
    mov r10, 1
    add rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_one_exit
    
    xor rax, rax ; Default return value 0
    L_function_one_exit:
    mov rsp, rbp
    pop rbp
    ret
    

two:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    sub rsp, 16 ; reserve stack space for 1 locals
    
    ; let local: i32; local = one(a);
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + 16]
    mov rcx, rbx ; arg 1
    call one
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -16] ; get address of 'local'
    mov DWORD [r10], ebx
    
    ; return one(b) + local
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + 24]
    mov rcx, rbx ; arg 1
    call one
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    movsx r10, DWORD [rbp + -16]
    add rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_two_exit
    
    xor rax, rax ; Default return value 0
    L_function_two_exit:
    mov rsp, rbp
    pop rbp
    ret
    

recursive:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    
    ; if (a == 0)
    movsx rbx, DWORD [rbp + 16]
    mov r10, 0
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit0
        ; return b
        movsx rbx, DWORD [rbp + 24]
        mov rax, rbx ; return via rax
        jmp L_function_recursive_exit
        
    L_exit0:
    
    ; b = b + 2
    movsx rbx, DWORD [rbp + 24]
    mov r10, 2
    add rbx, r10
    lea r10, QWORD [rbp + 24] ; get address of 'b'
    mov DWORD [r10], ebx
    
    ; a = a - 1
    movsx rbx, DWORD [rbp + 16]
    mov r10, 1
    sub rbx, r10
    lea r10, QWORD [rbp + 16] ; get address of 'a'
    mov DWORD [r10], ebx
    
    ; return recursive(a, b)
    sub rsp, 32 ; reserve shadow space and 2 arguments
    movsx rbx, DWORD [rbp + 16]
    mov rcx, rbx ; arg 1
    movsx rbx, DWORD [rbp + 24]
    mov rdx, rbx ; arg 2
    call recursive
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    jmp L_function_recursive_exit
    
    xor rax, rax ; Default return value 0
    L_function_recursive_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 2 locals
    
    ; let bla: i32; bla = two(1, 2);
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, 1
    mov rcx, rbx ; arg 1
    mov rbx, 2
    mov rdx, rbx ; arg 2
    call two
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -16] ; get address of 'bla'
    mov DWORD [r10], ebx
    
    ; let tmp: i32; tmp = one(bla);
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + -16]
    mov rcx, rbx ; arg 1
    call one
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    lea r10, QWORD [rbp + -12] ; get address of 'tmp'
    mov DWORD [r10], ebx
    
    ; return recursive(one(bla), 0)
    sub rsp, 32 ; reserve shadow space and 2 arguments
    push rcx ; preserve
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movsx rbx, DWORD [rbp + -16]
    mov rcx, rbx ; arg 1
    call one
    add rsp, 32 ; pop arguments
    pop rcx ; restore
    mov rbx, rax ; get return value
    mov rcx, rbx ; arg 1
    mov rbx, 0
    mov rdx, rbx ; arg 2
    call recursive
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

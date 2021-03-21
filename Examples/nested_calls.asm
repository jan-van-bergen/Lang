; Generated by Lang compiler

extern GetCommandLineA
extern ExitProcess

section .code
global _start
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

identity:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov BYTE [rbp + 16], cl ; push arg 0 
    
    ; return x
    movzx rbx, BYTE [rbp + 16]
    mov rax, rbx ; return via rax
    jmp L_function_identity_exit
    
    xor rax, rax ; Default return value 0
    L_function_identity_exit:
    mov rsp, rbp
    pop rbp
    ret
    

ayy:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov BYTE [rbp + 24], dl ; push arg 1 
    
    ; if (c == 'a')
    movzx rbx, BYTE [rbp + 24]
    mov r10, 97
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    ; else
    je L_else0
        ; return base + 6
        movsx rbx, DWORD [rbp + 16]
        mov r10, 6
        add rbx, r10
        mov rax, rbx ; return via rax
        jmp L_function_ayy_exit
        
    jmp L_exit0
    L_else0:
        ; return base + 4
        movsx rbx, DWORD [rbp + 16]
        mov r10, 4
        add rbx, r10
        mov rax, rbx ; return via rax
        jmp L_function_ayy_exit
        
    L_exit0:
    
    xor rax, rax ; Default return value 0
    L_function_ayy_exit:
    mov rsp, rbp
    pop rbp
    ret
    

fun:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov BYTE [rbp + 24], dl ; push arg 1 
    mov QWORD [rbp + 32], r8 ; push arg 2 
    
    ; return ayy(2, identity('b') - b) - a * c
    sub rsp, 32 ; reserve shadow space and 2 arguments
    mov rbx, 2
    mov rcx, rbx ; arg 1
    push rcx ; preserve
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, 98
    mov rcx, rbx ; arg 1
    call identity
    add rsp, 32 ; pop arguments
    pop rcx ; restore
    mov rbx, rax ; get return value
    movzx r10, BYTE [rbp + 24]
    sub rbx, r10
    mov rdx, rbx ; arg 2
    call ayy
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    movsx r10, DWORD [rbp + 16]
    mov r11, QWORD [rbp + 32]
    imul r10, r11
    sub rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_fun_exit
    
    xor rax, rax ; Default return value 0
    L_function_fun_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    
    ; return fun(3, identity(identity(1)), identity(2))
    sub rsp, 32 ; reserve shadow space and 3 arguments
    mov rbx, 3
    mov rcx, rbx ; arg 1
    push rcx ; preserve
    sub rsp, 32 ; reserve shadow space and 1 arguments
    push rcx ; preserve
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, 1
    mov rcx, rbx ; arg 1
    call identity
    add rsp, 32 ; pop arguments
    pop rcx ; restore
    mov rbx, rax ; get return value
    mov rcx, rbx ; arg 1
    call identity
    add rsp, 32 ; pop arguments
    pop rcx ; restore
    mov rbx, rax ; get return value
    mov rdx, rbx ; arg 2
    push rcx ; preserve
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, 2
    mov rcx, rbx ; arg 1
    call identity
    add rsp, 32 ; pop arguments
    pop rcx ; restore
    mov rbx, rax ; get return value
    mov r8, rbx ; arg 3
    call fun
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


section .data

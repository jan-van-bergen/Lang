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

calling_convention:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov DWORD [rbp + 16], ecx ; push arg 0 
    mov DWORD [rbp + 24], edx ; push arg 1 
    mov DWORD [rbp + 32], r8d ; push arg 2 
    mov DWORD [rbp + 40], r9d ; push arg 3 
    
    ; return rcx + rdx + r8 + r9 + stack1 - stack0
    movsx rbx, DWORD [rbp + 16]
    movsx r10, DWORD [rbp + 24]
    add rbx, r10
    movsx r10, DWORD [rbp + 32]
    add rbx, r10
    movsx r10, DWORD [rbp + 40]
    add rbx, r10
    movsx r10, DWORD [rbp + 52]
    add rbx, r10
    movsx r10, DWORD [rbp + 48]
    sub rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_calling_convention_exit
    
    xor rax, rax ; Default return value 0
    L_function_calling_convention_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    
    ; return calling_convention(1, 2, 3, 4, 5, 6)
    sub rsp, 48 ; reserve shadow space and 6 arguments
    mov rbx, 1
    mov rcx, rbx ; arg 1
    mov rbx, 2
    mov rdx, rbx ; arg 2
    mov rbx, 3
    mov r8, rbx ; arg 3
    mov rbx, 4
    mov r9, rbx ; arg 4
    mov rbx, 5
    mov DWORD [rsp + 32], ebx ; arg 5
    mov rbx, 6
    mov DWORD [rsp + 36], ebx ; arg 6
    call calling_convention
    add rsp, 48 ; pop arguments
    mov rbx, rax ; get return value
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

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

bla:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    
    ; *ptr = 4321
    mov rbx, QWORD [rbp + 16]
    mov r10, 4321
    mov DWORD [rbx], r10d
    
    xor rax, rax ; Default return value 0
    L_function_bla_exit:
    mov rsp, rbp
    pop rbp
    ret
    

deref:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov QWORD [rbp + 16], rcx ; push arg 0 
    sub rsp, 16 ; reserve stack space for 1 locals
    
    ; let local: i32;
    mov DWORD [rbp + -16], 0 ; zero initialize 'local'
    
    ; *ptr = 21
    mov rbx, QWORD [rbp + 16]
    mov r10, 21
    mov DWORD [rbx], r10d
    
    ; local = 1234
    lea rbx, QWORD [rbp + -16] ; get address of 'local'
    mov r10, 1234
    mov DWORD [rbx], r10d
    
    ; bla(&local)
    sub rsp, 32 ; reserve shadow space and 1 arguments
    lea rbx, QWORD [rbp + -16] ; get address of 'local'
    mov rcx, rbx ; arg 1
    call bla
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    xor rax, rax ; Default return value 0
    L_function_deref_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 32 ; reserve stack space for 3 locals
    
    ; let a: i32; a = 42;
    lea rbx, QWORD [rbp + -32] ; get address of 'a'
    mov r10, 42
    mov DWORD [rbx], r10d
    
    ; let p: i32*;
    mov QWORD [rbp + -24], 0 ; zero initialize 'p'
    
    ; p = &a
    lea rbx, QWORD [rbp + -32] ; get address of 'a'
    lea r10, QWORD [rbp + -24] ; get address of 'p'
    mov QWORD [r10], rbx
    
    ; let p2: i32*;
    mov QWORD [rbp + -16], 0 ; zero initialize 'p2'
    
    ; p2 = p
    lea rbx, QWORD [rbp + -16] ; get address of 'p2'
    mov r10, QWORD [rbp + -24]
    mov QWORD [rbx], r10
    
    ; deref(p2)
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov rbx, QWORD [rbp + -16]
    mov rcx, rbx ; arg 1
    call deref
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    ; return a
    movsx rbx, DWORD [rbp + -32]
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

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

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 48 ; reserve stack space for 2 locals
    
    ; let test: Test;
    lea rdi, QWORD [rbp + -48] ; zero initialize 'test'
    xor rax, rax
    mov ecx, 24
    rep stosb
    
    ; test.a = 3
    lea rbx, QWORD [rbp + -48] ; get address of 'test'
    add rbx, 0 ; member offset 'a'
    mov r10, 3
    mov BYTE [rbx], r10b
    
    ; test.n.b = 4
    lea rbx, QWORD [rbp + -48] ; get address of 'test'
    add rbx, 8 ; member offset 'n'
    add rbx, 0 ; member offset 'b'
    mov r10, 4
    mov DWORD [rbx], r10d
    
    ; test.n.c = 5
    lea rbx, QWORD [rbp + -48] ; get address of 'test'
    add rbx, 8 ; member offset 'n'
    add rbx, 8 ; member offset 'c'
    mov r10, 5
    mov QWORD [rbx], r10
    
    ; let nest: Nested; nest = test.n;
    lea rbx, QWORD [rbp + -48] ; get address of 'test'
    add rbx, 8 ; member offset 'n'
    lea r10, QWORD [rbp + -24] ; get address of 'nest'
    mov rdi, r10
    mov rsi, rbx
    mov ecx, 16
    rep movsb
    
    ; return nest.b
    lea rbx, QWORD [rbp + -24] ; get address of 'nest'
    add rbx, 0 ; member offset 'b'
    movsx rbx, DWORD [rbx]
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

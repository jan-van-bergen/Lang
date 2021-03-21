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

; let globla: Bla;

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    
    ; globla.b = 3
    lea rbx, QWORD [REL globla] ; get address of 'globla'
    add rbx, 0 ; member offset 'b'
    mov r10, 3
    mov DWORD [rbx], r10d
    
    ; globla.l = 5
    lea rbx, QWORD [REL globla] ; get address of 'globla'
    add rbx, 8 ; member offset 'l'
    mov r10, 5
    mov QWORD [rbx], r10
    
    ; globla.a = cast(i8) 7
    lea rbx, QWORD [REL globla] ; get address of 'globla'
    add rbx, 16 ; member offset 'a'
    mov r10, 7
    mov BYTE [rbx], r10b
    
    ; return globla.b + globla.l + globla.a
    lea rbx, QWORD [REL globla] ; get address of 'globla'
    add rbx, 0 ; member offset 'b'
    movsx rbx, DWORD [rbx]
    lea r10, QWORD [REL globla] ; get address of 'globla'
    add r10, 8 ; member offset 'l'
    mov r10, QWORD [r10]
    add rbx, r10
    lea r10, QWORD [REL globla] ; get address of 'globla'
    add r10, 16 ; member offset 'a'
    movsx r10, BYTE [r10]
    add rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


section .data
globla dq 0, 0, 0

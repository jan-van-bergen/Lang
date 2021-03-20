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
    sub rsp, 16 ; reserve stack space for 3 locals
    
    ; let a: u32;
    mov DWORD [rbp + -16], 0 ; zero initialize 'a'
    
    ; let b: u32;
    mov DWORD [rbp + -12], 0 ; zero initialize 'b'
    
    ; let c: u32;
    mov DWORD [rbp + -8], 0 ; zero initialize 'c'
    
    ; a = 4278190080
    lea rbx, QWORD [rbp + -16] ; get address of 'a'
    mov r10, 4278190080
    mov DWORD [rbx], r10d
    
    ; b = 255
    lea rbx, QWORD [rbp + -12] ; get address of 'b'
    mov r10, 255
    mov DWORD [rbx], r10d
    
    ; c = a | b
    mov ebx, DWORD [rbp + -16]
    mov r10d, DWORD [rbp + -12]
    or rbx, r10
    lea r10, QWORD [rbp + -8] ; get address of 'c'
    mov DWORD [r10], ebx
    
    ; assert(c == 4278190335)
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov ebx, DWORD [rbp + -8]
    mov r10, 4278190335
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    ; c = ~a & b
    mov ebx, DWORD [rbp + -16]
    not rbx
    mov r10d, DWORD [rbp + -12]
    and rbx, r10
    lea r10, QWORD [rbp + -8] ; get address of 'c'
    mov DWORD [r10], ebx
    
    ; assert(c == 255)
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov ebx, DWORD [rbp + -8]
    mov r10, 255
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    ; a = 16776960
    lea rbx, QWORD [rbp + -16] ; get address of 'a'
    mov r10, 16776960
    mov DWORD [rbx], r10d
    
    ; b = b << 8
    mov ebx, DWORD [rbp + -12]
    mov r10, 8
    mov rcx, r10
    shl rbx, cl
    lea r10, QWORD [rbp + -12] ; get address of 'b'
    mov DWORD [r10], ebx
    
    ; c = a ^ b | c
    mov ebx, DWORD [rbp + -16]
    mov r10d, DWORD [rbp + -12]
    xor rbx, r10
    mov r10d, DWORD [rbp + -8]
    or rbx, r10
    lea r10, QWORD [rbp + -8] ; get address of 'c'
    mov DWORD [r10], ebx
    
    ; assert(c == 16711935)
    sub rsp, 32 ; reserve shadow space and 1 arguments
    mov ebx, DWORD [rbp + -8]
    mov r10, 16711935
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    ; return c >> 16
    mov ebx, DWORD [rbp + -8]
    mov r10, 16
    mov rcx, r10
    shr rbx, cl
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    

EXTERN ExitProcess

assert:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    mov BYTE [rbp + 16], cl ; push arg 0 
    
    ; if (!expression)
    movzx rbx, BYTE [rbp + 16]
    xor rbx, -1
    and rbx, 1
    cmp rbx, 0
    je L_exit1
        ; ExitProcess(1)
        sub rsp, 32 ; reserve shadow space and 1 arguments
        mov rbx, 1
        mov rcx, rbx ; arg 1
        call ExitProcess
        add rsp, 32 ; pop arguments
        mov rbx, rax ; get return value
        
    L_exit1:
    
    xor rax, rax ; Default return value 0
    L_function_assert_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

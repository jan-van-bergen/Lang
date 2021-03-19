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
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 1 locals
    
    ; let a: bool;
    mov BYTE [rbp + -16], 0 ; zero initialize 'a'
    
    ; a = true || false
    mov rbx, 1
    test rbx, rbx
    jne L_lor_true_2 ; short circuit '||'
    mov r10, 0
    test r10, r10
    jne L_lor_true_2
    mov rbx, 0
    jmp L_lor_exit_2
    L_lor_true_2:
    mov rbx, 1
    L_lor_exit_2:
    lea r10, QWORD [rbp + -16] ; get address of 'a'
    mov BYTE [r10], bl
    
    ; assert(a == true)
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movzx rbx, BYTE [rbp + -16]
    mov r10, 1
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    ; a = false && true
    mov rbx, 0
    test rbx, rbx
    je L_land_false_3 ; short circuit '&&'
    mov r10, 1
    test r10, r10
    je L_land_false_3
    mov rbx, 1
    jmp L_land_exit_3
    L_land_false_3:
    mov rbx, 0
    L_land_exit_3:
    lea r10, QWORD [rbp + -16] ; get address of 'a'
    mov BYTE [r10], bl
    
    ; assert(a == false)
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movzx rbx, BYTE [rbp + -16]
    mov r10, 0
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    ; a = a || !a
    movzx rbx, BYTE [rbp + -16]
    test rbx, rbx
    jne L_lor_true_4 ; short circuit '||'
    movzx r10, BYTE [rbp + -16]
    xor r10, -1
    and r10, 1
    test r10, r10
    jne L_lor_true_4
    mov rbx, 0
    jmp L_lor_exit_4
    L_lor_true_4:
    mov rbx, 1
    L_lor_exit_4:
    lea r10, QWORD [rbp + -16] ; get address of 'a'
    mov BYTE [r10], bl
    
    ; assert(a == true)
    sub rsp, 32 ; reserve shadow space and 1 arguments
    movzx rbx, BYTE [rbp + -16]
    mov r10, 1
    cmp rbx, r10
    sete bl
    and bl, 1
    movzx rbx, bl
    mov rcx, rbx ; arg 1
    call assert
    add rsp, 32 ; pop arguments
    mov rbx, rax ; get return value
    
    ; return 0
    mov rbx, 0
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

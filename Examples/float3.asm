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

fun:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    movss DWORD [rbp + 16], xmm0 ; push arg 0 
    movsd QWORD [rbp + 24], xmm1 ; push arg 1 
    
    ; return cast(f64) f + d
    movss xmm5, DWORD [rbp + 16]
    cvtss2sd xmm5, xmm5
    movsd xmm6, QWORD [rbp + 24]
    addsd xmm5, xmm6
    movsd xmm0, xmm5 ; return via xmm0
    jmp L_function_fun_exit
    
    xor rax, rax ; Default return value 0
    L_function_fun_exit:
    mov rsp, rbp
    pop rbp
    ret
    

function:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    movsd QWORD [rbp + 16], xmm0 ; push arg 0 
    movss DWORD [rbp + 24], xmm1 ; push arg 1 
    
    ; return cast(f32) fun(1.500000f, d) + fun(f, 1.250000)
    sub rsp, 32 ; reserve shadow space and 2 arguments
    movss xmm4, DWORD [REL lit_flt_0]
    movss xmm0, xmm4 ; arg 1
    movsd xmm5, QWORD [rbp + 16]
    movsd xmm1, xmm5 ; arg 2
    call fun
    add rsp, 32 ; pop arguments
    movsd xmm4, xmm0 ; get return value (f64)
    sub rsp, 32 ; reserve shadow space and 2 arguments
    movss xmm6, DWORD [rbp + 24]
    movss xmm0, xmm6 ; arg 1
    movsd xmm5, QWORD [REL lit_flt_1]
    movsd xmm1, xmm5 ; arg 2
    call fun
    add rsp, 32 ; pop arguments
    movsd xmm5, xmm0 ; get return value (f64)
    addsd xmm4, xmm5
    cvtsd2ss xmm4, xmm4
    movss xmm0, xmm4 ; return via xmm0
    jmp L_function_function_exit
    
    xor rax, rax ; Default return value 0
    L_function_function_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 2 locals
    
    ; let d: f64; d = 4.500000;
    lea rbx, QWORD [rbp + -16] ; get address of 'd'
    movsd xmm4, QWORD [REL lit_flt_2]
    movsd QWORD [rbx], xmm4
    
    ; let f: f32; f = 3.250000f;
    lea rbx, QWORD [rbp + -8] ; get address of 'f'
    movss xmm4, DWORD [REL lit_flt_3]
    movss DWORD [rbx], xmm4
    
    ; return cast(i32) function(d, f)
    sub rsp, 32 ; reserve shadow space and 2 arguments
    movsd xmm5, QWORD [rbp + -16]
    movsd xmm0, xmm5 ; arg 1
    movss xmm5, DWORD [rbp + -8]
    movss xmm1, xmm5 ; arg 2
    call function
    add rsp, 32 ; pop arguments
    movss xmm4, xmm0 ; get return value (f32)
    cvttss2si rbx, xmm4
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


section .data
lit_flt_0 dq 3fc00000h ; 1.500000f
lit_flt_1 dq 3ff4000000000000h ; 1.250000
lit_flt_2 dq 4012000000000000h ; 4.500000
lit_flt_3 dq 40500000h ; 3.250000f
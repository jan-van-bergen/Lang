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

min:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    movss DWORD [rbp + 16], xmm0 ; push arg 0 
    movss DWORD [rbp + 24], xmm1 ; push arg 1 
    
    ; if (x < y)
    movss xmm5, DWORD [rbp + 16]
    movss xmm6, DWORD [rbp + 24]
    comiss xmm5, xmm6
    setl bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    ; else
    je L_else0
        ; return x
        movss xmm5, DWORD [rbp + 16]
        movss xmm0, xmm5 ; return via xmm0
        jmp L_function_min_exit
        
    jmp L_exit0
    L_else0:
        ; return y
        movss xmm5, DWORD [rbp + 24]
        movss xmm0, xmm5 ; return via xmm0
        jmp L_function_min_exit
        
    L_exit0:
    
    xor rax, rax ; Default return value 0
    L_function_min_exit:
    mov rsp, rbp
    pop rbp
    ret
    

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 16 ; reserve stack space for 3 locals
    
    ; let a: f32; a = 4.250000f;
    lea rbx, QWORD [rbp + -16] ; get address of 'a'
    movss xmm4, DWORD [REL lit_flt_0]
    movss DWORD [rbx], xmm4
    
    ; let b: f32; b = 2.740000f;
    lea rbx, QWORD [rbp + -12] ; get address of 'b'
    movss xmm4, DWORD [REL lit_flt_1]
    movss DWORD [rbx], xmm4
    
    ; let m: f32; m = min(a, b);
    sub rsp, 32 ; reserve shadow space and 2 arguments
    movss xmm5, DWORD [rbp + -16]
    movss xmm0, xmm5 ; arg 1
    movss xmm5, DWORD [rbp + -12]
    movss xmm1, xmm5 ; arg 2
    call min
    add rsp, 32 ; pop arguments
    movss xmm4, xmm0 ; get return value (f32)
    lea rbx, QWORD [rbp + -8] ; get address of 'm'
    movss DWORD [rbx], xmm4
    
    ; return cast(i32) m + 0.500000f
    movss xmm5, DWORD [rbp + -8]
    movss xmm4, DWORD [REL lit_flt_2]
    addss xmm5, xmm4
    cvttss2si rbx, xmm5
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data
lit_flt_0 dq 40880000h ; 4.250000f
lit_flt_1 dq 402f5c29h ; 2.740000f
lit_flt_2 dq 3f000000h ; 0.500000f

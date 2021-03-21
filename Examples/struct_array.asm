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

main:
    push rbp ; save RBP
    mov rbp, rsp ; stack frame
    sub rsp, 656 ; reserve stack space for 2 locals
    
    ; let points: Point[80];
    mov QWORD [rbp + -656], 0 ; zero initialize 'points'
    
    ; let i: u32; i = 0;
    lea rbx, QWORD [rbp + -16] ; get address of 'i'
    mov r10, 0
    mov DWORD [rbx], r10d
    
    ; while (i < 80)
    L_loop0:
    mov ebx, DWORD [rbp + -16]
    mov r10, 80
    cmp rbx, r10
    setl bl
    and bl, 1
    movzx rbx, bl
    cmp rbx, 0
    je L_exit0
        ; points[i].x = i * 2
        lea rbx, QWORD [rbp + -656] ; get address of 'points'
        mov r10d, DWORD [rbp + -16]
        imul r10, 8
        add rbx, r10
        add rbx, 0 ; member offset 'x'
        mov r10d, DWORD [rbp + -16]
        mov r11, 2
        imul r10, r11
        mov DWORD [rbx], r10d
        
        ; points[i].y = i * 2 + 1
        lea rbx, QWORD [rbp + -656] ; get address of 'points'
        mov r10d, DWORD [rbp + -16]
        imul r10, 8
        add rbx, r10
        add rbx, 4 ; member offset 'y'
        mov r10d, DWORD [rbp + -16]
        mov r11, 2
        imul r10, r11
        mov r11, 1
        add r10, r11
        mov DWORD [rbx], r10d
        
        ; i++
        lea rbx, QWORD [rbp + -16] ; get address of 'i'
        mov r10, rbx
        mov ebx, DWORD [rbx]
        mov r11, rbx
        inc r11
        mov DWORD [r10], r11d
        
    jmp L_loop0
    L_exit0:
    
    ; return points[40].x + points[21].y
    lea rbx, QWORD [rbp + -656] ; get address of 'points'
    mov r10, 40
    imul r10, 8
    add rbx, r10
    add rbx, 0 ; member offset 'x'
    movsx rbx, DWORD [rbx]
    lea r10, QWORD [rbp + -656] ; get address of 'points'
    mov r11, 21
    imul r11, 8
    add r10, r11
    add r10, 4 ; member offset 'y'
    movsx r10, DWORD [r10]
    add rbx, r10
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


section .data
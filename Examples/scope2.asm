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
    
    ; return sizeof(StructName)
    mov rbx, 32 ; sizeof 'StructName'
    mov rax, rbx ; return via rax
    jmp L_function_main_exit
    
    xor rax, rax ; Default return value 0
    L_function_main_exit:
    mov rsp, rbp
    pop rbp
    ret
    


SECTION .data

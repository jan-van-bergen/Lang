EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC

GLOBAL main

SECTION .code
deref:
    push rcx
    mov rax, QWORD [rsp + 0 * 8]
    mov QWORD [rax], 21 ; set ptr ptr
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
main:
    sub rsp, 8
    mov QWORD [rsp + 0 * 8], 42 ; set a
    sub rsp, 32 + 0
    lea rax, QWORD [RSP + 4 * 8] ; addrof a
    mov rcx, rax ; arg 0
    call deref
    add rsp, 32 + 0
    mov rax, QWORD [rsp + 0 * 8] ; get a
    add rsp, 8
    ret
    ; Default return
    add rsp, 8
    xor rax, rax
    ret
    
SECTION .data

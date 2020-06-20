EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC
.data
hello_msg db "Hello world", 0
info_msg  db "Info", 0
.code
main PROC
	push rbp ; save frame pointer
	mov rbp, rsp ; fix stack pointer
	sub rsp, 8 * (4 + 2) ; allocate shadow register area + 2 QWORDs for stack alignment
	; Get a window handle
	call GetForegroundWindow
	mov rcx, rax
	lea rdx, hello_msg
	lea r8,  info_msg
	mov r9, 0 ; MB_OK
	and rsp, not 8 ; align stack to 16 bytes prior to API call
	call MessageBoxA
	; epilog. restore stack pointer
	mov rsp, rbp
	pop rbp
	mov rax, 3
	ret 
main ENDP
END
EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC
.data
hello_msg db "Hello world", 0
info_msg  db "Info", 0
.code
main PROC
mov eax, 3
mov eax, 2
mov ebx, eax
mov eax, 4
add eax, ebx
ret
main ENDP
END
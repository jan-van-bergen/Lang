EXTERN MessageBoxA: PROC
EXTERN GetForegroundWindow: PROC
.data
hello_msg db "Hello world", 0
info_msg  db "Info", 0
a dd 0
b dd 0
c dd 0
d dd 0
.code
main PROC
mov eax, 1
mov a, eax
mov eax, 0
mov b, eax
L_loop0:
mov eax, a
mov ebx, 5
cmp eax, ebx
jg L1
mov eax, 1
jmp L2
L1:
mov eax, 0
L2:
cmp eax, 0
je L_exit0
mov eax, a
mov ebx, 1
add eax, ebx
mov a, eax
mov eax, a
mov ebx, 2
cmp eax, ebx
jne L3
mov eax, 1
jmp L4
L3:
mov eax, 0
L4:
cmp eax, 0
je L_exit5
jmp L_loop0
L_exit5:
mov eax, a
mov ebx, 6
cmp eax, ebx
jne L6
mov eax, 1
jmp L7
L6:
mov eax, 0
L7:
cmp eax, 0
je L_exit8
jmp L_exit0
L_exit8:
mov eax, b
mov ebx, a
add eax, ebx
mov b, eax
jmp L_loop0
L_exit0:
mov eax, b
ret
main ENDP
END
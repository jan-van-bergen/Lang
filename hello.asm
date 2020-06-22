                                                ; Console Message, 64 bit. V1.03
NULL              EQU 0                         ; Constants
STD_OUTPUT_HANDLE EQU -11

extern GetStdHandle                             ; Import external symbols
extern WriteFile                                ; Windows API functions, not decorated
extern ExitProcess

global main                                    ; Export symbols. The entry point

section .data                                   ; Initialized data segment
 Message        db "Console Message 64", 0Dh, 0Ah
 MessageLength  EQU $-Message                   ; Address of this line ($) - address of Message

section .bss                                    ; Uninitialized data segment
alignb 8
 StandardHandle resq 1
 Written        resq 1

section .code                                   ; Code segment
main:
 sub   RSP, 8                                   ; Align the stack to a multiple of 16 bytes

 sub   RSP, 32                                  ; 32 bytes of shadow space
 mov   ECX, STD_OUTPUT_HANDLE
 call  GetStdHandle
 mov   qword [REL StandardHandle], RAX
 add   RSP, 32                                  ; Remove the 32 bytes

 sub   RSP, 32 + 8 + 8                          ; Shadow space + 5th parameter + align stack
                                                ; to a multiple of 16 bytes
 mov   RCX, qword [REL StandardHandle]          ; 1st parameter
 lea   RDX, [REL Message]                       ; 2nd parameter
 mov   R8, MessageLength                        ; 3rd parameter
 lea   R9, [REL Written]                        ; 4th parameter
 mov   qword [RSP + 4 * 8], NULL                ; 5th parameter
 call  WriteFile                                ; Output can be redirect to a file using >
 add   RSP, 48                                  ; Remove the 48 bytes

 xor   ECX, ECX
 call  ExitProcess
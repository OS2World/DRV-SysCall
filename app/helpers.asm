.686p
.MODEL FLAT,SYSCALL

.CODE
ReadCR0 PROC PUBLIC
   mov eax,cr0
   ret
ReadCR0 ENDP

ReadCR4 PROC PUBLIC
   mov eax,cr4
   ret
ReadCR4 ENDP

; NOTE: a hidden parameter will be passed from C-Code as the leftmost
; parameter to contain the address of the 64-bit result variable
ReadMSR PROC PUBLIC USES ebx,resultAddr:DWORD,regNo:DWORD
   mov ebx,resultAddr
   mov ecx,regNo
   rdmsr
   mov [ebx],eax
   mov [ebx+4],edx
   ret
ReadMSR ENDP


END

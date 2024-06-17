;*****************************************************************************;
;*                                                                           *;
;* COPYRIGHT    Copyright (C) 2017 Lars Erdmann                              *;
;*                                                                           *;
;*    The following OS/2 source code is provided to you solely for           *;
;*    the purpose of assisting you in your development of OS/2 device        *;
;*    drivers. This Copyright statement may not be removed.                  *;
;*    All Rights Reserved                                                    *;
;*                                                                           *;
;*****************************************************************************;
.686p
.XMM
.MODEL FLAT,SYSCALL

OPTION OLDSTRUCTS
include infoseg.inc

EXTRN SYSCALL KernThunkStackTo32:PROC
EXTRN SYSCALL KernThunkStackTo16:PROC

MAJOR_TRACE_CODE EQU 0Ah

.DATA
PUBLIC FunctionTable,gStackBase,gGDTInfo,gSavStackPtr
FunctionTable DD 256 DUP(0)
gStackBase     DD 0
ALIGN 4
gGDTInfo       DF 0
ALIGN 4
gSavStackPtr   DF 0

.CODE
FunctionWrapper PROC PUBLIC, FunctionCode:DWORD,Arg1:DWORD,Arg2:DWORD
   push ss		               	; save default OS/2 flat Ring3 stack selector
   push cs                       ; save default OS/2 flat Ring3 code selector
   mov  ecx,esp                  ; for sysexit: save current Ring3 stack selector in ecx
   mov  edx,OFFSET @F            ; for sysexit: save return address in edx
   sysenter
@@:
   push OFFSET @F
   retf                          ; restore default OS/2 flat Ring3 code selector
                                 ; do far ret wich pops offset and selector
                                 ; off the stack and takes us to the next "@@" label
@@:
   pop ss			               ; restore default OS/2 flat Ring3 stack selector
   ret
FunctionWrapper ENDP

Ring0EntryPoint PROC PUBLIC      ; this is the function that the "sysenter" instruction will directly jump to
   push ebp
   push ebx
   push ecx
   push edx
   push esi
   push edi
   push ds
   push es

   mov eax,ss
   mov ds,eax
   mov es,eax

   mov DWORD PTR [gSavStackPtr],esp
   mov WORD PTR [gSavStackPtr+4],ss

   sgdt [gGDTInfo]
   mov ebx,DWORD PTR [gGDTInfo+2]
   str ax
   and eax,0FFF8h                ; use TR to directly index into GDT
   add ebx,eax                   ; address of GDT descriptor entry of current TSS now in EBX
   mov eax,DWORD PTR [ebx]       ; read TSS descriptor, lower DWORD
   mov edx,DWORD PTR [ebx+4]     ; read TSS descriptor, upper DWORD
   shrd eax,edx,8
   shr edx,24
   shrd eax,edx,8                ; base address of TSS now in EAX
   lss esp,FWORD PTR [eax+4]     ; switch to Ring-0 SS:ESP as defined in TSS
   mov ebp,esp                   ; ebp needs to be set in relation to esp for stack thunking to work
                                 ; "sysenter" enters with interrupts disabled
                                 ; we have to reenable them, otherwise we get a
   sti                           ; SYS1924 on return from this routine

   call KernThunkStackTo32	      ; need to thunk from 16-bit to 32-bit stack
   mov  esi,ecx
   add  esi,8                    ; skip over "push ss,push cs" in function wrapper
   xor  ebx,ebx                  ; preset return code in case we cannot execute the user function
   mov  eax,[esi+8]              ; set function code (skip "push ebp" and return address in FunctionWrapper)
   cmp  eax,256                  ; limit to the number of functions in the "FunctionTable"
   jae  short @F
   mov  edx,FunctionTable[eax*4]
   or   edx,edx
   jz   short @F
   push DWORD PTR [esi+16]       ; push arg2 from Ring-3 stack to Ring-0 stack
   push DWORD PTR [esi+12]       ; push arg1 from Ring-3 stack to Ring-0 stack
   call FunctionTable[eax*4]
   add  esp,8
   mov  ebx,eax
@@:
   call KernThunkStackTo16       ; thunk back to the 16-bit stack, need to do this for consistency reasons (OS/2 saves ss and esp internally)
   mov eax,ebx

   lss esp,FWORD PTR [gSavStackPtr] ;swap back to our own Ring-0 stack
   pop es
   pop ds
   pop edi
   pop esi
   pop edx                       ; restore Ring-3 return addr   on sysexit
   pop ecx                       ; restore Ring-3 stack pointer on sysexit
   pop ebx
   pop ebp
   sysexit                       ; return to "FunctionWrapper"
; returning from here will set the 32-bit stack selector to the DPL=3 data descriptor
; that we established in the GDT
Ring0EntryPoint ENDP

DGROUP GROUP DATA32             ; to combine with the Watcom data segment

END


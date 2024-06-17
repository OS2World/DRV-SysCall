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
OPTION OLDSTRUCTS    ; to properly handle deprecated structure defs in infoseg.inc
OPTION LANGUAGE:C
include devhlp.inc
include infoseg.inc

.286p
EXTRN PASCAL DOS32FLATDS:ABS
EXTRN PASCAL DOS32FLATCS:ABS
; for some reason I don't know, you should NEVER place data externals into a code segment
; because if you do ALP will prepend the memory reference with a CS prefix !
; that's why we place the external data reference here. Maybe that's a bug in ALP.EXE.
EXTRN C      Device_Help:DWORD
EXTRN C      gSelArray:WORD

.586p
; these externals have to go here, if we would place them AFTER the .288p and BEFORE the .586p
; they would only resolve to 16-bit offsets instead of 32-bit offsets !
EXTRN SYSCALL KernRASSysTrace:PROC
EXTRN SYSCALL KernThunkStackTo16:PROC
EXTRN SYSCALL KernThunkStackTo32:PROC
EXTRN SYSCALL KernLinToPageList:PROC
EXTRN C       KernSISData:InfoSegGDT

pause MACRO
   db 073h,090h
ENDM

MAJOR_TRACE_CODE EQU 0Ah

IA32_APIC_BASE   EQU 01bh
IPI_VECTOR       EQU 058h

OPSIZEBIT       EQU 400000h

SEP                     EQU 800h

IA32_SYSENTER_CS        EQU 174h
IA32_SYSENTER_ESP       EQU 175h
IA32_SYSENTER_EIP       EQU 176h


IF_FLAG                 EQU 200h

ICR_LOW                 EQU 300h
ICR_HIGH                EQU 310h
EOI                     EQU 0B0h

ALL_INCLUDING_SELF      EQU 00080000h
ASSERT                  EQU 00004000h
PENDING                 EQU 00001000h




_HEADER     segment dword public use16 'DATA'
_HEADER     ends

CONST       segment dword public use16 'DATA'
CONST       ends

CONST2      segment dword public use16 'DATA'
CONST2      ends

_DATA       segment dword public use16 'DATA'
PUBLIC C gdtSave,idtSave,gAPICLinAddr,gGlobalInfoSeg,gEntryPoint,gIPIVector,gStackAddr,gStackSize
gdtSave        DF 0
ALIGN 4
idtSave        DF 0
ALIGN 4
gAPICLinAddr   DD 0
gGlobalInfoSeg DD 0
gEntryPoint    DD 0
gIPIVector     DD IPI_VECTOR
ALIGN 4
gStackAddr     DD OFFSET D32GROUP:gRing0Stack
ALIGN 4
gStackSize     DD initialESP - OFFSET D32GROUP:gRing0Stack
_DATA       ends

_BSS        segment dword public use16 'BSS'
_BSS        ends

_TEXT       segment dword public use16 'CODE'
ASSUME DS:DGROUP

Trace PROC NEAR C PUBLIC,MinorCode:WORD,Buffer:DWORD,BufferLength:WORD
; we are saving ALL registers as we want to use
; this function from assembler code that might
; potentially need to preserve any register across this function call
    pusha
    push ds
    push es
    mov  ax,DGROUP
    mov  es,ax
    les bx,es:[gGlobalInfoSeg]
    add bx,SIS_mec_table
    mov dl,(080h SHR (MAJOR_TRACE_CODE MOD 8))
    test es:[bx+(MAJOR_TRACE_CODE/8)],dl
    jz short @F

    mov  ax,DGROUP
    mov  es,ax
    mov  ax,MAJOR_TRACE_CODE
    mov  bx,BufferLength
    lds  si,Buffer
    mov  cx,MinorCode
    mov  dl,DevHlp_RAS
    call DWORD PTR es:[Device_Help]

@@:
    pop es
    pop ds
    popa
    ret
Trace ENDP

isIntel PROC NEAR C PUBLIC USES ebx
; INTEL or AMD ? eax=0;cpuid;ebx=756e6547h,edx=49656e69h,ecx=6c65746eh for INTEL    	
    xor eax,eax
    cpuid

    xor eax,eax
    cmp ebx,0756e6547h
    jne short @F
    cmp edx,049656e69h
    jne short @F
    cmp ecx,06c65746eh
    jne short @F
    inc eax
@@:
    push ecx
    push edx
    push ebx
    push 3*SIZE DWORD
    push ss
    mov  ax,sp
    add  ax,4
    push ax
    push 4
    call Trace
    add sp,8+3*SIZE DWORD

    ret
isIntel ENDP

getProcessorInfo PROC NEAR C PUBLIC USES ebx
    xor eax,eax
    inc eax
    cpuid
    shld edx,eax,16
    ret
getProcessorInfo ENDP

getAPICLinAddr PROC NEAR C PUBLIC USES esi edi
LOCAL physAddr:DWORD
    mov ecx,IA32_APIC_BASE
    rdmsr
    and eax,0FFFFF000h  ; phys addr in eax
    mov [physAddr],eax

    push eax
    push SIZE DWORD
    push ss
    mov  ax,sp
    add  ax,4
    push ax
    push 2
    call Trace
    add sp,8+SIZE DWORD

    lea ax,[physAddr]
    movzx esi,ax
    mov   ax,ss
    mov dl,DevHlp_VirtToLin
    call DWORD PTR [Device_Help]
    jc short @F
    mov edi,eax
    mov ecx,4096
    mov eax,010h		; VMDHA_PHYS
    mov dl,DevHlp_VMAlloc
    call DWORD PTR [Device_Help]
    jc short @F
    jmp short end
@@:
    xor eax,eax
end:
    shld edx,eax,16
    ret
getAPICLinAddr ENDP

genIPI PROC NEAR C PUBLIC USES ebx ds,APICLinAddr:DWORD,UserRoutine:DWORD
    mov eax,[UserRoutine]
    mov [gEntryPoint],eax
    mov ebx,[APICLinAddr]
    mov ecx,[gIPIVector]

    mov ax,DOS32FLATDS
    mov ds,ax
ASSUME DS:FLAT
@@:
    mov eax,[ebx+ICR_LOW]
    test ax,PENDING
    jz  short @F
    pause
    jmp short @B
@@:
    pushf
    cli
    mov eax,0FF000000h              ; destination = 0xFF = all APICs (necessary for AMD CPUs, "don't care" for Intel CPUs)
    mov [ebx+ICR_HIGH],eax          ; address ICR, high DWORD
    add ecx,ALL_INCLUDING_SELF + ASSERT ; IPI to "All Including Self", edge triggered, fixed, asserted, physical destination
    mov [ebx+ICR_LOW],ecx           ; address ICR, low DWORD
    pop ax
    test ax,0200h
    jz short @F
    sti
@@:
    mov eax,[ebx+ICR_LOW]
    test ax,PENDING
    jz  short @F
    pause
    jmp short @B
@@:
    ret
ASSUME DS:DGROUP
genIPI ENDP

prepareGDTDescriptors PROC NEAR C PUBLIC USES ebx ds es,pSels:FAR PTR
ASSUME DS:DGROUP
    pushf
    cli

    les bx,pSels
    db 066h                 ; Intel: need to switch to 32-bit operand size to save full 32-bit base address
    sgdt [gdtSave]
    mov ax,es:[bx]

    and ax,0FFF8h           ; directly use selector value as byte offset into GDT
    movzx eax,ax            ; (by stripping off lowest 3 bits)
    mov ebx,DWORD PTR [gdtSave+2]
    add ebx,eax

    mov ax,DOS32FLATDS
    mov ds,ax
ASSUME DS:FLAT
    ; code segment for Ring 0
    mov DWORD PTR [ebx]   ,0000FFFFh ; Base = 0, Limit = FFFFFh (in pages)
    mov DWORD PTR [ebx+4] ,00CF9B00h ; Execute/Read+Accessed, DPL=0, S=1, P=1, D=1, G=1

    ; stack segment for Ring 0
    mov DWORD PTR [ebx+8] ,0000FFFFh ; Base = 0, Limit = FFFFFh (in pages)
    mov DWORD PTR [ebx+12],00CF9300h ; Read/Write+Accessed, DPL=0, S=1, P=1, D=1, G=1

    ; code segment for Ring 3
    mov DWORD PTR [ebx+16],0000FFFFh ; Base = 0, Limit = FFFFFh (in pages)
    mov DWORD PTR [ebx+20],00CFFB00h ; Execute/Read+Accessed, DPL=3, S=1, P=1, D=1, G=1

    ; stack segment for Ring 3
    mov DWORD PTR [ebx+24],0000FFFFh ; Base = 0, Limit = FFFFFh (in pages)
    mov DWORD PTR [ebx+28],00CFF300h ; Read/Write+Accessed, DPL=3, S=1, P=1, D=1, G=1

    xor eax,eax
    cpuid
    pop ax
    test ax,0200h
    jz short @F
    sti
@@:
    mov ax,1
    ret
prepareGDTDescriptors ENDP


setIPIHandler PROC NEAR C PUBLIC USES ebx ds
    mov ax,DOS32FLATDS
    mov ds,ax
ASSUME DS:FLAT

    db  066h                 ; Intel: need to switch to 32-bit operand size to save full 32-bit base address
    sidt [idtSave]
    mov ebx,[gIPIVector]
    shl ebx,3
    add ebx,DWORD PTR [idtSave+2]
    sub ebx,8
    mov cx,24
@@:
    add ebx,8
    mov eax,[ebx]
    or  eax,[ebx+4]
    loopnz short @B
    jnz short quit

    pushf
    cli

    add  WORD PTR [gIPIVector],23
    sub  WORD PTR [gIPIVector],cx

    push DWORD PTR [ebx+4]
    push DWORD PTR [ebx]
    push 2*SIZE DWORD
    push ss
    mov  ax,sp
    add  ax,4
    push ax
    push 3
    call Trace
    add sp,8+2*SIZE DWORD

    lock and DWORD PTR [ebx+4],0FFFF70FFh ; clear PRESENT bit and type field in descriptor

    mov  ax,DOS32FLATCS
    shl  eax,16                      ; new selector into the upper 16 bits of first DWORD
    mov  edx,OFFSET FLAT:programMSRs
    mov  ax,dx                       ; new lower 16 offset bits into lower 16 bits of first DWORD
    mov  ecx,[ebx+4]                 ; load second DWORD of descriptor
    movzx ecx,cx                     ; zero out upper 16 bits in second DWORD
    xor  dx,dx                       ; zero out lower 16 offset bits
    or   edx,ecx                     ; new upper 16 offset bits into upper 16 bits of second DWORD
    mov  [ebx],eax                   ; overwrite first DWORD in IDT
    mov  [ebx+4],edx                 ; overwrite second DWORD in IDT

    lock or DWORD PTR [ebx+4],000008E00h ; set PRESENT bit and set type to 32-bit interrupt gate

    xor eax,eax
    cpuid                            ; serialize instruction stream
    pop ax
    test ax,0200h
    jz short @F
    sti
@@:
    mov ax,1
    jmp short @F
quit:
    xor ax,ax
@@:
    ret
ASSUME DS:DGROUP
setIPIHandler ENDP

GetCSLimit PROC NEAR C PUBLIC
	mov ax,CGROUP
	lsl ax,ax
	ret
GetCSLimit ENDP

GetDSLimit PROC NEAR C PUBLIC
	mov ax,DGROUP
	lsl ax,ax
	ret
GetDSLimit ENDP

Get32BitSegSelector PROC NEAR C PUBLIC
ASSUME CS:NOTHING
	mov ax,C32GROUP
        ret
Get32BitSegSelector ENDP

hasSysEnterSupport PROC NEAR C PUBLIC USES ebx
    mov eax,1
    cpuid
    xor eax,eax
    test edx,SEP
    setnz al
    ret
hasSysEnterSupport ENDP
_TEXT       ends

RMCode      segment dword public use16 'CODE'
RMCode      ends


STACK32 segment dword public use32 'DATA'
PUBLIC C gRing0Stack,initialESP
gRing0Stack DB 65536 DUP(0)
initialESP EQU $
STACK32 ENDS


CODE32 segment dword public use32 'CODE'
ASSUME DS:FLAT

programMSRs PROC FAR C PUBLIC
    push ebp
    mov  ebp,esp
    and  esp,0FFFFFFFCh
    pushad
    push ds
    push es

    mov eax,DOS32FLATDS
    mov ds,eax
    mov es,eax
ASSUME ds:FLAT
    mov eax,ss
    lar eax,eax
    mov esi,eax
    test esi,OPSIZEBIT
    jnz short @F
    and ebp,0FFFFh
    and esp,0FFFFh
    call KernThunkStackTo32
@@:

    movzx eax,[gSelArray]
    and eax,0FFF8h
    xor edx,edx
    mov ecx,IA32_SYSENTER_CS
    wrmsr

    push eax
    push edx

    mov eax,[gEntryPoint]
    xor edx,edx
    mov ecx,IA32_SYSENTER_EIP
    wrmsr

    push eax
    push edx

ASSUME ds:D32GROUP
    mov eax,OFFSET [initialESP]
    xor edx,edx
    mov ecx,IA32_SYSENTER_ESP
    wrmsr

    push eax
    push edx

    push 6*SIZE DWORD
    lea  eax,[esp+4]
    push eax
    push 1
    call Trace32
    add  esp,12+6*SIZE DWORD

ASSUME ds:FLAT
    mov ebx,[gAPICLinAddr]
    mov DWORD PTR [ebx+EOI],0

    test esi,OPSIZEBIT
    jnz short @F
    call KernThunkStackTo16
@@:
    pop es
    pop ds
    popad
    mov esp,ebp
    pop ebp
    iretd
programMSRs ENDP


Trace32 PROC NEAR C PUBLIC uses ebx,MinorCode:DWORD,Buffer:DWORD,BufferLength:DWORD
    mov ebx,OFFSET [KernSISData.SIS_mec_table]; KernSISData is pointer to global InfoSeg
    mov dl,(080h SHR (MAJOR_TRACE_CODE MOD 8))
    test [ebx+(MAJOR_TRACE_CODE/8)],dl
    jz short @F
; https://beta.groups.yahoo.com/neo/groups/os2ddprog/conversations/topics/923
; KernRASSysTrace is limited to trace data placed on the stack !!!
    push BufferLength
    push Buffer
    push MinorCode
    push MAJOR_TRACE_CODE
    call KernRASSysTrace
    add esp,16
@@:
    ret
Trace32 ENDP
CODE32      ends



DGROUP      group _HEADER, CONST, CONST2, _DATA, _BSS
CGROUP      group _TEXT, RMCode
D32GROUP    group STACK32
C32GROUP    group CODE32

end


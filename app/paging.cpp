/* the purpose of this program is read all the necessary register information regarding to the type of paging
   used by the OS. That said, control registers CR0 and CR4 as well as MSR register IA32_ENFER must be read.
   All of these registers can only be read from Ring-0 which is why SYSCALL is used
*/

#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>

extern ULONG APIENTRY ReadCR0(VOID);
extern ULONG APIENTRY ReadCR4(VOID);
extern ULONGLONG APIENTRY ReadMSR(ULONG msrNum);

#define IA32_EFER 0xC0000080UL

#define PE  0x00000001UL
#define PG  0x80000000UL
#define PSE 0x00000010UL
#define PAE 0x00000020UL
#define LME 0x00000100UL
#define LMA 0x00000400UL

#define CR0 pagingRegs.cr0Reg
#define CR4 pagingRegs.cr4Reg
#define EFER pagingRegs.eferReg.ulLo


typedef struct _PAGINGREGS_
{
   ULONGLONG eferReg;
   ULONG cr0Reg;
   ULONG cr4Reg;
} PAGINGREGS, *PPAGINGREGS;


APIRET APIENTRY QueryPagingInfo(ULONG arg1,ULONG arg2)
{
     PPAGINGREGS pRegs = (PPAGINGREGS)arg1;

     pRegs->eferReg = ReadMSR(IA32_EFER);
     pRegs->cr0Reg  = ReadCR0();
     pRegs->cr4Reg  = ReadCR4();

     return NO_ERROR;
}

int main(int argc, char *argv[])
{

   UCHAR bitString[64]={0};
   PAGINGREGS pagingRegs={0};
   APIRET rc = NO_ERROR;
   USERPFN FuncTable[1]={QueryPagingInfo};
   WRAPPERPFN pfnWrapper = NULL;

   pfnWrapper = registerUserFunctions(FuncTable,1);
   if (pfnWrapper) {
      rc = pfnWrapper(0,(ULONG)&pagingRegs,0);

      printf("CR0: %32s\n",ultoa(CR0,(char*)bitString,2));
      printf("CR4: %32s\n",ultoa(CR4,(char*)bitString,2));
      printf("EFER:%32s\n",ultoa(EFER,(char*)bitString,2));
      printf("\n");

      if ((CR0 & PE) && (CR0 & PG) && !(CR4 & PSE) && !(CR4 & PAE) && !(EFER & LME) && !(EFER & LMA)) {
         printf("32-Bit Paging with 4 kByte Pages\n");
      } else
      if ((CR0 & PE) && (CR0 & PG) && (CR4 & PSE) && !(CR4 & PAE) && !(EFER & LME) && !(EFER & LMA)) {
         printf("32-bit Paging with 4 MByte Pages\n");
      } else
      if ((CR0 & PE) && (CR0 & PG) && (CR4 & PAE) && !(EFER & LME) && !(EFER & LMA)) {
         printf("PAE Paging\n");
      } else
      if ((CR0 & PE) && (CR0 & PG) && (CR4 & PAE) && (EFER & LME) && (EFER & LMA)) {
         printf("4-Level Paging\n");
      } /* endif */
   } /* endif */

   return 0;
}


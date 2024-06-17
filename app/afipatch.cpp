/* the purpose of this program is to patch AFINETK.SYS and overwrite all calls
   to KernSerialize16BitDD and KernUnserialize16BitDD with NOPs or RET/INT 3 respectively
   see patchTable: the first 4 values are the offsets into the 32-bit memory object (the 3. memory object)
                   the second 4 chains of values are the instruction bytes to write
   BEWARE: while the patching works, the changes made to AFINETK.SYS might be detrimental to
           operation of your system !
*/

#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <syscall.h>


typedef struct _KernPageList /* PGL */
{
  ULONG Addr;
  ULONG Size;
} KernPageList_t;

/*
 * AllocFlags
 */
#define VMDHA_16MB        0x001 /* (0000 0000 00001B)
                                 * If set, the object will be allocated below
                                 * the 16M line
                                 */
#define VMDHA_FIXED       0x002 /* (0000 0000 00010B)
                                 * If set, the object must be in fixed memory
                                 * at all times.
                                 * If clear, then the object may be moved or
                                 * paged out as is necessary.
                                 */
#define VMDHA_SWAP        0x004 /* (0000 0000 0100B)
                                 * If set, swappable memory will be allocated
                                 * for the object.
                                 * If clear, the memory will be movable or
                                 * fixed dependending on the setting of
                                 * VMDHA_FIXED.
                                 * If set, VMDHA_FIXED must be clear.
                                 */
#define VMDHA_CONTIG      0x008 /* (0000 0000 1000B)
                                 * If set, the object needs to be allocated in
                                 * contiguous memory.
                                 * If clear, the the pages may be discontiguous
                                 * in physical memory.
                                 * In order to request contiguous memory, the
                                 * PDD must have also requested fixed memory.
                                 * (VMDHA_FIXED must be set)
                                 */
#define VMDHA_PHYS        0x010 /* (0000 0001 0000B)
                                 * If set, a linear address mapping will be
                                 * obtained for the physical passed in the
                                 * PhysAddr pointer.
                                 */
#define VMDHA_PROCESS     0x020 /* (0000 0010 0000B)
                                 * If set, the linear address returned will be
                                 * in the process address range.
                                 * If clear, the allocation will be done in the
                                 * global address range, that is, accessible
                                 * outside of the current process' context.
                                 */
#define VMDHA_SGSCONT     0x040 /* (0000 0100 0000B)
                                 * If set, the allocated memory can be
                                 * registered under screen group switch control.
                                 * This flag is valid only if mapping is in
                                 * process address range.  (VMDHA_PROCESS must
                                 * be set)
                                 */
#define VMDHA_SELMAP      0x080 /* (0000 1000 0000B)
                                 * If set, provide a selector mapping for the
                                 * linear address range.
                                 */
#define VMDHA_RESERVE     0x100 /* (0001 0000 0000B)
                                 * If set, the memory will only be reserved.
                                 * No committment will be done and any attempt
                                 * to access reserved but not committed memory
                                 * will cause a fatal page fault.
                                 */
#define VMDHIA_USER       0x200 /* (0010 0000 0000B)
                                 * If set, provides user mode access.
                                 * (init time only)
                                 */
#define VMDHA_SHARED      0x400 /* (0100 0000 0000B)
                                 * If set, Allocate shared memory object. This
                                 * flag is only supported for mapping a
                                 * physical address to non-system memory into
                                 * the shared arena. VMDHA_PHYS must be
                                 * specified.
                                 */
#define VMDHA_USEHIGHMEM  0x800 /* (1000 0000 0000B)
                                 * If set, the object will be allocated above
                                 * the 16MB line if possible.  If memory above
                                 * 16MB exists but there is not enough to
                                 * satisfy the request, the memory above 16MB
                                 * will be used first and the remainder taken
                                 * from below 16MB.  If no memory above 16MB
                                 * exists, the allocation will be taken from
                                 * existing memory.
                                 * This bit is valid only during device driver
                                 * initialization.  If this bit is used at any
                                 * other time, KernVMAlloc will return an error.
                                 * All other bits must be clear.
                                 */
#define VMDHA_ALIGN64K 0x1000   /* (0001 0000 0000 0000B)
                                 * If set, the linear address of the object
                                 * will be aligned on 64K boundary.
                                 */

APIRET  APIENTRY KernLinToPageList (VOID           *pLinAddr,
                                    ULONG           Size,
                                    KernPageList_t *pPageList,
                                    ULONG          *pPageListCount);

APIRET APIENTRY KernVMAlloc (ULONG   cBytes,
                             ULONG   VMAllocFlags,
                             VOID  **ppLinAddr,
                             VOID  **ppPhysAddr,
                             VOID  **SysReserved);

APIRET  APIENTRY KernVMFree (VOID *pLinAddr);

VOID    APIENTRY KernSerialize16BitDD (VOID);
VOID    APIENTRY KernUnserialize16BitDD (VOID);


typedef struct _PATCHTYPE_
{
   ULONG  baseAddr;
   ULONG  size;
   PUCHAR PatchAddresses[5];
   UCHAR PatchBytes[5][16];
} PATCHTYPE, *PPATCHTYPE;

UCHAR SysStateBuffer[65536]={0};
/* these are the memory offsets and bytes to patch in AFINETK in order to overwrite the KernSerialize16Bit and KernUnserialize16Bit calls in the driver */
PATCHTYPE patchTable = {0,
                        0,
                        {(PUCHAR)0x4CDF,(PUCHAR)0x1DFF1,(PUCHAR)0x4E01,(PUCHAR)0x1E6CB},
                        {{0x90,0x90,0x90,0x90,0x90},{0x90,0x90,0x90,0x90,0x90},{0xC3,0xCC,0xCC,0xCC,0xCC},{0x90,0x90,0x90,0x90,0x90},{0}}};


APIRET APIENTRY PatchCode(ULONG arg1,ULONG arg2)
{
   PPATCHTYPE ppatchInfo = (PPATCHTYPE)arg1;
   ULONG i=0,j=0;
   APIRET rc = NO_ERROR;
   KernPageList_t pageList={0};
   ULONG ulNumEntries=0;
   ULONG linAddr = 0;
   PUCHAR pByte = NULL;

   /*
      NOTE: we are patching a driver's static code segment.
      That allows us to make assumptions that hold true FOR THIS SPECIFIC CASE ONLY:
      1) the whole code segment is physically contiguous. That's why it is enough to cater for one
         entry in the "pageList" structure
      2) a driver's code segment is RESIDENT and IMMOVABLE in memory. That's why it is unnecessary
         to lock the segment
      3) what we DO have to do is to create a read/write mapping to the code segment. There is no
         way around that, we cannot work with the linear code address
      4) KernSerialize16BitDD and KernUnserialize16BitDD are used to acquire and release the global
         "R0SubSystem" Spinlock. For as long as this spinlock is held, it prevents any other driver
         from executing which hopefully will prevent interference while we are busy patching
         the driver
   */
   rc = KernLinToPageList((PVOID)ppatchInfo->baseAddr,ppatchInfo->size,&pageList,&ulNumEntries);
   if (NO_ERROR == rc) {
      ULONG physAddr = pageList.Addr;
      PVOID pSel = NULL;
      rc = KernVMAlloc(pageList.Size,VMDHA_PHYS,(PPVOID)&linAddr,(PPVOID)&physAddr,&pSel);
   } /* endif */

   if (NO_ERROR == rc) {
      KernSerialize16BitDD();
      while (ppatchInfo->PatchAddresses[i]) {
         ppatchInfo->PatchAddresses[i] += linAddr;
         pByte = (PUCHAR)ppatchInfo->PatchAddresses[i];
         j=0;
         while(ppatchInfo->PatchBytes[i][j]) {
            pByte[j] = ppatchInfo->PatchBytes[i][j];
            j++;
         }
         i++;
      }
      rc = KernVMFree((PVOID)linAddr);
      KernUnserialize16BitDD();
   } /* endif */

   return rc;
}

int main(int argc,char *argv[]) {
   APIRET rc = NO_ERROR;
   QSPTRREC *pPointers = NULL;
   QSLREC *pMTERec = NULL;
   ULONG baseAddr = 0;
   ULONG size = 0;
   WRAPPERPFN wrapper = NULL;
   USERPFN FunctionTable[1] = {PatchCode};
   BOOL quit = FALSE;

   /* first thing to do is to query all module info to find AFINETK and where its 32-bit code segment (3. memory object) is located in memory */
   rc = DosQuerySysState(QS_MTE,QS_MTE,0UL,0UL,SysStateBuffer,sizeof(SysStateBuffer));
   pPointers = (QSPTRREC *)SysStateBuffer;
   pMTERec = pPointers->pLibRec;

   /* work around bug in DosQuerySysState: unfortunately, some pointers are set to zero where they should be set to the correct values */
   /* the affected pointers are the pointers to the object tables as well as the "pNextRec" pointer */
   while (pMTERec) {
      quit = FALSE;
      if (!pMTERec->pNextRec && pMTERec->pObjInfo) {
         quit = TRUE;
      } /* endif */

      if (pMTERec->ctObj && !pMTERec->pObjInfo) {
         pMTERec->pObjInfo = (QSLOBJREC *)(((ULONG)pMTERec->pName + strlen((PCSZ)pMTERec->pName) + sizeof(ULONG)) & ~(sizeof(ULONG)-1));
      } /* endif */

      if (strstr((PCSZ)pMTERec->pName,"AFINETK")) {
         QSLOBJREC *pObjects = pMTERec->pObjInfo;
         baseAddr = pObjects[2].oaddr;
         size     = pObjects[2].osize;
         break;
      } /* endif */

      if (quit) {
         break;
      } /* endif */

      if (!pMTERec->pNextRec && pMTERec->ctObj) {
         pMTERec->pNextRec = (PVOID)((((ULONG)pMTERec->pName + strlen((PCSZ)pMTERec->pName) + sizeof(ULONG)) & ~(sizeof(ULONG)-1)) + pMTERec->ctObj*sizeof(QSLOBJREC));
      } /* endif */
      pMTERec = (QSLREC *)pMTERec->pNextRec;
   } /* endwhile */

   wrapper = registerUserFunctions(FunctionTable,1);
   if (baseAddr && wrapper) {
      patchTable.baseAddr = baseAddr;
      patchTable.size     = size;
      rc = wrapper(0,(ULONG)&patchTable,0UL);
      printf("rc: %#x\n",rc);
   } /* endif */


   return 0;
}

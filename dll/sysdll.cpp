/*****************************************************************************/
/*                                                                           */
/* COPYRIGHT    Copyright (C) 2017 Lars Erdmann                              */
/*                                                                           */
/*    The following OS/2 source code is provided to you solely for           */
/*    the purpose of assisting you in your development of OS/2 device        */
/*    drivers. This Copyright statement may not be removed.                  */
/*    All Rights Reserved                                                    */
/*                                                                           */
/*****************************************************************************/
#define INCL_BASE
#include <os2.h>
#include <memory.h>
#include <stdio.h>
#include <syscall.h>

typedef ULONG LIN;

extern APIRET APIENTRY FunctionWrapper(ULONG,ULONG,ULONG);
extern APIRET APIENTRY Ring0EntryPoint(VOID);
extern LIN APIENTRY gStackBase;
extern PFN APIENTRY FunctionTable[256];

#pragma pack(1)
typedef struct
{
   LIN StackBase;
} STACKINFO, FAR *PSTACKINFO;
#pragma pack()


WRAPPERPFN APIENTRY registerUserFunctions(USERPFN *FuncTable,ULONG numFuncs)
{
   HFILE hFile    = NULLHANDLE;
   ULONG ulAction = 0UL;
   PFN   pFunction = (PFN)Ring0EntryPoint;
   STACKINFO info = {0};
   ULONG plen = sizeof(pFunction);
   ULONG dlen = sizeof(info);
   APIRET rc = NO_ERROR;

   if (numFuncs > sizeof(FunctionTable)/sizeof(FunctionTable[0]))
   {
      return NULL;
   }

   memset(FunctionTable,0,sizeof(FunctionTable));
   memcpy(FunctionTable,FuncTable,numFuncs*sizeof(PFN));

   if (NO_ERROR == DosOpen("\\DEV\\SYSCALL$",&hFile,&ulAction,0UL,FILE_NORMAL,OPEN_ACTION_OPEN_IF_EXISTS,OPEN_SHARE_DENYREADWRITE,NULL))
   {
      rc = DosDevIOCtl(hFile,0x80,1,&pFunction,plen,&plen,&info,dlen,&dlen);
      gStackBase     = info.StackBase;
      DosClose(hFile);
      return (NO_ERROR == rc) ? FunctionWrapper : NULL;
   }

   return NULL;
}

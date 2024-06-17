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
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include <strat2.h>
#include <reqpkt.h>
#include <devhdr.h>
#include <devcmd.h>
#include <devhelp.h>
#include <string.h>
#include <rmcalls.h>
#include <infoseg.h>

extern VOID FAR Strat(PRPH rp);
extern VOID    Trace(USHORT MinorCode,PVOID Buffer,USHORT BufLen);
extern USHORT  GetCSLimit(VOID);
extern USHORT  GetDSLimit(VOID);
extern SEL     Get32BitSegSelector(VOID);
extern BOOL    hasSysEnterSupport(VOID);
extern BOOL    setIPIHandler(VOID);
extern LIN     getAPICLinAddr(VOID);
extern VOID    genIPI(LIN APICLInAddr,LIN UserRoutine);
extern BOOL    isIntel(VOID);
extern ULONG   getProcessorInfo(VOID);
extern USHORT  StratInit( PRPINITIN rp );
extern USHORT  StratOpen( PRP_OPENCLOSE rp);
extern USHORT  StratClose( PRP_OPENCLOSE rp);
extern USHORT  StratIOCTL( PRP_GENIOCTL rp);
extern BOOL    prepareGDTDescriptors(PSEL pSels);
extern DDHDR   FAR DrvHeader;
extern struct  InfoSegGDT FAR *gGlobalInfoSeg;
extern LIN     gAPICLinAddr;
extern LIN     gStackAddr;
extern ULONG   gStackSize;
extern BOOL    fIsOpen;
extern USHORT  sfn;

#define MSG_REPLACEMENT_STRING 1178
#define NUM_SELS 4

#pragma pack(1)
typedef struct
{
   LIN   StackBase;
} STACKINFO, FAR *PSTACKINFO;
#pragma pack()


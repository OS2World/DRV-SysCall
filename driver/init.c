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
#include <includes.h>

static char BootupBanner[] = {
    "\r\nSysEnter/SysExit Ring-0 direct call driver installed as:        \r\n"
};

static char NoConsecutiveGDTs[] = {
    "\r\nCould not allocate 5 consecutive GDT entries !\r\n"
};

static char NoSupport[] = {
    "\r\nThis CPU does not support the SYSENTER/SYSEXIT instructions !\r\n"
};


MSGTABLE msgTable={MSG_REPLACEMENT_STRING,1,BootupBanner};
PFN    Device_Help;
ULONG  RMFlags   = 0UL;
PFNRM  RM_Help0  = NULL;
PFNRM  RM_Help3  = NULL;
SEL    gSelArray[NUM_SELS] = {0};

DRIVERSTRUCT drvStruct = {"SYSCALL.SYS",
                          "SysEnter/SysExit Ring-0 direct call mechanism",
                          VENDOR,
                          VERMAJOR,
                          VERMINOR,
                          {YEAR,MONTH,DAY},
                          DRF_STATIC,
                          DRT_OS2,
                          DRS_APP_HELPER,
                          NULL};
HDRIVER      hDriver   = 0UL;


USHORT StratInit( PRPINITIN rp )
{
    SELDESCINFO info={0};
    PRPINITOUT rpo = (PRPINITOUT)rp;
    PSEL pSel = NULL;
    APIRET rc = NO_ERROR;
    UCHAR lock1[12]={0};
    UCHAR lock2[12]={0};

    Device_Help = rp->DevHlpEP;

    DevHelp_GetDOSVar(DHGETDOSV_SYSINFOSEG,0,(PPVOID)&pSel);
    gGlobalInfoSeg = (struct InfoSegGDT FAR *)MAKEP(*pSel,0);

    rc = DevHelp_GetDescInfo(Get32BitSegSelector(),(PBYTE)&info);
    if (NO_ERROR == rc)
    {
       ULONG ulCount=0UL;
       LIN linAddr=0UL;
       PVOID pLock = lock1;
       DevHelp_VirtToLin(SELECTOROF(pLock),OFFSETOF(pLock),&linAddr);
       rc = DevHelp_VMLock(VMDHL_LONG,info.BaseAddr,info.Limit+1UL,(LIN)-1,linAddr,&ulCount);
    }
    if (NO_ERROR == rc)
    {
       ULONG ulCount=0UL;
       LIN linAddr=0UL;
       PVOID pLock = lock2;
       DevHelp_VirtToLin(SELECTOROF(pLock),OFFSETOF(pLock),&linAddr);
       DevHelp_VMLock(VMDHL_LONG,gStackAddr,gStackSize,(LIN)-1,linAddr,&ulCount);

       if (0UL != (gAPICLinAddr = getAPICLinAddr()))
       {
          UCHAR ucFam      = 0;
          UCHAR ucStepping = 0;
          UCHAR bIsValid   = TRUE;


#pragma pack(1)
          struct {
            ULONG ulFamMod;
            UCHAR ucDispFam;
            UCHAR ucMod;
          } TraceData;
#pragma pack()

          if (isIntel())
          {
             TraceData.ulFamMod = getProcessorInfo();
             ucFam              = (UCHAR)((TraceData.ulFamMod >> 8) & 0xF);
             ucStepping         = (UCHAR)(TraceData.ulFamMod & 0xF);

             if (ucFam != 0xF)
             {
                TraceData.ucDispFam = ucFam;
             }
             else
             {
                TraceData.ucDispFam = ((UCHAR)(TraceData.ulFamMod >> 20)) + ucFam;
             }
             if ((ucFam == 0x6) || (ucFam == 0xF))
             {
                TraceData.ucMod = ((UCHAR)(TraceData.ulFamMod >> 12) & 0xF0) + ((UCHAR)(TraceData.ulFamMod >> 4) & 0xF);
             }
             else
             {
                TraceData.ucMod = ((UCHAR)(TraceData.ulFamMod >> 4) & 0xF);
             }
             if ((TraceData.ucDispFam == 0x6) && (TraceData.ucMod < 0x3) & (ucStepping < 3))
             {
                bIsValid = FALSE;
             }
          }
          else
          {
             TraceData.ulFamMod = getProcessorInfo();
             ucFam              = (UCHAR)((TraceData.ulFamMod >> 8) & 0xF);
             if (ucFam != 0xF)
             {
                TraceData.ucDispFam = ucFam;
             }
             else
             {
                TraceData.ucDispFam = ((UCHAR)(TraceData.ulFamMod >> 20)) + ucFam;
             }
             if (ucFam == 0xF)
             {
                TraceData.ucMod = ((UCHAR)(TraceData.ulFamMod >> 12) & 0xF0) + ((UCHAR)(TraceData.ulFamMod >> 4) & 0xF);
             }
             else
             {
                TraceData.ucMod = ((UCHAR)(TraceData.ulFamMod >> 4) & 0xF);
             }
          }

          Trace(5,&TraceData,sizeof(TraceData));

          if (!hasSysEnterSupport() || !bIsValid)
          {
             msgTable.MsgStrings[0] = NoSupport;
             DevHelp_Save_Message((NPBYTE)&msgTable);

             rpo->Unit       = 0;
             rpo->BPBArray   = 0;
             rpo->CodeEnd    = 0;
             rpo->DataEnd    = 0;
             return STDON | STERR | ERROR_I24_QUIET_INIT_FAIL;
          }

          rc = DevHelp_AllocGDTSelector(gSelArray,NUM_SELS);
          if (NO_ERROR == rc)
          {
             USHORT i=0;

             gSelArray[0] &= 0xFFF8;
             for(i=1;i<NUM_SELS;i++)
             {
                gSelArray[i] &= 0xFFF8;
                if ((gSelArray[i] - gSelArray[i-1]) != 8)
                {
                   break;
                }
             }

             if (i<NUM_SELS)
             {
                for (i=0;i<NUM_SELS;i++)
                {
                   rc = DevHelp_FreeGDTSelector(gSelArray[i]);
                }
                msgTable.MsgStrings[0] = NoConsecutiveGDTs;
                DevHelp_Save_Message((NPBYTE)&msgTable);

                rpo->Unit       = 0;
                rpo->BPBArray   = 0;
                rpo->CodeEnd    = 0;
                rpo->DataEnd    = 0;
                return STDON | STERR | ERROR_I24_QUIET_INIT_FAIL;
             }


             if (prepareGDTDescriptors(gSelArray) && setIPIHandler())
             {
                _fmemcpy(BootupBanner+58,DrvHeader.DevName,8);
                DevHelp_Save_Message((NPBYTE)&msgTable);

                RMCreateDriver(&drvStruct,&hDriver);

                rpo->Unit       = 0;
                rpo->BPBArray   = 0;
                rpo->CodeEnd    = GetCSLimit();
                rpo->DataEnd    = GetDSLimit();
                return STDON;
             }
          }
       }
    }

    rpo->Unit       = 0;
    rpo->BPBArray   = 0;
    rpo->CodeEnd    = 0;
    rpo->DataEnd    = 0;

    return STDON | STERR | ERROR_I24_QUIET_INIT_FAIL;
}


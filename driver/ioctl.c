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


USHORT StratIOCTL( PRP_GENIOCTL rp)
{

   if (rp->Category == 0x80 && rp->Function == 1)
   {
      if (NO_ERROR != DevHelp_VerifyAccess(SELECTOROF(rp->ParmPacket),sizeof(LIN),OFFSETOF(rp->ParmPacket),VERIFY_READONLY))
      {
         return STDON | STERR | ERROR_I24_INVALID_PARAMETER;
      }

      if (NO_ERROR != DevHelp_VerifyAccess(SELECTOROF(rp->DataPacket),sizeof(STACKINFO),OFFSETOF(rp->DataPacket),VERIFY_READWRITE))
      {
         return STDON | STERR | ERROR_I24_INVALID_PARAMETER;
      }

      LIN UserRoutine  = *(PLIN)(rp->ParmPacket);
      PSTACKINFO p     = (PSTACKINFO)(rp->DataPacket);
      p->StackBase     = gStackAddr;

      genIPI(gAPICLinAddr,UserRoutine);
      return STDON;
   }
   return STDON | STERR | ERROR_I24_BAD_COMMAND;
}

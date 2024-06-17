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

USHORT StratClose( PRP_OPENCLOSE rp)
{
   if (sfn == rp->sfn)
   {
      fIsOpen = FALSE;
      sfn     = 0;
      return STDON;
   }
   return STDON | STERR | ERROR_I24_INVALID_PARAMETER;
}

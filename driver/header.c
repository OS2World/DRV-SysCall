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

// Ensure that the header is located at the beginning of the driver
#pragma data_seg ("_HEADER", "DATA" ) ;

DDHDR FAR DrvHeader = {
      (PVOID)-1     ,
      DEV_CHAR_DEV | DEV_30 | DEVLEV_3 ,
      (USHORT)Strat,
      0,
      "SYSCALL$",
      0,0,0,0,
      DEV_INITCOMPLETE | DEV_IOCTL2 | DEV_16MB
};


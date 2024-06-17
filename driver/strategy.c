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

BOOL fIsOpen = FALSE;
USHORT sfn   = 0;

#pragma aux STRATEGY far parm [es bx];
#pragma aux (STRATEGY) Strat;

VOID FAR Strat( PRPH rp)
{
        if (rp->Cmd <= CMDStop)
        {
                switch( rp->Cmd ) {
                case CMDInitBase:
                        rp->Status = StratInit( ( PRPINITIN)rp );
                        break;
                case CMDInitComplete:
                        rp->Status = STDON;
                        break;
                case CMDOpen:
                        rp->Status = StratOpen( ( PRP_OPENCLOSE)rp );
                        break;
                case CMDClose:
                        rp->Status = StratClose( ( PRP_OPENCLOSE)rp );
                        break;
                case CMDGenIOCTL:
                        rp->Status = StratIOCTL ((PRP_GENIOCTL)rp );
                        break;
                default:
                        rp->Status = STERR | STDON | ERROR_I24_BAD_COMMAND;
                }
        } else {
                rp->Status = STERR | STDON | ERROR_I24_BAD_COMMAND;
        }
}

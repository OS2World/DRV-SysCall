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
#ifndef __SYSCALL_H__
#define __SYSCALL_H__

typedef APIRET (APIENTRY *WRAPPERPFN)(ULONG index,ULONG arg1,ULONG arg2);
typedef APIRET (APIENTRY *USERPFN)(ULONG arg1,ULONG arg2);
extern WRAPPERPFN APIENTRY registerUserFunctions(USERPFN *FuncTable,ULONG numFuncs);

#endif

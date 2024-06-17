#*****************************************************************************#
#*                                                                           *#
#* COPYRIGHT    Copyright (C) 2017 Lars Erdmann                              *#
#*                                                                           *#
#*    The following OS/2 source code is provided to you solely for           *#
#*    the purpose of assisting you in your development of OS/2 device        *#
#*    drivers. This Copyright statement may not be removed.                  *#
#*    All Rights Reserved                                                    *#
#*                                                                           *#
#*****************************************************************************#
#===================================================================
#
#   Auto-dependency information
#
#===================================================================
VERSION=$(%VERMAJOR).$(%VERMINOR)
VENDOR=$(%VENDOR)
VERMAJOR=$(%VERMAJOR)
VERMINOR=$(%VERMINOR)
DAY=$(%DAY)
MONTH=$(%MONTH)
YEAR=$(%YEAR)

LIB_H   = $(%WATCOM)\h
OS2_H   = $(%WATCOM)\h\os2
DDK_H   = $(%DDK)\base\h


C32FLAGS  = -i. -i..\DLL -i$(LIB_H) -i$(OS2_H) -i$(DDK_H) -bt=os2 -5 -za99 -ze -zq -zlf -wx
AFLAGS    = -Mb +Lm -Lr -Lf -Llp:0
!ifdef DEBUG
C32FLAGS = $(C32FLAGS) -d2
AFLAGS   = $(AFLAGS) +Od
!else
C32FLAGS = $(C32FLAGS) -ox
!endif

WC32    = wcc386
ALP     = alp
LINK    = wlink

!ifdef __LOADDLL__
!   loaddll $(WC32) $(%WATCOM)\BINP\DLL\wccd386.dll
!   loaddll $(LINK) $(%WATCOM)\BINP\DLL\wlinkd.dll
!endif

.HOLD

.BEFORE
        @set PATH=$(%PATH);$(%WATCOM)\BINW

.SUFFIXES:
.SUFFIXES: .obj .cpp .asm
.cpp.obj: .AUTODEPEND
        $(WC32) $(C32FLAGS) $<

.asm.obj: .AUTODEPEND
	$(ALP) $(AFLAGS) $< -Fl:$*.cod

all:    .SYMBOLIC afipatch.exe paging.exe
        @echo All done!

EXEOBJS   =  afipatch.obj
afipatch.exe: $(EXEOBJS)
        @%create $*.lnk
        @%append $*.lnk name $@
        @%append $*.lnk system os2v2
        @%append $*.lnk option quiet
        @%append $*.lnk option map=$*.map
        @%append $*.lnk option offset=0x10000
        @%append $*.lnk option stack=0x10000
        @%append $*.lnk sort global
        @%append $*.lnk option osname='eComStation'
!ifdef DEBUG
        @%append $*.lnk debug all
!endif
        @for %f in ($(EXEOBJS)) do @%append $*.lnk file %f
        @%append $*.lnk import registerUserFunctions SYSCALL.registerUserFunctions
        @%append $*.lnk import KernVMFree KEE.61
        @%append $*.lnk import KernLinToPageList KEE.64
        @%append $*.lnk import KernSerialize16BitDD KEE.42
        @%append $*.lnk import KernUnserialize16BitDD KEE.43
        @%append $*.lnk import KernVMAlloc KEE.60
        $(LINK) @$*.lnk
        @wat2map $*.map $*.mms
        @mapsym -n $*.mms
        @%erase $*.mms

EXEOBJS2   = helpers.obj paging.obj
paging.exe: $(EXEOBJS2)
        @%create $*.lnk
        @%append $*.lnk name $@
        @%append $*.lnk system os2v2
        @%append $*.lnk option quiet
        @%append $*.lnk option map=$*.map
        @%append $*.lnk option offset=0x10000
        @%append $*.lnk option stack=0x10000
        @%append $*.lnk sort global
        @%append $*.lnk option osname='eComStation'
!ifdef DEBUG
        @%append $*.lnk debug all
!endif
        @for %f in ($(EXEOBJS2)) do @%append $*.lnk file %f
        @%append $*.lnk import registerUserFunctions SYSCALL.registerUserFunctions
        $(LINK) @$*.lnk
        @wat2map $*.map $*.mms
        @mapsym -n $*.mms
        @%erase $*.mms

clean: .SYMBOLIC
        @if exist *.obj del *.obj
        @if exist *.exe del *.exe
        @if exist *.sym del *.sym
        @if exist *.err del *.err
        @if exist *.map del *.map
        @if exist *.lnk del *.lnk

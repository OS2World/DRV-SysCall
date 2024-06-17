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
DDK_INC = $(%DDK)\base\inc


C32FLAGS  = -i. -i$(LIB_H) -i$(OS2_H) -bt=os2 -5 -za99 -ze -zq -zlf -wx
!ifdef DEBUG
C32FLAGS = $(C32FLAGS) -d2
!else
C32FLAGS = $(C32FLAGS) -ox
!endif
AFLAGS    = -Mb -M:4512- -I:$(DDK_INC) +Fl -Lr -Llp:0 -Li +Lm +Ls # disable Warning ALP4512: Can't ASSUME CS to different segment or group; ignored
!ifdef DEBUG
AFLAGS = $(AFLAGS) -Odl:MS16
!endif

WC32    = wcc386
ASM     = alp
LINK    = wlink
LIB     = wlib

!ifdef __LOADDLL__
!   loaddll $(WC32) $(%WATCOM)\BINP\DLL\wccd386.dll
!   loaddll $(LIB) $(%WATCOM)\BINP\DLL\wlibd.dll
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
        $(ASM) $(AFLAGS) $<

all:    .SYMBOLIC syscall.dll syscall.lib
        @echo All done!
        @copy syscall.dll \os2\dll
        @copy syscall.sym \os2\dll

sysdll.obj: sysdll.cpp
        $(WC32) -bd $(C32FLAGS) $<

dllasm.obj: dllasm.asm
	$(ASM) -Mb -M:4512- -I:$(DDK_INC) +Fl -Lr -Llp:0 -Li +Lm +Ls $<

DLLOBJS  =  dllasm.obj sysdll.obj
syscall.dll: $(DLLOBJS)
        @%create $*.lnk
        @%append $*.lnk name $@
        @%append $*.lnk system os2v2_dll initinstance terminstance
        @%append $*.lnk option quiet,manyautodata
        @%append $*.lnk option map=$*.map
        @%append $*.lnk sort global
        @%append $*.lnk option osname='eComStation'
        @AddToFile.cmd $*.lnk,option description,BLDLEVEL,$(VENDOR),$(VERSION),SysEnter/SysExit Ring-0 direct call mechanism
!ifdef DEBUG
        @%append $*.lnk debug all
!endif
        @%append $*.lnk import KernThunkStackTo16 KEE.40
        @%append $*.lnk import KernThunkStackTo32 KEE.41
        @%append $*.lnk export registerUserFunctions
        @for %f in ($(DLLOBJS)) do @%append $*.lnk file %f
        $(LINK) @$*.lnk
        @wat2map $*.map $*.mms
        @mapsym -n $*.mms
        @%erase $*.mms

syscall.lib: syscall.dll
        $(LIB) -n -b -q -fo -io -irn -p=16 $@ $<



clean: .SYMBOLIC
        @if exist *.obj del *.obj
        @if exist *.sys del *.sys
        @if exist *.exe del *.exe
        @if exist *.dll del *.dll
        @if exist *.sym del *.sym
        @if exist *.err del *.err
        @if exist *.map del *.map
        @if exist *.lnk del *.lnk
        @if exist *.lst del *.lst
        @if exist *.tff del *.tff
        @if exist *.lib del *.lib

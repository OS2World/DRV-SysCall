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

OS2_H   = $(%WATCOM)\h
DDK_H   = $(%DDK)\base\h
DDK_INC = $(%DDK)\base\inc
WATCOM_LIB = $(%WATCOM)\lib286\os2
DDK_LIB    = $(%DDK)\base\lib


C16FLAGS  = -i. -i=$(OS2_H) -i=$(DDK_H) -bt=os2 -ms -ecc -5 -s -za99 -ze -zp1 -zdp -zff -zgf -zu -zl -zq -wx -dVERMAJOR=$(VERMAJOR) -dVERMINOR=$(VERMINOR) -dDAY=$(DAY) -dMONTH=$(MONTH) -DYEAR=$(YEAR) -dVENDOR="$(VENDOR)"
!ifdef DEBUG
C16FLAGS = $(C16FLAGS) -d1 -hc
!else
C16FLAGS = $(C16FLAGS) -ox
!endif
AFLAGS    = -Mb -M:4512- -I:$(DDK_INC) +Fl -Lr -Llp:0 -Li +Lm +Ls # disable Warning ALP4512: Can't ASSUME CS to different segment or group; ignored
!ifdef DEBUG
AFLAGS = $(AFLAGS) -Odl:MS16
!endif

WC16    = wcc
ASM     = alp
LINK    = wlink

!ifdef __LOADDLL__
!   loaddll $(WC16) $(%WATCOM)\BINP\DLL\wccd.dll
!   loaddll $(LINK) $(%WATCOM)\BINP\DLL\wlinkd.dll
!endif

.HOLD

.BEFORE
        @set PATH=$(%PATH);$(%WATCOM)\BINW

.SUFFIXES:
.SUFFIXES: .obj .tff .c .asm .tsf
.c.obj: .AUTODEPEND
        $(WC16) $(C16FLAGS) $<

.asm.obj: .AUTODEPEND
        $(ASM) $(AFLAGS) $<

all:    .SYMBOLIC syscall.sys trc000a.tff
        @echo All done!
        @copy syscall.sys \os2\boot
        @copy syscall.sym \os2\boot
        @copy trc000a.tff \os2\system\trace
        @echo To install driver using CONFIG.SYS: BASEDEV=SYSCALL.SYS

trc000a.tff: syscall.tsf
        trcust.exe $< $@


OBJS  =  devsegs.obj header.obj strategy.obj init.obj open.obj close.obj ioctl.obj
syscall.sys : $(OBJS)
        @%create $*.lnk
        @%append $*.lnk name $@
        @%append $*.lnk format os2 lx physdevice
        @%append $*.lnk option mixed1632,nostub,align=512
        @%append $*.lnk option quiet,internalrelocs,verbose,nocaseexact,eliminate
        @%append $*.lnk option map=$*.map
        @%append $*.lnk option stack=0
        @%append $*.lnk sort global
        @%append $*.lnk option osname='eComStation'
        @AddToFile.cmd $*.lnk,option description,BLDLEVEL,$(VENDOR),$(VERSION),SysEnter/SysExit Ring-0 direct call mechanism
        @%append $*.lnk import DOS32FLATCS DOSCALLS.369
        @%append $*.lnk import DOS32FLATDS DOSCALLS.370
        @%append $*.lnk import KernRASSysTrace KEE.80
        @%append $*.lnk import KernThunkStackTo16 KEE.40
        @%append $*.lnk import KernThunkStackTo32 KEE.41
        @%append $*.lnk import KernLinToPageList KEE.64
        @%append $*.lnk import _KernSISData KEE.90
        @%append $*.lnk libpath $(WATCOM_LIB)
        @%append $*.lnk libpath $(DDK_LIB)
!ifdef DEBUG
        @%append $*.lnk option cvpack
        @%append $*.lnk debug codeview lines
!endif
        @%append $*.lnk lib os2
        @%append $*.lnk lib clibs
        @%append $*.lnk lib rmcalls
        @for %f in ($(OBJS)) do @%append $*.lnk file %f
        $(LINK) @$*.lnk
        @wat2map $*.map $*.mms
        @mapsym -n $*.mms
        @%erase $*.mms
        

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

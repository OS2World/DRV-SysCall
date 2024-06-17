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
!ifndef %WATCOM
!   error Define WATCOM env var to point to WATCOM installation !
!endif

!ifndef %DDK
!   error Define DDK env var to point to DDK installation !
!endif

!ifndef %VERMAJOR
!   error Missing major version info for build ! Specify VERMAJOR as env var !
!endif
!ifndef %VERMINOR
!   error Missing minor version info for build ! Specify VERMINOR as env var !
!endif
!ifndef %DAY
!   error Missing day info for build ! Specify DAY as env var !
!endif
!ifndef %MONTH
!   error Missing day info for build ! Specify MONTH as env var !
!endif
!ifndef %YEAR
!   error Missing day info for build ! Specify YEAR as env var !
!endif
!ifndef %VENDOR
!   error Missing vendor info for build ! Specify VENDOR as env var !
!endif

%PATH=..;$(%PATH);$(%WATCOM)\BINP
%INCLUDE=
%LIB=

all: .SYMBOLIC
    @echo $(__MAKEFILES__)
	cd driver
	wmake -h $(__MAKEOPTS__)
	cd..

	cd dll
	wmake -h $(__MAKEOPTS__)
	cd..

	cd app
	wmake -h $(__MAKEOPTS__)
	cd..

clean: .SYMBOLIC
	cd driver
	wmake -h clean
	cd..

	cd dll
	wmake -h clean
	cd..

	cd app
	wmake -h clean
	cd..



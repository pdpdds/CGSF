# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=libGiST - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to libGiST - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "libGiST - Win32 Release" && "$(CFG)" !=\
 "libGiST - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "libGiST.mak" CFG="libGiST - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libGiST - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libGiST - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
CPP=cl.exe

!IF  "$(CFG)" == "libGiST - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : ".\Release\libGiST.lib"

CLEAN : 
	-@erase ".\Release\GiST.obj"
	-@erase ".\Release\GiSTcursor.obj"
	-@erase ".\Release\GiSTfile.obj"
	-@erase ".\Release\GiSTdb.obj"
	-@erase ".\Release\GiSTnode.obj"
	-@erase ".\Release\GiSTpredicate.obj"
	-@erase ".\Release\libGiST.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /I "..\.." /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)/libGiST.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/libGiST.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/libGiST.lib" 
LIB32_OBJS= \
	".\Release\GiST.obj" \
	".\Release\GiSTcursor.obj" \
	".\Release\GiSTfile.obj" \
	".\Release\GiSTdb.obj" \
	".\Release\GiSTnode.obj" \
	".\Release\GiSTpredicate.obj"

".\Release\libGiST.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libGiST - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : ".\Debug\libGiST.lib"

CLEAN : 
	-@erase ".\Debug\GiST.obj"
	-@erase ".\Debug\GiSTcursor.obj"
	-@erase ".\Debug\GiSTfile.obj"
	-@erase ".\Debug\GiSTdb.obj"
	-@erase ".\Debug\GiSTnode.obj"
	-@erase ".\Debug\GiSTpredicate.obj"
	-@erase ".\Debug\libGiST.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /I..\.. /MTd /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)/libGiST.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/libGiST.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/libGiST.lib" 
LIB32_OBJS= \
	".\Debug\GiST.obj" \
	".\Debug\GiSTcursor.obj" \
	".\Debug\GiSTdb.obj" \
	".\Debug\GiSTfile.obj" \
	".\Debug\GiSTnode.obj" \
	".\Debug\GiSTpredicate.obj"

".\Debug\libGiST.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "libGiST - Win32 Release"
# Name "libGiST - Win32 Debug"

!IF  "$(CFG)" == "libGiST - Win32 Release"

!ELSEIF  "$(CFG)" == "libGiST - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\GiSTpredicate.cpp
DEP_CPP_GISTP=\
	".\GiST.h"\
	".\GiSTcursor.h"\
	".\GiSTdefs.h"\
	".\GiSTentry.h"\
	".\GiSTlist.h"\
	".\GiSTnode.h"\
	".\GiSTpath.h"\
	".\GiSTpredicate.h"\
	".\GiSTstore.h"\
	

!IF  "$(CFG)" == "libGiST - Win32 Release"


".\Release\GiSTpredicate.obj" : $(SOURCE) $(DEP_CPP_GISTP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "libGiST - Win32 Debug"


".\Debug\GiSTpredicate.obj" : $(SOURCE) $(DEP_CPP_GISTP) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GiSTcursor.cpp
DEP_CPP_GISTC=\
	".\GiST.h"\
	".\GiSTcursor.h"\
	".\GiSTdefs.h"\
	".\GiSTentry.h"\
	".\GiSTlist.h"\
	".\GiSTnode.h"\
	".\GiSTpath.h"\
	".\GiSTpredicate.h"\
	".\GiSTstore.h"\
	

!IF  "$(CFG)" == "libGiST - Win32 Release"


".\Release\GiSTcursor.obj" : $(SOURCE) $(DEP_CPP_GISTC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "libGiST - Win32 Debug"


".\Debug\GiSTcursor.obj" : $(SOURCE) $(DEP_CPP_GISTC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GiSTfile.cpp
DEP_CPP_GISTF=\
	".\GiSTdefs.h"\
	".\GiSTfile.h"\
	".\GiSTstore.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

!IF  "$(CFG)" == "libGiST - Win32 Release"


".\Release\GiSTfile.obj" : $(SOURCE) $(DEP_CPP_GISTF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "libGiST - Win32 Debug"


".\Debug\GiSTfile.obj" : $(SOURCE) $(DEP_CPP_GISTF) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GiSTdb.cpp
DEP_CPP_GISTDB=\
	".\GiSTdefs.h"\
	".\GiSTdb.h"\
	".\GiSTpath.h"\
	".\GiSTstore.h"\
	"..\..\fastdb.h"\
	"..\..\database.h"\
	"..\..\array.h"\
	"..\..\query.h"\
	"..\..\cursor.h"\
	"..\..\stdtp.h"\
	"..\..\class.h"\
	"..\..\reference.h"\
	"..\..\file.h"\
	"..\..\hashtab.h"\
	"..\..\ttree.h"\
	"..\..\sync.h"\
	"..\..\array.h"\
	

!IF  "$(CFG)" == "libGiST - Win32 Release"


".\Release\GiSTdb.obj" : $(SOURCE) $(DEP_CPP_GISTDB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "libGiST - Win32 Debug"


".\Debug\GiSTdb.obj" : $(SOURCE) $(DEP_CPP_GISTDB) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GiSTnode.cpp
DEP_CPP_GISTN=\
	".\GiST.h"\
	".\GiSTcursor.h"\
	".\GiSTdefs.h"\
	".\GiSTentry.h"\
	".\GiSTlist.h"\
	".\GiSTnode.h"\
	".\GiSTpath.h"\
	".\GiSTpredicate.h"\
	".\GiSTstore.h"\
	

!IF  "$(CFG)" == "libGiST - Win32 Release"


".\Release\GiSTnode.obj" : $(SOURCE) $(DEP_CPP_GISTN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "libGiST - Win32 Debug"


".\Debug\GiSTnode.obj" : $(SOURCE) $(DEP_CPP_GISTN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GiST.cpp
DEP_CPP_GIST_=\
	".\GiST.h"\
	".\GiSTcursor.h"\
	".\GiSTdefs.h"\
	".\GiSTentry.h"\
	".\GiSTlist.h"\
	".\GiSTnode.h"\
	".\GiSTpath.h"\
	".\GiSTpredicate.h"\
	".\GiSTstore.h"\
	

!IF  "$(CFG)" == "libGiST - Win32 Release"


".\Release\GiST.obj" : $(SOURCE) $(DEP_CPP_GIST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "libGiST - Win32 Debug"


".\Debug\GiST.obj" : $(SOURCE) $(DEP_CPP_GIST_) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################

# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=RSTree - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to RSTree - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "RSTree - Win32 Release" && "$(CFG)" != "RSTree - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "RSTree.mak" CFG="RSTree - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RSTree - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "RSTree - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "RSTree - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RSTree - Win32 Release"

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

ALL : "$(OUTDIR)\RSTree.exe"

CLEAN : 
	-@erase "$(INTDIR)\command.obj"
	-@erase "$(INTDIR)\gram.tab.obj"
	-@erase "$(INTDIR)\RT.obj"
	-@erase "$(INTDIR)\RTentry.obj"
	-@erase "$(INTDIR)\RTnode.obj"
	-@erase "$(INTDIR)\RTpredicate.obj"
	-@erase "$(INTDIR)\scan.obj"
	-@erase "$(OUTDIR)\RSTree.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\libGiST"  /I "..\.." /D "WIN32" /D "NDEBUG" /D "_CONSOLE"  /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\libGiST" /I "..\.." /D "WIN32" /D "NDEBUG" /D\
 "_CONSOLE"  /Fp"$(INTDIR)/RSTree.pch" /YX /Fo"$(INTDIR)/"\
 /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/RSTree.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libGiST\Release\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\libGiST\Release\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console\
 /incremental:no /pdb:"$(OUTDIR)/RSTree.pdb" /machine:I386\
 /out:"$(OUTDIR)/RSTree.exe" 
LINK32_OBJS= \
	"$(INTDIR)\command.obj" \
	"$(INTDIR)\gram.tab.obj" \
	"$(INTDIR)\RT.obj" \
	"$(INTDIR)\RTentry.obj" \
	"$(INTDIR)\RTnode.obj" \
	"$(INTDIR)\RTpredicate.obj" \
	"$(INTDIR)\scan.obj"

"$(OUTDIR)\RSTree.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "RSTree - Win32 Debug"

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

ALL : "$(OUTDIR)\RSTree.exe" "$(OUTDIR)\RSTree.bsc"

CLEAN : 
	-@erase "$(INTDIR)\command.obj"
	-@erase "$(INTDIR)\command.sbr"
	-@erase "$(INTDIR)\gram.tab.obj"
	-@erase "$(INTDIR)\gram.tab.sbr"
	-@erase "$(INTDIR)\RT.obj"
	-@erase "$(INTDIR)\RT.sbr"
	-@erase "$(INTDIR)\RTentry.obj"
	-@erase "$(INTDIR)\RTentry.sbr"
	-@erase "$(INTDIR)\RTnode.obj"
	-@erase "$(INTDIR)\RTnode.sbr"
	-@erase "$(INTDIR)\RTpredicate.obj"
	-@erase "$(INTDIR)\RTpredicate.sbr"
	-@erase "$(INTDIR)\scan.obj"
	-@erase "$(INTDIR)\scan.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\RSTree.bsc"
	-@erase "$(OUTDIR)\RSTree.exe"
	-@erase "$(OUTDIR)\RSTree.ilk"
	-@erase "$(OUTDIR)\RSTree.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\libGiST" /I "..\.." /D "WIN32" /D "_DEBUG" /D "_CONSOLE"  /FR /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\libGiST" /I "..\.." /D "WIN32" /D\
 "_DEBUG" /D "_CONSOLE" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/RSTree.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/RSTree.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\command.sbr" \
	"$(INTDIR)\gram.tab.sbr" \
	"$(INTDIR)\RT.sbr" \
	"$(INTDIR)\RTentry.sbr" \
	"$(INTDIR)\RTnode.sbr" \
	"$(INTDIR)\RTpredicate.sbr" \
	"$(INTDIR)\scan.sbr"

"$(OUTDIR)\RSTree.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libGiST\Debug\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\libGiST\Debug\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console\
 /incremental:yes /pdb:"$(OUTDIR)/RSTree.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/RSTree.exe" 
LINK32_OBJS= \
	"$(INTDIR)\command.obj" \
	"$(INTDIR)\gram.tab.obj" \
	"$(INTDIR)\RT.obj" \
	"$(INTDIR)\RTentry.obj" \
	"$(INTDIR)\RTnode.obj" \
	"$(INTDIR)\RTpredicate.obj" \
	"$(INTDIR)\scan.obj"

"$(OUTDIR)\RSTree.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
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

# Name "RSTree - Win32 Release"
# Name "RSTree - Win32 Debug"

!IF  "$(CFG)" == "RSTree - Win32 Release"

!ELSEIF  "$(CFG)" == "RSTree - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\RTnode.cpp
DEP_CPP_RTNOD=\
	"..\libGiST\GiST.h"\
	"..\libGiST\GiSTcursor.h"\
	"..\libGiST\GiSTdefs.h"\
	"..\libGiST\GiSTentry.h"\
	"..\libGiST\GiSTdb.h"\
	"..\libGiST\GiSTlist.h"\
	"..\libGiST\GiSTnode.h"\
	"..\libGiST\GiSTpath.h"\
	"..\libGiST\GiSTpredicate.h"\
	"..\libGiST\GiSTstore.h"\
	".\RT.h"\
	".\RTentry.h"\
	".\RTnode.h"\
	".\RTpredicate.h"\
	

!IF  "$(CFG)" == "RSTree - Win32 Release"


"$(INTDIR)\RTnode.obj" : $(SOURCE) $(DEP_CPP_RTNOD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RSTree - Win32 Debug"


"$(INTDIR)\RTnode.obj" : $(SOURCE) $(DEP_CPP_RTNOD) "$(INTDIR)"

"$(INTDIR)\RTnode.sbr" : $(SOURCE) $(DEP_CPP_RTNOD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gram.tab.cpp
DEP_CPP_GRAM_=\
	"..\libGiST\GiST.h"\
	"..\libGiST\GiSTcursor.h"\
	"..\libGiST\GiSTdefs.h"\
	"..\libGiST\GiSTentry.h"\
	"..\libGiST\GiSTdb.h"\
	"..\libGiST\GiSTlist.h"\
	"..\libGiST\GiSTnode.h"\
	"..\libGiST\GiSTpath.h"\
	"..\libGiST\GiSTpredicate.h"\
	"..\libGiST\GiSTstore.h"\
	".\command.h"\
	".\RT.h"\
	".\RTentry.h"\
	".\RTnode.h"\
	".\RTpredicate.h"\
	

!IF  "$(CFG)" == "RSTree - Win32 Release"


"$(INTDIR)\gram.tab.obj" : $(SOURCE) $(DEP_CPP_GRAM_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RSTree - Win32 Debug"


"$(INTDIR)\gram.tab.obj" : $(SOURCE) $(DEP_CPP_GRAM_) "$(INTDIR)"

"$(INTDIR)\gram.tab.sbr" : $(SOURCE) $(DEP_CPP_GRAM_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RT.cpp
DEP_CPP_RT_CP=\
	"..\libGiST\GiST.h"\
	"..\libGiST\GiSTcursor.h"\
	"..\libGiST\GiSTdefs.h"\
	"..\libGiST\GiSTentry.h"\
	"..\libGiST\GiSTdb.h"\
	"..\libGiST\GiSTlist.h"\
	"..\libGiST\GiSTnode.h"\
	"..\libGiST\GiSTpath.h"\
	"..\libGiST\GiSTpredicate.h"\
	"..\libGiST\GiSTstore.h"\
	".\RT.h"\
	".\RTentry.h"\
	".\RTnode.h"\
	".\RTpredicate.h"\
	

!IF  "$(CFG)" == "RSTree - Win32 Release"


"$(INTDIR)\RT.obj" : $(SOURCE) $(DEP_CPP_RT_CP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RSTree - Win32 Debug"


"$(INTDIR)\RT.obj" : $(SOURCE) $(DEP_CPP_RT_CP) "$(INTDIR)"

"$(INTDIR)\RT.sbr" : $(SOURCE) $(DEP_CPP_RT_CP) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RTentry.cpp
DEP_CPP_RTENT=\
	"..\libGiST\GiST.h"\
	"..\libGiST\GiSTcursor.h"\
	"..\libGiST\GiSTdefs.h"\
	"..\libGiST\GiSTentry.h"\
	"..\libGiST\GiSTdb.h"\
	"..\libGiST\GiSTlist.h"\
	"..\libGiST\GiSTnode.h"\
	"..\libGiST\GiSTpath.h"\
	"..\libGiST\GiSTpredicate.h"\
	"..\libGiST\GiSTstore.h"\
	".\RT.h"\
	".\RTentry.h"\
	".\RTnode.h"\
	".\RTpredicate.h"\
	

!IF  "$(CFG)" == "RSTree - Win32 Release"


"$(INTDIR)\RTentry.obj" : $(SOURCE) $(DEP_CPP_RTENT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RSTree - Win32 Debug"


"$(INTDIR)\RTentry.obj" : $(SOURCE) $(DEP_CPP_RTENT) "$(INTDIR)"

"$(INTDIR)\RTentry.sbr" : $(SOURCE) $(DEP_CPP_RTENT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\command.cpp
DEP_CPP_COMMA=\
	"..\libGiST\GiST.h"\
	"..\libGiST\GiSTcursor.h"\
	"..\libGiST\GiSTdefs.h"\
	"..\libGiST\GiSTentry.h"\
	"..\libGiST\GiSTdb.h"\
	"..\libGiST\GiSTlist.h"\
	"..\libGiST\GiSTnode.h"\
	"..\libGiST\GiSTpath.h"\
	"..\libGiST\GiSTpredicate.h"\
	"..\libGiST\GiSTstore.h"\
	".\RT.h"\
	".\RTentry.h"\
	".\RTnode.h"\
	".\RTpredicate.h"\
	

!IF  "$(CFG)" == "RSTree - Win32 Release"


"$(INTDIR)\command.obj" : $(SOURCE) $(DEP_CPP_COMMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RSTree - Win32 Debug"


"$(INTDIR)\command.obj" : $(SOURCE) $(DEP_CPP_COMMA) "$(INTDIR)"

"$(INTDIR)\command.sbr" : $(SOURCE) $(DEP_CPP_COMMA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RTpredicate.cpp
DEP_CPP_RTPRE=\
	"..\libGiST\GiST.h"\
	"..\libGiST\GiSTcursor.h"\
	"..\libGiST\GiSTdefs.h"\
	"..\libGiST\GiSTentry.h"\
	"..\libGiST\GiSTdb.h"\
	"..\libGiST\GiSTlist.h"\
	"..\libGiST\GiSTnode.h"\
	"..\libGiST\GiSTpath.h"\
	"..\libGiST\GiSTpredicate.h"\
	"..\libGiST\GiSTstore.h"\
	".\RT.h"\
	".\RTentry.h"\
	".\RTnode.h"\
	".\RTpredicate.h"\
	

!IF  "$(CFG)" == "RSTree - Win32 Release"


"$(INTDIR)\RTpredicate.obj" : $(SOURCE) $(DEP_CPP_RTPRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RSTree - Win32 Debug"


"$(INTDIR)\RTpredicate.obj" : $(SOURCE) $(DEP_CPP_RTPRE) "$(INTDIR)"

"$(INTDIR)\RTpredicate.sbr" : $(SOURCE) $(DEP_CPP_RTPRE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\scan.cpp
DEP_CPP_SCAN_=\
	"..\libGiST\GiST.h"\
	"..\libGiST\GiSTcursor.h"\
	"..\libGiST\GiSTdefs.h"\
	"..\libGiST\GiSTentry.h"\
	"..\libGiST\GiSTdb.h"\
	"..\libGiST\GiSTlist.h"\
	"..\libGiST\GiSTnode.h"\
	"..\libGiST\GiSTpath.h"\
	"..\libGiST\GiSTpredicate.h"\
	"..\libGiST\GiSTstore.h"\
	".\gram.tab.h"\
	".\RT.h"\
	".\RTentry.h"\
	".\RTnode.h"\
	".\RTpredicate.h"\
	

!IF  "$(CFG)" == "RSTree - Win32 Release"


"$(INTDIR)\scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RSTree - Win32 Debug"


"$(INTDIR)\scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"

"$(INTDIR)\scan.sbr" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################

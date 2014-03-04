# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=RTree - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to RTree - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "RTree - Win32 Release" && "$(CFG)" != "RTree - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "RTree.mak" CFG="RTree - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RTree - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "RTree - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "RTree - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RTree - Win32 Release"

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

ALL : ".\Release\RTree.exe"

CLEAN : 
	-@erase ".\Release\command.obj"
	-@erase ".\Release\gram.tab.obj"
	-@erase ".\Release\RTentry.obj"
	-@erase ".\Release\RTnode.obj"
	-@erase ".\Release\RTpredicate.obj"
	-@erase ".\Release\RTree.exe"
	-@erase ".\Release\scan.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "..\libGiST"  /I "..\.."  /D "NDEBUG" /D "WIN32" /D "_CONSOLE"  /YX /c
CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /I "..\libGiST"  /I "..\.." /D "NDEBUG" /D "WIN32" /D\
 "_CONSOLE"  /Fp"$(INTDIR)/RTree.pch" /YX /Fo"$(INTDIR)/"\
 /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/RTree.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libGiST\Release\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console /profile /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\libGiST\Release\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console /profile\
 /machine:I386 /out:"$(OUTDIR)/RTree.exe" 
LINK32_OBJS= \
	".\Release\command.obj" \
	".\Release\gram.tab.obj" \
	".\Release\RTentry.obj" \
	".\Release\RTnode.obj" \
	".\Release\RTpredicate.obj" \
	".\Release\scan.obj"

".\Release\RTree.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "RTree - Win32 Debug"

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

ALL : ".\Debug\RTree.exe"

CLEAN : 
	-@erase ".\Debug\command.obj"
	-@erase ".\Debug\gram.tab.obj"
	-@erase ".\Debug\RTentry.obj"
	-@erase ".\Debug\RTnode.obj"
	-@erase ".\Debug\RTpredicate.obj"
	-@erase ".\Debug\RTree.exe"
	-@erase ".\Debug\RTree.ilk"
	-@erase ".\Debug\RTree.pdb"
	-@erase ".\Debug\scan.obj"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\vc40.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GR /GX /Zi /Od /I "..\libGiST" /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE"  /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\libGiST" /I "..\.." /D "_DEBUG" /D\
 "WIN32" /D "_CONSOLE"  /Fp"$(INTDIR)/RTree.pch" /YX\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/RTree.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libGiST\debug\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\libGiST\debug\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console\
 /incremental:yes /pdb:"$(OUTDIR)/RTree.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/RTree.exe" 
LINK32_OBJS= \
	".\Debug\command.obj" \
	".\Debug\gram.tab.obj" \
	".\Debug\RTentry.obj" \
	".\Debug\RTnode.obj" \
	".\Debug\RTpredicate.obj" \
	".\Debug\scan.obj"

".\Debug\RTree.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "RTree - Win32 Release"
# Name "RTree - Win32 Debug"

!IF  "$(CFG)" == "RTree - Win32 Release"

!ELSEIF  "$(CFG)" == "RTree - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\RTpredicate.cpp

!IF  "$(CFG)" == "RTree - Win32 Release"

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
	

".\Release\RTpredicate.obj" : $(SOURCE) $(DEP_CPP_RTPRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RTree - Win32 Debug"

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
	

".\Debug\RTpredicate.obj" : $(SOURCE) $(DEP_CPP_RTPRE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RTentry.cpp

!IF  "$(CFG)" == "RTree - Win32 Release"

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
	

".\Release\RTentry.obj" : $(SOURCE) $(DEP_CPP_RTENT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RTree - Win32 Debug"

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
	

".\Debug\RTentry.obj" : $(SOURCE) $(DEP_CPP_RTENT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RTnode.cpp

!IF  "$(CFG)" == "RTree - Win32 Release"

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
	

".\Release\RTnode.obj" : $(SOURCE) $(DEP_CPP_RTNOD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RTree - Win32 Debug"

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
	

".\Debug\RTnode.obj" : $(SOURCE) $(DEP_CPP_RTNOD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\command.cpp

!IF  "$(CFG)" == "RTree - Win32 Release"

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
	

".\Release\command.obj" : $(SOURCE) $(DEP_CPP_COMMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RTree - Win32 Debug"

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
	

".\Debug\command.obj" : $(SOURCE) $(DEP_CPP_COMMA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\scan.cpp

!IF  "$(CFG)" == "RTree - Win32 Release"

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
	

".\Release\scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RTree - Win32 Debug"

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
	

".\Debug\scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gram.tab.cpp

!IF  "$(CFG)" == "RTree - Win32 Release"

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
	

".\Release\gram.tab.obj" : $(SOURCE) $(DEP_CPP_GRAM_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "RTree - Win32 Debug"

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
	

".\Debug\gram.tab.obj" : $(SOURCE) $(DEP_CPP_GRAM_) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################

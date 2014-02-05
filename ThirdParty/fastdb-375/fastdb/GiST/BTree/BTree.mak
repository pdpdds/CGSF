# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=BTree - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to BTree - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "BTree - Win32 Release" && "$(CFG)" != "BTree - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "BTree.mak" CFG="BTree - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BTree - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "BTree - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "BTree - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BTree - Win32 Release"

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

ALL : ".\Release\BTree.exe"

CLEAN : 
	-@erase ".\Release\BTentry.obj"
	-@erase ".\Release\BTnode.obj"
	-@erase ".\Release\BTpredicate.obj"
	-@erase ".\Release\BTree.exe"
	-@erase ".\Release\command.obj"
	-@erase ".\Release\gram.tab.obj"
	-@erase ".\Release\scan.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\libGiST" /I "..\.." /D "WIN32" /D "NDEBUG" /D "_CONSOLE"  /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\libGiST" /I "..\.." /D "WIN32" /D "NDEBUG" /D\
 "_CONSOLE"  /Fp"$(INTDIR)/BTree.pch" /YX /Fo"$(INTDIR)/"\
 /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/BTree.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libGiST\Release\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\libGiST\Release\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console\
 /incremental:no /pdb:"$(OUTDIR)/BTree.pdb" /machine:I386\
 /out:"$(OUTDIR)/BTree.exe" 
LINK32_OBJS= \
	".\Release\BTentry.obj" \
	".\Release\BTnode.obj" \
	".\Release\BTpredicate.obj" \
	".\Release\command.obj" \
	".\Release\gram.tab.obj" \
	".\Release\scan.obj"

".\Release\BTree.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BTree - Win32 Debug"

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

ALL : ".\Debug\BTree.exe"

CLEAN : 
	-@erase ".\Debug\BTentry.obj"
	-@erase ".\Debug\BTnode.obj"
	-@erase ".\Debug\BTpredicate.obj"
	-@erase ".\Debug\BTree.exe"
	-@erase ".\Debug\BTree.ilk"
	-@erase ".\Debug\BTree.pdb"
	-@erase ".\Debug\command.obj"
	-@erase ".\Debug\gram.tab.obj"
	-@erase ".\Debug\scan.obj"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\vc40.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\libGiST" /I "..\.." /D "WIN32" /D "_DEBUG" /D "_CONSOLE"  /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\libGiST" /I "..\.." /D "WIN32" /D\
 "_DEBUG" /D "_CONSOLE"  /Fp"$(INTDIR)/BTree.pch" /YX\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/BTree.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libGiST\debug\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\libGiST\debug\libGiST.lib ..\..\fastdb.lib /nologo /subsystem:console\
 /incremental:yes /pdb:"$(OUTDIR)/BTree.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/BTree.exe" 
LINK32_OBJS= \
	".\Debug\BTentry.obj" \
	".\Debug\BTnode.obj" \
	".\Debug\BTpredicate.obj" \
	".\Debug\command.obj" \
	".\Debug\gram.tab.obj" \
	".\Debug\scan.obj"

".\Debug\BTree.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "BTree - Win32 Release"
# Name "BTree - Win32 Debug"

!IF  "$(CFG)" == "BTree - Win32 Release"

!ELSEIF  "$(CFG)" == "BTree - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\scan.cpp

!IF  "$(CFG)" == "BTree - Win32 Release"

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
	".\BT.h"\
	".\BTentry.h"\
	".\BTnode.h"\
	".\BTpredicate.h"\
	".\gram.tab.h"\
	

".\Release\scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "BTree - Win32 Debug"

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
	".\BT.h"\
	".\BTentry.h"\
	".\BTnode.h"\
	".\BTpredicate.h"\
	".\gram.tab.h"\
	

".\Debug\scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BTnode.cpp
DEP_CPP_BTNOD=\
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
	".\BT.h"\
	".\BTentry.h"\
	".\BTnode.h"\
	".\BTpredicate.h"\
	

!IF  "$(CFG)" == "BTree - Win32 Release"


".\Release\BTnode.obj" : $(SOURCE) $(DEP_CPP_BTNOD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "BTree - Win32 Debug"


".\Debug\BTnode.obj" : $(SOURCE) $(DEP_CPP_BTNOD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BTpredicate.cpp
DEP_CPP_BTPRE=\
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
	".\BT.h"\
	".\BTentry.h"\
	".\BTnode.h"\
	".\BTpredicate.h"\
	

!IF  "$(CFG)" == "BTree - Win32 Release"


".\Release\BTpredicate.obj" : $(SOURCE) $(DEP_CPP_BTPRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "BTree - Win32 Debug"


".\Debug\BTpredicate.obj" : $(SOURCE) $(DEP_CPP_BTPRE) "$(INTDIR)"


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
	".\BT.h"\
	".\BTentry.h"\
	".\BTnode.h"\
	".\BTpredicate.h"\
	

!IF  "$(CFG)" == "BTree - Win32 Release"


".\Release\command.obj" : $(SOURCE) $(DEP_CPP_COMMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "BTree - Win32 Debug"


".\Debug\command.obj" : $(SOURCE) $(DEP_CPP_COMMA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gram.tab.cpp

!IF  "$(CFG)" == "BTree - Win32 Release"

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
	".\BT.h"\
	".\BTentry.h"\
	".\BTnode.h"\
	".\BTpredicate.h"\
	".\command.h"\
	

".\Release\gram.tab.obj" : $(SOURCE) $(DEP_CPP_GRAM_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "BTree - Win32 Debug"

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
	".\BT.h"\
	".\BTentry.h"\
	".\BTnode.h"\
	".\BTpredicate.h"\
	".\command.h"\
	

".\Debug\gram.tab.obj" : $(SOURCE) $(DEP_CPP_GRAM_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BTentry.cpp
DEP_CPP_BTENT=\
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
	".\BT.h"\
	".\BTentry.h"\
	".\BTnode.h"\
	".\BTpredicate.h"\
	

!IF  "$(CFG)" == "BTree - Win32 Release"


".\Release\BTentry.obj" : $(SOURCE) $(DEP_CPP_BTENT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "BTree - Win32 Debug"


".\Debug\BTentry.obj" : $(SOURCE) $(DEP_CPP_BTENT) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################

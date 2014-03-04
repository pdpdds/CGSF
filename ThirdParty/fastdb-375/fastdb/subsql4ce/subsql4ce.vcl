<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: subsql4ce - Win32 (WCE x86) Debug--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\DOCUME~1\Knizhnik\LOCALS~1\Temp\RSP2EF.tmp" with contents
[
/nologo /W3 /Zi /Od /I "..\inc" /D "DEBUG" /D "_i386_" /D UNDER_CE=400 /D _WIN32_WCE=400 /D "WCE_PLATFORM_STANDARDSDK" /D "i_386_" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /Fp"X86Dbg/subsql4ce.pch" /YX /Fo"X86Dbg/" /Fd"X86Dbg/" /Gs8192 /GF /c 
"C:\fastdb\src\server.cpp"
"C:\fastdb\src\subsql.cpp"
]
Creating command line "cl.exe @C:\DOCUME~1\Knizhnik\LOCALS~1\Temp\RSP2EF.tmp" 
Creating temporary file "C:\DOCUME~1\Knizhnik\LOCALS~1\Temp\RSP2F0.tmp" with contents
[
corelibc.lib ..\fastdb.lib commctrl.lib coredll.lib winsock.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:yes /pdb:"X86Dbg/subsql.pdb" /debug /nodefaultlib:"OLDNAMES.lib" /nodefaultlib:libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /out:"subsql.exe" /subsystem:windowsce,4.00 /MACHINE:IX86 
.\X86Dbg\server.obj
.\X86Dbg\subsql.obj
]
Creating command line "link.exe @C:\DOCUME~1\Knizhnik\LOCALS~1\Temp\RSP2F0.tmp"
<h3>Output Window</h3>
Compiling...
server.cpp
C:\fastdb\src\server.cpp(1324) : warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify -GX
subsql.cpp
C:\fastdb\src\subsql.cpp(197) : warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify -GX
C:\Program Files\Windows CE Tools\wce400\STANDARDSDK\Include\x86\xstring(724) : warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify -GX
        C:\Program Files\Windows CE Tools\wce400\STANDARDSDK\Include\x86\xstring(720) : while compiling class-template member function 'void __thiscall std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >::_Copy(unsigned int)'
Linking...




<h3>Results</h3>
subsql.exe - 0 error(s), 3 warning(s)
</pre>
</body>
</html>

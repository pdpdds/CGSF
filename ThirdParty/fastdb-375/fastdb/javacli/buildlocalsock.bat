if not defined JAVA_HOME set JAVA_HOME=\jdk1.3
if not defined PACKAGE_NAME set PACKAGE_NAME=javacli

cl -LD  -Zi -W3 -MTd -I.. -DPACKAGE_NAME="\"%PACKAGE_NAME%\"" -I%JAVA_HOME%\include  -I%JAVA_HOME%\include\win32 jnilocalsock.cpp ..\gigabase.lib wsock32.lib %JAVA_HOME%\lib\jvm.lib


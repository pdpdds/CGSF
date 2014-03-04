if not defined JAVA_HOME set JAVA_HOME=\jdk1.3

javac -g *.java
jar cvf jnicli.jar *.class

cl -LD  -Zi -W3 -EHsc -MTd -I../inc  -I"%JAVA_HOME%\include"  -I"%JAVA_HOME%\include\win32" jnicli.cpp ..\fastdb.lib "%JAVA_HOME%\lib\jvm.lib"

# -*- makefile -*-
# Makefile for Microsoft Windows with Intel C++ Compiler

GENERATE_DLL=0
FAULT_TOLERANT=0

OBJS = class.obj compiler.obj database.obj xml.obj hashtab.obj file.obj \
	sync.obj symtab.obj ttree.obj rtree.obj container.obj cursor.obj query.obj w32sock.obj \
	wwwapi.obj localcli.obj stdtp.obj server.obj

CLI_OBJS = cli.obj w32sock.obj sync.obj repsock.obj stdtp.obj

INCS = inc/fastdb.h inc/stdtp.h inc/class.h inc/database.h inc/cursor.h inc/reference.h inc/wwwapi.h \
	inc/array.h inc/file.h inc/hashtab.h inc/ttree.h inc/rtree.h inc/container.h inc/sync.h inc/sync_w32.h inc/config.h \
	inc/query.h inc/datetime.h inc/harray.h inc/timeseries.h inc/rectangle.h inc/exception.h

FASTDB_LIB = fastdb.lib 
CLI_LIB = cli.lib
FASTDB_DLL = fastdb.dll 
CLI_DLL = cli.dll

EXES = subsql.exe cgistub.exe forcerecovery.exe

EXAMPLES = guess.exe guess2.exe guess_std.exe testdb.exe testleak.exe testjoin.exe testddl.exe testperf.exe testsync.exe testconc.exe testiref.exe testtrav.exe testidx.exe bugdb.exe clidb.exe clitest.exe clitest2.exe localclitest.exe testraw.exe testharr.exe testspat.exe testtl.exe

CC = icl

# DEFS macro is deprecatred, edit inc/config.h file instead
DEFS = 

CFLAGS = -c  -Iinc -nologo -Zi -W3 -GX -MTd $(DEFS)
#CFLAGS = -c  -Iinc -nologo -Od -Zi -W3 -MTd -GX -DFASTDB_DEBUG=DEBUG_TRACE $(DEFS)
#CFLAGS = -c  -Iinc -Ox -G6 -nologo -Zi -W3 -GX -MT $(DEFS)

LD = $(CC)
LDFLAGS = -MTd -Zi -nologo
#LDFLAGS = -MT -Zi -nologo

#DLLFLAGS = -LD -nologo
DLLFLAGS = -LDd -Zi -nologo

AR = lib
ARFLAGS = 

STDLIBS = advapi32.lib

SOCKLIBS = wsock32.lib

!IF $(FAULT_TOLERANT)
DEFS = $(DEFS) -DREPLICATION_SUPPORT
STDLIBS = $(STDLIBS) $(SOCKLIBS)
!ENDIF


!IF $(GENERATE_DLL)
all: $(FASTDB_LIB) $(CLI_LIB) $(FASTDB_DLL) $(CLI_DLL) $(EXES) $(EXAMPLES)
DEFS = $(DEFS) -DFASTDB_DLL
!ELSE
all: $(FASTDB_LIB) $(CLI_LIB) $(EXES) $(EXAMPLES)
!ENDIF

class.obj: src/class.cpp inc/compiler.h inc/compiler.d inc/symtab.h $(INCS)
	$(CC) $(CFLAGS) src/class.cpp

compiler.obj: src/compiler.cpp inc/compiler.h inc/compiler.d inc/symtab.h $(INCS)
	$(CC) $(CFLAGS) src/compiler.cpp

query.obj: src/query.cpp inc/compiler.h inc/compiler.d inc/symtab.h $(INCS)
	$(CC) $(CFLAGS) src/query.cpp

database.obj: src/database.cpp inc/compiler.h inc/compiler.d inc/symtab.h $(INCS)
	$(CC) $(CFLAGS) src/database.cpp

xml.obj: src/xml.cpp inc/compiler.h inc/compiler.d inc/symtab.h $(INCS)
	$(CC) $(CFLAGS) src/xml.cpp

localcli.obj: src/localcli.cpp inc/compiler.h inc/compiler.d inc/symtab.h inc/localcli.h $(INCS)
	$(CC) $(CFLAGS) src/localcli.cpp

cursor.obj: src/cursor.cpp inc/compiler.h inc/compiler.d $(INCS)
	$(CC) $(CFLAGS) src/cursor.cpp

hashtab.obj: src/hashtab.cpp $(INCS)
	$(CC) $(CFLAGS) src/hashtab.cpp

file.obj: src/file.cpp $(INCS)
	$(CC) $(CFLAGS) src/file.cpp

stdtp.obj: src/stdtp.cpp $(INCS)
	$(CC) $(CFLAGS) src/stdtp.cpp

sync.obj: src/sync.cpp $(INCS)
	$(CC) $(CFLAGS) src/sync.cpp

symtab.obj: src/symtab.cpp inc/symtab.h $(INCS)
	$(CC) $(CFLAGS) src/symtab.cpp

ttree.obj: src/ttree.cpp $(INCS)
	$(CC) $(CFLAGS) src/ttree.cpp

rtree.obj: src/rtree.cpp $(INCS)
	$(CC) $(CFLAGS) src/rtree.cpp

container.obj: src/container.cpp $(INCS)
	$(CC) $(CFLAGS) src/container.cpp

wwwapi.obj: src/wwwapi.cpp inc/wwwapi.h inc/stdtp.h inc/sync.h inc/sockio.h inc/sync_w32.h inc/config.h
	$(CC) $(CFLAGS) src/wwwapi.cpp

w32sock.obj: src/w32sock.cpp inc/w32sock.h inc/sockio.h inc/stdtp.h
	$(CC) $(CFLAGS) src/w32sock.cpp

repsock.obj: src/repsock.cpp inc/repsock.h inc/sockio.h inc/stdtp.h inc/sync.h inc/sync_w32.h inc/config.h
	$(CC) $(CFLAGS) src/repsock.cpp

cli.obj: src/cli.cpp inc/cli.h inc/cliproto.h inc/repsock.h inc/sockio.h inc/stdtp.h inc/sync.h inc/sync_w32.h inc/config.h
	$(CC) $(CFLAGS) src/cli.cpp


fastdb.lib: $(OBJS)
	$(AR) $(ARFLAGS) /OUT:fastdb.lib $(OBJS)

fastdb.dll: $(OBJS)
	$(CC) $(DLLFLAGS) /Fefastdb.dll $(OBJS) $(STDLIBS) $(SOCKLIBS)

cli.lib: $(CLI_OBJS)
	$(AR) $(ARFLAGS) /OUT:cli.lib $(CLI_OBJS)

cli.dll: $(CLI_OBJS)
	$(CC) $(DLLFLAGS) /Fecli.dll $(CLI_OBJS) $(STDLIBS) $(SOCKLIBS)

subsql.obj: src/subsql.cpp inc/compiler.h inc/server.h inc/compiler.d inc/subsql.h inc/symtab.h $(INCS)
	$(CC) $(CFLAGS) src/subsql.cpp

server.obj: src/server.cpp inc/server.h inc/compiler.h inc/compiler.d inc/subsql.h inc/symtab.h $(INCS)
	$(CC) $(CFLAGS) src/server.cpp

subsql.exe: subsql.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) subsql.obj $(FASTDB_LIB) $(STDLIBS) $(SOCKLIBS)


clitest.obj: examples/clitest.c inc/cli.h
	$(CC) $(CFLAGS) examples/clitest.c

clitest.exe: clitest.obj $(CLI_LIB)
	$(LD) $(LDFLAGS) clitest.obj $(CLI_LIB) $(STDLIBS) $(SOCKLIBS)

clitest2.obj: examples/clitest2.c inc/cli.h
	$(CC) $(CFLAGS) examples/clitest2.c

clitest2.exe: clitest2.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) /Feclitest2.exe clitest2.obj $(FASTDB_LIB) $(STDLIBS)

localclitest.exe: clitest.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) /Felocalclitest.exe clitest.obj $(FASTDB_LIB) $(STDLIBS)

bugdb.obj: examples/bugdb.cpp examples/bugdb.h $(INCS)
	$(CC) $(CFLAGS) examples/bugdb.cpp

bugdb.exe: bugdb.obj  $(FASTDB_LIB)
	$(LD) $(LDFLAGS) bugdb.obj $(FASTDB_LIB) $(STDLIBS) $(SOCKLIBS)


clidb.obj: examples/clidb.cpp examples/clidb.h $(INCS)
	$(CC) $(CFLAGS) examples/clidb.cpp

clidb.exe: clidb.obj  $(FASTDB_LIB)
	$(LD) $(LDFLAGS) clidb.obj $(FASTDB_LIB) $(STDLIBS) $(SOCKLIBS)


cgistub.obj: src/cgistub.cpp inc/stdtp.h inc/sockio.h
	$(CC) $(CFLAGS) src/cgistub.cpp 

cgistub.exe: cgistub.obj
	$(LD) $(LDFLAGS) cgistub.obj $(FASTDB_LIB) $(STDLIBS) $(SOCKLIBS)


forcerecovery.obj: src/forcerecovery.cpp inc/database.h
	$(CC) $(CFLAGS) src/forcerecovery.cpp 

forcerecovery.exe: forcerecovery.obj
	$(LD) $(LDFLAGS) forcerecovery.obj


guess.obj: examples/guess.cpp $(INCS)
	$(CC) $(CFLAGS) examples/guess.cpp 

guess.exe: guess.obj $(FASTDB_LIB)
	$(LD) -Feguess.exe $(LDFLAGS) guess.obj $(FASTDB_LIB) $(STDLIBS)

testharr.obj: examples/testharr.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testharr.cpp 

testharr.exe: testharr.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testharr.obj $(FASTDB_LIB) $(STDLIBS)

guess_std.obj: examples/guess_std.cpp $(INCS)
	$(CC) $(CFLAGS) examples/guess_std.cpp 

guess_std.exe: guess_std.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) guess_std.obj $(FASTDB_LIB) $(STDLIBS)

guess2.obj: examples/guess2.cpp $(INCS)
	$(CC) $(CFLAGS) examples/guess2.cpp 

guess2.exe: guess2.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) guess2.obj $(FASTDB_LIB) $(STDLIBS)

testdb.obj: examples/testdb.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testdb.cpp 

testdb.exe: testdb.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testdb.obj $(FASTDB_LIB) $(STDLIBS)

testraw.obj: examples/testraw.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testraw.cpp 

testraw.exe: testraw.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testraw.obj $(FASTDB_LIB) $(STDLIBS)

testleak.obj: examples/testleak.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testleak.cpp 

testleak.exe: testleak.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testleak.obj $(FASTDB_LIB) $(STDLIBS)

testjoin.obj: examples/testjoin.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testjoin.cpp 

testjoin.exe: testjoin.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testjoin.obj $(FASTDB_LIB) $(STDLIBS)

testddl.obj: examples/testddl.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testddl.cpp 

testddl.exe: testddl.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testddl.obj $(FASTDB_LIB) $(STDLIBS)

testperf.obj: examples/testperf.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testperf.cpp 

testperf.exe: testperf.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testperf.obj $(FASTDB_LIB) $(STDLIBS)

testidx.obj: examples/testidx.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testidx.cpp 

testidx.exe: testidx.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testidx.obj $(FASTDB_LIB) $(STDLIBS)

testtl.obj: examples/testtl.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testtl.cpp 

testtl.exe: testtl.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testtl.obj $(FASTDB_LIB) $(STDLIBS)

testspat.obj: examples/testspat.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testspat.cpp 

testspat.exe: testspat.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testspat.obj $(FASTDB_LIB) $(STDLIBS)

testsync.obj: examples/testsync.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testsync.cpp 

testsync.exe: testsync.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testsync.obj $(FASTDB_LIB) $(STDLIBS)

testconc.obj: examples/testconc.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testconc.cpp 

testconc.exe: testconc.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testconc.obj $(FASTDB_LIB) $(STDLIBS)

testiref.obj: examples/testiref.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testiref.cpp 

testiref.exe: testiref.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testiref.obj $(FASTDB_LIB) $(STDLIBS)

testtrav.obj: examples/testtrav.cpp $(INCS)
	$(CC) $(CFLAGS) examples/testtrav.cpp 

testtrav.exe: testtrav.obj $(FASTDB_LIB)
	$(LD) $(LDFLAGS) testtrav.obj $(FASTDB_LIB) $(STDLIBS)



cleanobj:
	-del *.odb,*.exp,*.obj,*.pch,*.pdb,*.ilk,*.dsp,*.dsw,*.ncb,*.opt

cleandbs:
	-del *.fdb

clean: cleanobj cleandbs
	-del *~,*.lib,*.dll,*.exe


zip: clean
	cd ..
	-del fastdb.zip
        zip -r fastdb.zip fastdb

copyzip: zip
	copy fastdb.zip d:


doxygen:
	doxygen doxygen.cfg


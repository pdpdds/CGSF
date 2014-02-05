#
CC=mcs
#
CFLAGS= -unsafe 
DEBUG= -g
#
LIB_TARGET=FastDbNet
LIB_FILES=FastDbCli.cs FastDbFields.cs FastDbCommand.cs FastDbConnection.cs AssemblyInfo.cs
LIB_DEFS= -define:LINUX
#
all:	$(LIB_TARGET) test

clean:
		rm $(LIB_TARGET).dll *.fdb

$(LIB_TARGET):	$(LIB_FILES)
		$(CC) $(CFLAGS) $(DEBUG) -target:library -out:$(LIB_TARGET).dll $(LIB_DEFS) $(LIB_FILES)

test:	$(LIB_TARGET).dll TestIndex.cs
		$(CC) $(DEBUG) -target:exe -out:TestIndex.exe -lib:./ -reference:$(LIB_TARGET).dll TestIndex.cs 

#
.SUFFIXES : .cs

#         Makefile for GiST test program
#
# Generalized Search Tree

PROGRAM = RTree

SRCS = command.cpp gram.y scan.l RTentry.cpp RTnode.cpp RTpredicate.cpp 
       
OBJS = command.o gram.tab.o scan.o RTentry.o RTnode.o RTpredicate.o 

DEST = .

$(PROGRAM): $(OBJS) ../libGiST .depend
	$(LINKER) -o $(PROGRAM) $(LDFLAGS) $(OBJS) $(LIBS)

.depend:
	$(MAKE) depend
	touch .depend

gram.tab.cpp: gram.y
	$(YACC) $(YACCFLAGS) gram.y
	mv gram.tab.c gram.tab.cpp

scan.cpp: scan.l
	$(LEX) $(LEXFLAGS) scan.l >scan.cpp

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCS) $*.cpp

clean:
	rm -f $(OBJS) $(PROGRAM) gram.tab.cpp scan.cpp gram.tab.h gram.output *~ #*# *pure* TAGS

depend:
	makedepend $(DEFINES) $(INCS) $(SRCS)

install: $(PROGRAM)
	echo Installing $(PROGRAM) in $(DEST)
	install -s $(PROGRAM) $(DEST)


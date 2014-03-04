LIBRARY = libGiST.a

SRCS = GiSTnode.cpp GiST.cpp GiSTfile.cpp GiSTpredicate.cpp GiSTcursor.cpp GiSTdb.cpp

OBJS = $(SRCS:.cpp=.o)

DEST = /usr/local/lib

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCS) $*.cpp

$(LIBRARY): .depend $(OBJS)
	rm -f $(LIBRARY)
	ar rs $(LIBRARY) $(OBJS)

clean:
	rm -f $(OBJS) $(LIBRARY) *~ #*# *pure*

depend:
	makedepend $(DEFINES) $(INCS) $(SRCS)

.depend:
	$(MAKE) depend
	touch .depend

install: $(PROGRAM)
	echo Installing $(PROGRAM) in $(DEST)
	install -s $(PROGRAM) $(DEST)


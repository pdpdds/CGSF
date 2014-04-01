# platforms:
#  linux-x86-32
#  linux-x86-64
#  win32-cygwin
#  solaris9-sparc-64
#  macosx
#  solaris8
PLATFORM =	linux-x86-32

# 'Makefile.version' defines the $(VERSION) of the library, and also
# setup the OBJS variable - stuff to be compiled.
include		Makefile.version


# Overall name of library, used to define library name(s) and
# target directory of installed include files.

# Default
NAME =		Sockets

# Debian GNU/Linux
#NAME =		sockets

LIBNAME =	lib$(NAME).a
SONAME =	lib$(NAME).so.$(MAJOR)
SHAREDLIBNAME =	lib$(NAME).so.$(VERSION)


CONFNAME =	$(NAME)-config

# .h files installed to $(DESTDIR)/$(PREFIX)/include/$(NAME)
# static lib .a files installed to $(DESTDIR)/$(PREFIX)/lib
PREFIX =	/usr/local

# debian
#PREFIX =	/usr

# include paths
INCLUDE =	-I/usr/include/libxml2

# Add for libxml2 if not in above location
#CFLAGS +=	`xml2-config --cflags`

# CXX, CFLAGS, LIBS, LDFLAGS, LDFLAGSSO
include		Makefile.Defines.$(PLATFORM)

# uncomment these lines if the library should be in its own namespace
#CFLAGS +=	-DSOCKETS_NAMESPACE=sockets
#CFLAGS +=	-DSOCKETS_NAMESPACE_STR='"sockets"'

# Enable insane amounts of debug output to stdout/stderr:
#CFLAGS +=	-D_DEBUG

CPPFLAGS =	$(CFLAGS) 

PROGS =		$(CONFNAME)

all:		$(LIBNAME) $(PROGS) pc shared

shared:		$(SHAREDLIBNAME)

$(LIBNAME):	$(OBJS)
		@echo Creating $@
		@ar cru $@ $^
		@ranlib $@

$(SHAREDLIBNAME): $(OBJS)
		@echo Creating $@
		@$(CXX) $(LDFLAGSSO) -o $@ $^ 

$(CONFNAME):	Sockets-config.o
		@echo Creating $@
		@$(CXX) $(LDFLAGS) -o $@ $^

pc:
		@cat pkgconfig/libsockets2.pc.IN | \
			sed -e "s/%VERSION%/$(VERSION)/g"| \
			sed -e "s/%NAME%/$(NAME)/g" > pkgconfig/libsockets2.pc 
		@cat pkgconfig/libSockets.pc.IN | \
			sed -e "s/%VERSION%/$(VERSION)/g"| \
			sed -e "s/%NAME%/$(NAME)/g" > pkgconfig/libSockets.pc 

clean:
		rm -f *.o *~ slask *.d $(PROGS) *.a *.so *.so.* */*~

-include	*.d


# everything which follows is www.alhem.net/Sockets website maintenance stuff, please ignore
HTDOCS =	/usr/local/apache/www.alhem.net/htdocs

diff:
		diff -b -B -C 3 /usr/src/Sockets-$(DIFF_VERSION) . | \
			/usr/local/bin/d2html > \
			$(HTDOCS)/Sockets/latest_diff.html

install:	all 
		@mkdir -p $(DESTDIR)/$(PREFIX)/lib
		cp $(LIBNAME) $(DESTDIR)/$(PREFIX)/lib
		@mkdir -p $(DESTDIR)/$(PREFIX)/include/$(NAME)
		cp -a *.h $(DESTDIR)/$(PREFIX)/include/$(NAME)
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/SSLSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/HttpsGetSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/HttpsSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/EventSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/PoolSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/SocketThread.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/CircularBuffer.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/*Crypt.h
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/CTcpSocket.h
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/Min*Socket.h
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/Min*Handler.h
		@rm -f $(DESTDIR)/$(PREFIX)/include/$(NAME)/Uid.h
		@mkdir -p $(DESTDIR)/$(PREFIX)/bin
		install $(CONFNAME) $(DESTDIR)/$(PREFIX)/bin

install_shared:	install shared
		@mkdir -p $(DESTDIR)/$(PREFIX)/lib/pkgconfig
		install -m 0644 $(SHAREDLIBNAME) $(DESTDIR)/$(PREFIX)/lib
		install -m 0644  pkgconfig/*pc $(DESTDIR)/$(PREFIX)/lib/pkgconfig
		rm -f $(DESTDIR)/$(PREFIX)/lib/lib$(NAME).so
		rm -f $(DESTDIR)/$(PREFIX)/lib/lib$(NAME).so.$(MAJOR)
		ln -s lib$(NAME).so.$(MAJOR).$(MINOR) $(DESTDIR)/$(PREFIX)/lib/lib$(NAME).so
		ln -s lib$(NAME).so.$(MAJOR).$(MINOR) $(DESTDIR)/$(PREFIX)/lib/lib$(NAME).so.$(MAJOR)

# no binary files, zip will translate lf to cr lf
FILES =		*.h *.cpp \
		Makefile Makefile.version \
		Makefile.Defines.* Makefile.solaris9-sparc-64 Project/*.ds* \
		README Changelog README.macosx gpl.txt mkdot.sh \
		Project.net/Sockets/*.vcproj Project.net/Sockets/*.sln \
		Project.net/Test/*.vcproj \
		DevCpp/*.dev tests/Makefile tests/*.cpp \
		OSX.zip pkgconfig/*.IN

tar:		clean
		rm -f MinderSocket_T.h
		rm -f uuid.h
		rm -f Stdin.*
		rm -f sockets_test.cpp
		rm -f ListenSocketBase.*
		rm -f CircularBuffer.*
		rm -f ICrypt.* NullCrypt.* CTcpSocket.*
		rm -f Min*Socket.* Min*Handler.*
		rm -f Uid.*
		rm -f Session.*
		tar czf Sockets-$(VERSION).tar.gz $(FILES)
		/usr/local/bin/tarfix.sh Sockets-$(VERSION)
		cp Sockets-$(VERSION).tar.gz $(HTDOCS)/Sockets
		cp Sockets-$(VERSION).zip $(HTDOCS)/Sockets
		cp tests/sockets_test.cpp /usr/local/apache/www.alhem.net/htdocs/Sockets/

tmptar:		clean
		rm -f MinderSocket_T.h
		rm -f uuid.h
		rm -f Stdin.*
		rm -f sockets_test.cpp
		rm -f ListenSocketBase.*
		rm -f CircularBuffer.*
		rm -f ICrypt.* NullCrypt.* CTcpSocket.*
		rm -f Min*Socket.* Min*Handler.*
		rm -f Uid.*
		rm -f Session.*
		tar czf Sockets-$(VERSION).tar.gz $(FILES)
		/usr/local/bin/tarfix.sh Sockets-$(VERSION)
		cp Sockets-$(VERSION).tar.gz $(HTDOCS)/Sockets-tmp
		cp Sockets-$(VERSION).zip $(HTDOCS)/Sockets-tmp
		cp tests/sockets_test.cpp /usr/local/apache/www.alhem.net/htdocs/Sockets-tmp

docs:		clean
		doxygen dox2.cfg
		rm -f MinderSocket_T.h
		rm -f uuid.h
		rm -f Stdin.*
		rm -f sockets_test.cpp
		rm -f ListenSocketBase.*
		rm -f CircularBuffer.*
		rm -f ICrypt.* NullCrypt.* CTcpSocket.*
		rm -f Min*Socket.* Min*Handler.*
		rm -f Uid.*
		rm -f tests/*.cpp
		rm -f Session.*
		./mkdot.sh
		rm -rf /usr/local/apache/www.alhem.net/htdocs/Sockets/html
		doxygen doxygen.cfg
		./packdocs.sh Sockets-$(VERSION)-doxygendocs
		svn up

tmpdocs:	clean
		rm -f MinderSocket_T.h
		rm -f uuid.h
		rm -f Stdin.*
		rm -f sockets_test.cpp
		rm -f ListenSocketBase.*
		rm -f CircularBuffer.*
		rm -f ICrypt.* NullCrypt.* CTcpSocket.*
		rm -f Min*Socket.* Min*Handler.*
		rm -f Uid.*
		rm -f tests/*.cpp
		rm -f Session.*
		./mkdot-tmp.sh
		rm -rf /usr/local/apache/www.alhem.net/htdocs/Sockets-tmp/html
		doxygen doxygen-tmp.cfg
		svn up

deb:
		@$(MAKE) -C debian
		@$(MAKE) -C debian-lib-dev

publish:
		@$(MAKE) -C debian publish
		@$(MAKE) -C debian-lib-dev publish

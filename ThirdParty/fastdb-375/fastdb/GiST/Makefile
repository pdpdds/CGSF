#	  Makefile for libGiST, GiST test programs
#
# Generalized Search Tree

# LINKER = purify -collector=/usr/sww/lib/gcc-lib/hppa1.1-hp-hpux9.03/2.7.2/ld -cache-dir=/tmp/jmh
LINKER = g++
# LINKER = c++
export LINKER

INCS = -I/usr/sww/lib/g++-include -I/usr/include/g++-3 -I../libGiST -I../../inc
# INCS = -I../libGiST
export INCS

LDFLAGS = -L../libGiST
export LDFLAGS

LIBS = -lGiST ../../libfastdb_r.a -lpthread
# LIBS = -lGiST
export LIBS

CC = g++
# CC = CC
export CC

DEFINES = -DUNIX
export DEFINES

CFLAGS = -g -Wall -Wno-unused $(DEFINES)
# CFLAGS = -g $(DEFINES)
export CFLAGS

# we recommend bison and flex for this code.
# if you want to use yacc and lex, you'll need to muck with the
# Makefiles a bit.
LEX = flex
export LEX

LEXFLAGS = -It
export LEXFLAGS

YACC = bison
export YACC

YACCFLAGS = -dv
export YACCFLAGS

all: LIBGIST RTREE BTREE RSTREE

LIBGIST: 
	cd libGiST ; $(MAKE) 

RTREE:  libGiST/libGiST.a
	cd RTree; $(MAKE)

BTREE:  libGiST/libGiST.a
	cd BTree; $(MAKE)

RSTREE: libGiST/libGiST.a
	cd RSTree; $(MAKE)

archive:
	$(MAKE) clean
	cd .. ; tar cf GiST.tar GiST ; gzip -f GiST.tar ; rm GiST.zip ; zip -pr GiST GiST/* ; cd GiST

clean:
	cd libGiST ; $(MAKE) clean 
	cd BTree; $(MAKE) clean 
	cd RSTree; $(MAKE) clean
	cd RTree; $(MAKE) clean

depend:
	cd libGiST ; $(MAKE) depend 
	cd BTree; $(MAKE) depend 
	cd RSTree; $(MAKE) depend
	cd RTree; $(MAKE) depend
tags:
	etags */*.{cpp,y,l,h}


INCLUDES = -I${top_srcdir}/inc

pkginclude_HEADERS = \
   ${top_srcdir}/inc/fastdb.h \
   ${top_srcdir}/inc/stdtp.h \
   ${top_srcdir}/inc/config.h \
   ${top_srcdir}/inc/class.h \
   ${top_srcdir}/inc/database.h \
   ${top_srcdir}/inc/cursor.h \
   ${top_srcdir}/inc/reference.h \
   ${top_srcdir}/inc/wwwapi.h \
   ${top_srcdir}/inc/cli.h \
   ${top_srcdir}/inc/array.h \
   ${top_srcdir}/inc/file.h \
   ${top_srcdir}/inc/hashtab.h \
   ${top_srcdir}/inc/ttree.h \
   ${top_srcdir}/inc/rtree.h \
   ${top_srcdir}/inc/container.h \
   ${top_srcdir}/inc/sync.h \
   ${top_srcdir}/inc/sync_unix.h \
   ${top_srcdir}/inc/query.h \
   ${top_srcdir}/inc/datetime.h \
   ${top_srcdir}/inc/harray.h \
   ${top_srcdir}/inc/rectangle.h \
   ${top_srcdir}/inc/timeseries.h \
   ${top_srcdir}/inc/exception.h

lib_LTLIBRARIES = libfastdb.la libcli.la
bin_PROGRAMS = subsql forcerecovery cleanupsem
noinst_PROGRAMS = cgistub

LIBSRC = \
   class.cpp \
   compiler.cpp \
   database.cpp \
   xml.cpp \
   hashtab.cpp \
   file.cpp \
   symtab.cpp \
   ttree.cpp \
   rtree.cpp \
   container.cpp \
   cursor.cpp \
   query.cpp \
   wwwapi.cpp \
   unisock.cpp \
   sync.cpp \
   localcli.cpp \
   stdtp.cpp

if NO_PTHREADS
else
LIBSRC += server.cpp
endif


libfastdb_la_SOURCES = $(LIBSRC)
libfastdb_la_LDFLAGS = -version-info 2:0:0

libcli_la_SOURCES = cli.cpp unisock.cpp repsock.cpp stdtp.cpp
libcli_la_LDFLAGS = -version-info 2:0:0

subsql_SOURCES = subsql.cpp
subsql_LDADD = ${builddir}/libfastdb.la

forcerecovery_SOURCES = forcerecovery.cpp
cleanupsem_SOURCES = cleanupsem.cpp

cgistub_SOURCES = cgistub.cpp
cgistub_LDADD = ${builddir}/libfastdb.la


# work around for libtool issue when compiling the same sources both
# with libtool and without
#libfastdb_la_CFLAGS = $(AM_CFLAGS)
#libfastdb_la_CXXFLAGS = $(AM_CXXFLAGS)

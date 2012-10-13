# -*- Makefile -*-
# $Id: platform_win32_dmc.GNU 93405 2011-02-17 19:30:42Z olli $

# debug = 1
# optimize = 1

winsock2 ?= 1
debug ?= 1
optimize ?= 1
threads ?= 1

pipes = 0
ifeq ($(debug),0)
CFLAGS += -DNDEBUG -v1
CFLAGS += -Ar

CC      = dmc
CXX     = dmc
RC      =

CFLAGS += $(MODEL_FLAGS) -ND

EXEEXT = .exe

DCFLAGS += -g
DLD     = link
LD      = link

PIC        =
AR         = ar
ARFLAGS    = rsuv
RANLIB     = ranlib
SOEXT      = dll
SOFLAGS    += $(MODEL_FLAGS) $(CPPFLAGS)
SOBUILD    = $(COMPILE.cc) $(PIC) -o $(VSHDIR)$*.so $<

PRELIB  = @true

CCFLAGS += $(CFLAGS)

# Dmc doesn't have rwho
rwho = 0

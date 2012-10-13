# $Id: platform_vxworks6.8.GNU 96017 2012-08-08 22:18:09Z mitza $
# VxWorks 6.8

ifeq ($(VXWORKS_VERSION_FLAG),)
  VXWORKS_VERSION_FLAG = -DACE_VXWORKS=0x680
endif # VXWORKS_VERSION_FLAG

WIND_USR_LIB ?= ../lib/usr/lib

include $(ACE_ROOT)/include/makeinclude/platform_vxworks6.7.GNU


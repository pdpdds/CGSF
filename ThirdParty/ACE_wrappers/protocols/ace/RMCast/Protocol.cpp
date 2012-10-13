// author    : Boris Kolpackov <boris@kolpackov.net>
// $Id: Protocol.cpp 91626 2010-09-07 10:59:20Z johnnyw $

#include "Protocol.h"

namespace ACE_RMCast
{
  u16 const From::  id = 0x0001;
  u16 const To::    id = 0x0002;
  u16 const Data::  id = 0x0003;
  u16 const SN::    id = 0x0004;
  u16 const NAK::   id = 0x0005;
  u16 const NRTM::  id = 0x0006;
  u16 const NoData::id = 0x0007;
  u16 const Part::  id = 0x0008;
}

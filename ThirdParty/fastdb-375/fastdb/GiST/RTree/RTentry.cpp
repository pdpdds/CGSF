// -*- Mode: C++ -*-

//         RTentry.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RTree/RTentry.cpp,v 1.1 2007/07/18 20:45:47 knizhnik Exp $
//
//  R-Tree Entry Class

#include "RT.h"

GiSTpenalty * 
RTentry::Penalty(const GiSTentry &newEntry) const
{
  GiSTpenalty *retval = new GiSTpenalty;
  assert(newEntry.IsA() == RTENTRY_CLASS);
  
  const RTentry& e = (const RTentry &) newEntry;
  RTkey *tmpkey = Key().expand(e.Key());

  // return area enlargement
  *retval = (MAX(tmpkey->area() - e.Key().area(), 0));
  delete tmpkey;
  return((GiSTpenalty *)retval);
}



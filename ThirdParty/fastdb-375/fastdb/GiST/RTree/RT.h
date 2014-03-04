// -*- Mode: C++ -*-

//          RT.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RTree/RT.h,v 1.1 2007/07/18 20:45:47 knizhnik Exp $

#define RT_H

#include "GiST.h"
#include "GiSTdb.h"
#include "RTentry.h"
#include "RTnode.h"
#include "RTpredicate.h"

class RT : public GiST
{
public:
  // optional, for debugging support
  GiSTobjid IsA() { return RT_CLASS; }
  RT(dbDatabase& aDb) : db(aDb) {} 

protected:
  // Required members
  GiSTnode  *CreateNode()  const { return new RTnode; }
  GiSTstore *CreateStore() const { return new GiSTdb(db); }

  dbDatabase& db;
};








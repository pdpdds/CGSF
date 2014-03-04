// -*- Mode: C++ -*-

//          BT.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/BTree/BT.h,v 1.1 2007/07/18 20:45:45 knizhnik Exp $

#ifndef BT_H
#define BT_H

#include "GiST.h"
#include "GiSTdb.h"
#include "BTentry.h"
#include "BTnode.h"
#include "BTpredicate.h"

class BT : public GiST
{
public:
  // optional, for debugging support
  GiSTobjid IsA() { return BT_CLASS; }
  BT(dbDatabase& aDb) : db(aDb) {} 

protected:
  // Required members
  GiSTnode  *CreateNode()  const { return new BTnode; }
  GiSTstore *CreateStore() const { return new GiSTdb(db); }

  // set special property
  int  IsOrdered()    const { return 1; }

  dbDatabase& db;
};

#endif











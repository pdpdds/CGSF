// -*- Mode: C++ -*-

//          BTpredicate.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/BTree/BTpredicate.cpp,v 1.1 2007/07/18 20:45:45 knizhnik Exp $

#include "BT.h"

int 
BTpredicate::Consistent(const GiSTentry& entry) const
{
  BTentry &btentry = (BTentry &)entry;
  const BTkey& l = btentry.LowerBound();
  const BTkey& u = btentry.UpperBound();

  switch (oper) {
  case BTEqual:
    return (value >= l) && (value <= u);
    break;
  case BTLessThan:
    return l < value;
    break;
  case BTGreaterThan:
    return value < u;  // Assume that the keys are integers.
    break;
  case BTLessEqual:
    return value >= l;
    break;
  case BTGreaterEqual:
    return value <= u;
    break;
  case BTNotEqual:
    // Assume that the keys are integers.
    return !((value == l) && (value == u));
    // return !((value == l) && (u - l == 1));
    break;
  default:
    assert(0);
	return (0);
    break;
  }
}


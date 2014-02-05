// -*- Mode: C++ -*-
//         BTpredicate.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/BTree/BTpredicate.h,v 1.1 2007/07/18 20:45:46 knizhnik Exp $

#ifndef BTPREDICATE_H
#define BTPREDICATE_H

#include "GiSTpredicate.h"

enum BToper {
  BTLessThan,
  BTLessEqual,
  BTEqual,
  BTNotEqual,
  BTGreaterEqual,
  BTGreaterThan
};

class BTpredicate : public GiSTpredicate {
public:
  BTpredicate(BToper oper, const BTkey& value) : oper(oper), value(value) {}
  BTpredicate(const BTpredicate& p) : oper(p.oper), value(p.value) {}
  GiSTobject *Copy() const { return new BTpredicate(*this); }
  GiSTobjid IsA() { return BTPREDICATE_CLASS; }
  int Consistent(const GiSTentry& entry) const;
  void SetOper(BToper op) { oper = op; }
  void SetValue(const BTkey& v) { value = v; }
#ifdef PRINTING_OBJECTS
  void Print(ostream& os) const {
    const char *operstrs[] = { "<", "<=", "=", "<>", ">=", ">" };
    os << "key " << operstrs[oper] << " " << value;
  }
#endif
private:
  BToper oper;
  BTkey value;
};

#endif

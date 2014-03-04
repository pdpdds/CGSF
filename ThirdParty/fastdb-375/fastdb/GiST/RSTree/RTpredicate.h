// -*- Mode: C++ -*-
//         RTpredicate.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RSTree/RTpredicate.h,v 1.1 2007/07/18 20:45:47 knizhnik Exp $

#ifndef RTPREDICATE_H
#define RTPREDICATE_H

#include "GiSTpredicate.h"

enum RToper {
  RToverlap,
  RTcontains,
  RTcontained,
  RTEqual
};

class RTpredicate : public GiSTpredicate {
public:
  RTpredicate(RToper oper, const RTkey& value) : oper(oper), value(value) {}
  RTpredicate(const RTpredicate& p) : oper(p.oper), value(p.value) {}
  GiSTobject *Copy() const { return new RTpredicate(*this); }
  GiSTobjid IsA() { return RTPREDICATE_CLASS; }
  int Consistent(const GiSTentry& entry) const;
  void SetOper(RToper op) { oper = op; }
  void SetValue(const RTkey& v) { value = v; }
#ifdef PRINTING_OBJECTS
  void Print(ostream& os) const {
    const char *operstrs[] = { "&&", "^", "||", "=" };
    os << "key " << operstrs[oper] << " " << value;
  }
#endif
private:
  RToper oper;
  RTkey value;
};

#endif

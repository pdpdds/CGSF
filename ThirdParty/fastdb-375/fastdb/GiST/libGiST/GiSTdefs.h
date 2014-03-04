// -*- Mode: C++ -*-

//          GiSTdefs.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiSTdefs.h,v 1.1 2007/07/18 20:45:49 knizhnik Exp $

#ifndef GISTDEFS_H
#define GISTDEFS_H

#include <assert.h>
#include <stdlib.h>
#include <math.h>

#ifdef PRINTING_OBJECTS
#include <iostream.h>
#endif

// A GiSTpage is a disk page number (a "pointer" to a disk page)
typedef unsigned long GiSTpage;

// GiSTobjid's are used to identify objects
// You can add new ones as you define new objects, or simply
// make things be GISTOBJECT_CLASS if you don't want to bother.
typedef enum {
  GISTOBJECT_CLASS,
  GIST_CLASS,
  BT_CLASS,
  RT_CLASS,
  GISTENTRY_CLASS,
  GISTNODE_CLASS,
  BTNODE_CLASS,
  BTENTRY_CLASS,
  BTKEY_CLASS,
  RTNODE_CLASS,
  RTENTRY_CLASS,
  RTKEY_CLASS,
  GISTPREDICATE_CLASS,
  BTPREDICATE_CLASS,
  RTPREDICATE_CLASS,
  GISTCURSOR_CLASS
} GiSTobjid;

// GiSTobject is the base class for all GiST classes
// It provides identity, equality tests and display of objects
  
class GiSTobject
{
public:
  virtual GiSTobjid IsA() const { return GISTOBJECT_CLASS; }
  virtual GiSTobject *Copy() const { return NULL; }
  virtual int IsEqual(const GiSTobject& obj) const { return 0; }
#ifdef PRINTING_OBJECTS
  virtual void Print(ostream& os) const { os << "No print method\n"; }
#endif
  virtual ~GiSTobject() {}
};

#ifdef PRINTING_OBJECTS
inline ostream& operator<< (ostream& os, const GiSTobject& obj) {
    obj.Print(os);
    return os;
}

inline ostream& operator<< (ostream& os, const GiSTobject *obj) {
    obj->Print(os);
    return os;
}
#endif

#endif

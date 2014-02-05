// -*- Mode: C++ -*-

//          RTnode.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RSTree/RTnode.h,v 1.1 2007/07/18 20:45:47 knizhnik Exp $

#ifndef RTNODE_H
#define RTNODE_H

#include "RTentry.h"

class RTnode : public GiSTnode {
public:

    GiSTobjid IsA() const { return RTNODE_CLASS; }
    GiSTentry *CreateEntry() const { return new RTentry; }
    int FixedLength() const { return sizeof(RTkey); }
    GiSTobject *Copy() const { return new RTnode(*this); }
    // A support routine for PickSplit
    RTentry *RTUnionEntries(const int entryvec[], const int min, 
			    const int max);

  // two of the basic GiST methods
    GiSTnode *PickSplit();
    GiSTentry* Union() const;
};

#endif







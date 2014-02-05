// -*- Mode: C++ -*-

//          BTnode.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/BTree/BTnode.h,v 1.1 2007/07/18 20:45:45 knizhnik Exp $


#ifndef BTNODE_H
#define BTNODE_H

#include "BTentry.h"

class BTnode : public GiSTnode {
public:
    // constructors, destructors, etc.
    GiSTobjid IsA() const { return BTNODE_CLASS; }
    GiSTobject *Copy() const { return new BTnode(*this); }

    // two of the basic GiST methods 
    GiSTentry* Union() const;

    // required support methods
    GiSTentry *CreateEntry() const { return new BTentry; }

    // members we are overriding for special cases
    void Insert(const GiSTentry &entry);
    void InsertBefore(const GiSTentry &entry, int index);
    void Coalesce(const GiSTnode& node, const GiSTentry& entry);

};

#endif







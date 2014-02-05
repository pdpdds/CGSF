// -*- Mode: C++ -*-

//          GiSTpredicate.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiSTpredicate.cpp,v 1.1 2007/07/18 20:45:50 knizhnik Exp $

#include "GiST.h"

int PtrPredicate::Consistent(const GiSTentry& entry) const
{
    return !entry.IsLeaf() || entry.Ptr() == page;
}

GiSTobject* PtrPredicate::Copy() const
{
    return new PtrPredicate(page);
}

#ifdef PRINTING_OBJECTS
void PtrPredicate::Print(ostream& os) const
{
    os << "ptr = " << page;
}
#endif

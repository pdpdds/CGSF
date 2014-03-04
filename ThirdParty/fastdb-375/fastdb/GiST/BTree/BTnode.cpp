// -*- Mode: C++ -*-

//         BTnode.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/BTree/BTnode.cpp,v 1.1 2007/07/18 20:45:45 knizhnik Exp $

#include <string.h>

#include "BT.h"

extern BTkey PosInf, NegInf;

void 
BTnode::InsertBefore(const GiSTentry& entry, int index)
{
    // Only BTentry's can be inserted into BTnodes.
    assert(entry.IsA() == BTENTRY_CLASS);

    BTentry e((const BTentry&) entry);

    // If this is an internal node, adjust the lower/upper bounds
    if (!IsLeaf()) {
	// If not leftmost entry...
	if (index != 0) {
	    // -inf changes to the upper bound of previous item
	    BTentry *prev = (BTentry*) (*this)[index-1].Ptr();
	    if (e.LowerBound() == NegInf)
		e.SetLowerBound(prev->UpperBound());
	}
	// If not rightmost entry...
	if (index != NumEntries()) {
	    // +inf changes to the lower bound of next item
	    BTentry *next = (BTentry*) (*this)[index].Ptr();
	    if (e.UpperBound() == PosInf)
		e.SetUpperBound(next->LowerBound());
	}
    }

    // Insert it into the node
    GiSTnode::InsertBefore(e, index);
}

void 
BTnode::Insert(const GiSTentry &entry)
{
    // Only BTentry's can be inserted into BTnode's.
    assert(entry.IsA() == BTENTRY_CLASS);

    // This doesn't work for internal nodes.  For internal nodes,
    // the caller must know exactly where to insert.
    assert(IsLeaf());
   
    GiSTnode::Insert(entry);
}

// We are moving entries from node, which is to our right, to this.
// If this is a non-empty internal page, the rightmost entry on this 
// has an upperbound of +inf, and the leftmost entry on node has
// a lower bound of -inf.  This code takes care of that -- both
// these values should be set to the right bound of our parent entry.
void 
BTnode::Coalesce(const GiSTnode& node, 
		 const GiSTentry& entry) // entry is in 1 level up, and
                                         // points to this
{
    if (!NumEntries() || IsLeaf()) {
	GiSTnode::Coalesce(node, entry);
    } else {
	int n = NumEntries();
	GiSTnode::Coalesce(node, entry);
	const BTentry &bte = (const BTentry&) entry;
	((BTentry*)(*this)[n-1].Ptr())->SetUpperBound(bte.UpperBound());
	((BTentry*)(*this)[n].Ptr())->SetLowerBound(bte.UpperBound());
    }
}

GiSTentry* 
BTnode::Union() const
{
  BTentry *u = new BTentry;
  int first = 1;

  for (int i=0; i<NumEntries(); i++) {
      BTentry *bte = (BTentry*) (*this)[i].Ptr();
      if (first || bte->LowerBound() < u->LowerBound())
	  u->SetLowerBound(bte->LowerBound());
      if (first || bte->UpperBound() > u->UpperBound())
	  u->SetUpperBound(bte->UpperBound());
      first = 0;
  }

//u->SetPtr(node.Path().Page());
  return u;
}

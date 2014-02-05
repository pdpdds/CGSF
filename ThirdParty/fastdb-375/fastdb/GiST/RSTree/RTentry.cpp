// -*- Mode: C++ -*-

//         RTentry.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RSTree/RTentry.cpp,v 1.1 2007/07/18 20:45:46 knizhnik Exp $
//
//  R-Tree Entry Class

#include "RT.h"

// R*-tree penalty metric returns a triplet:
// The first entry is the "Overlap Enlargement", which is used only
// at the level above the leaves.  The second entry is the area enlargement,
// which is used everywhere else, and to break ties in the first entry.
// The third entry is just area, which is used to break ties in the second 
// entry.
GiSTpenalty * 
RTentry::Penalty(const GiSTentry &newEntry) const
{
  RTpenalty *retval = new RTpenalty;
  assert(newEntry.IsA() == RTENTRY_CLASS);
  
  const RTentry& e = (const RTentry &) newEntry;
  RTkey *tmpkey = Key().expand(e.Key());

  // If we're one level up from the leaves, return overlap enlargement
  if (Level() == 1) {
    double curoverlap = OverlapArea(Key());
    double newoverlap = OverlapArea(*tmpkey);
    *retval = MAX(newoverlap - curoverlap, 0);
  }
  else *retval = 0;
  retval->amount2 = (MAX(tmpkey->area() - e.Key().area(), 0));
  retval->amount3 = e.Key().area();
  
  delete tmpkey;
  return((GiSTpenalty *)retval);
}

double
RTentry::OverlapArea(const RTkey &k) const
{
  int i;
  GiSTnode *n = Node();
  RTkey *okey, *tmpkey;
  double retval;

  okey = new RTkey(k);

  for (i = 0; i < n->NumEntries(); i++)
    if (i != Position()) {
      tmpkey = okey;
      okey = tmpkey->intersect(((RTentry *)((*n)[i].Ptr()))->bbox());
      delete tmpkey;
      if (okey == NULL)
	return(0);
    }
  retval = okey->area();
  delete okey;
  return(retval);
}

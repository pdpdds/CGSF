// -*- Mode: C++ -*-

//         RTnode.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RSTree/RTnode.cpp,v 1.1 2007/07/18 20:45:47 knizhnik Exp $

#include <string.h>

#include "RT.h"

typedef struct {
  int ix;
  double l;
  double h;
} doubleix;
extern "C" int RTcmp(const void *x, const void *y);

GiSTnode *
RTnode::PickSplit()
{
  // The R*-tree split algorithm
  int i, splitindex;
  int *xvec, *yvec, *thevec;
  doubleix *dxvec, *dyvec;
  double tmpS, minS=0, minarea=0; // initialize to prevent compiler warnings
  int xismin = 1;                 // ditto for xismin
  int m = (int)((double)NumEntries()*0.4); // m=40% is recommended by Beckmann, et al.
  RTentry *tmpe1, *tmpe2;
  RTentry *tmpentry;
  RTkey *tmpkey;

  xvec = new int[NumEntries()];
  yvec = new int[NumEntries()];
  dxvec = new doubleix[NumEntries()];
  dyvec = new doubleix[NumEntries()];
  // ChooseSplitAxis:
  // first sort entries by (xl,xh)
  // then by (yl,yh)
  // for each distribution of m-1+k entries, compute S, the sum of the
  // margin-value (margin[bb(first-group)] + margin[bb(second-group)]).  
  // Axis with min S is the split axis.  
  for (i = 0; i < NumEntries(); i++) {
    dxvec[i].ix = i;
	tmpentry = (RTentry *)((*this)[i].Ptr());
    dxvec[i].l = tmpentry->xlower();
    dxvec[i].h = tmpentry->xupper();
    dyvec[i].ix = i;
    dyvec[i].l = tmpentry->ylower();
    dyvec[i].h = tmpentry->yupper();
  }
  qsort(dxvec, NumEntries(), sizeof(doubleix), RTcmp);
  qsort(dyvec, NumEntries(), sizeof(doubleix), RTcmp);

  // set up simpler xvec and yvec (could be improved later)
  for (i = 0; i < NumEntries(); i++) {
    xvec[i] = dxvec[i].ix;
    yvec[i] = dyvec[i].ix;
  }

  
  for(i = m; i < NumEntries() - m; i++) {
    // compute bounding boxes of 0-i, i+1-NumEntries()
    // take sum of areas
    tmpentry = RTUnionEntries(xvec, 0, i);
    tmpS = ((RTkey)(tmpentry->bbox())).area();
    delete tmpentry;
    tmpentry = RTUnionEntries(xvec, i, NumEntries());
    tmpS += ((RTkey)(tmpentry->bbox())).area();
    delete tmpentry;
	
    // is tmpS < minS?  if so, set minS and xismin
    if (i == m || tmpS < minS) {
      minS = tmpS;
      xismin = 1;
    }
    // do the same for the y ordering
    tmpentry = RTUnionEntries(yvec, 0, i);
    tmpS = ((RTkey)(tmpentry->bbox())).area();
    delete tmpentry;
    tmpentry = RTUnionEntries(yvec, i, NumEntries());
    tmpS += ((RTkey)(tmpentry->bbox())).area();
    delete tmpentry;
    // is tmpS < minS?  if so, set minS and xismin
    if (tmpS < minS) {
      minS = tmpS;
      xismin = 0;
    }
  }
    
  // ChooseSplitIndex:
  // Compute minimum overlap-value
  // for each distribution (area[bb(first) intersect bb(second)]).
  // Along the chosen split axis, choose the distribution with the 
  // minimum overlap-value.
  if (xismin)
    thevec = xvec;
  else
    thevec = yvec;
  for (i = m; i < NumEntries() - m; i++) {
    tmpe1 = RTUnionEntries(thevec, 0, i);
    tmpe2 = RTUnionEntries(thevec, i, NumEntries());
    tmpkey = ((RTkey)(tmpe1->bbox())).intersect(tmpe2->bbox());
    if (i == m || !tmpkey || tmpkey->area() < minarea) {
      if (!tmpkey) minarea = 0;
      else minarea = tmpkey->area();
      splitindex = i;
    }
    delete tmpe1;
    delete tmpe2;
    if (tmpkey) delete tmpkey;
  }

  // distribute according to ChooseSplitIndex
  RTnode *rightnode = (RTnode *)Copy();
  DeleteBulk(&thevec[i], NumEntries() - i);
  rightnode->DeleteBulk(thevec, i);

  // be tidy
  delete xvec;
  delete yvec;
  delete dxvec;
  delete dyvec;
  
  return rightnode;
}

GiSTentry * 
RTnode::Union() const
{
  RTentry *u = new RTentry;
  int first = 1;

  u->InitKey();
  for (int i=0; i<NumEntries(); i++) {
      RTentry *RTe = (RTentry*) (*this)[i].Ptr();
      if (first || RTe->xlower() < u->xlower())
	  u->setxlower(RTe->xlower());
      if (first || RTe->xupper() > u->xupper())
	  u->setxupper(RTe->xupper());
      if (first || RTe->ylower() < u->ylower())
	  u->setylower(RTe->ylower());
      if (first || RTe->yupper() > u->yupper())
	  u->setyupper(RTe->yupper());
      first = 0;
  }

  return u;
}

extern "C" int 
RTcmp(const void *x, const void *y)
{
  doubleix *i = (doubleix *)x;
  doubleix *j = (doubleix *)y;

  if (i->l == j->l)
    return((int)(i->h - j->h));
  else return((int)(i->l - j->l));
}

RTentry * 
RTnode::RTUnionEntries(const int entryvec[], const int min, const int max)
{
    RTentry *u = new RTentry;
    int first = 1;

    u->InitKey();
    for (int i=min; i<max; i++) {
      RTentry *RTe = (RTentry*) (*this)[entryvec[i]].Ptr();
      if (first || RTe->xlower() < u->xlower())
	  u->setxlower(RTe->xlower());
      if (first || RTe->xupper() > u->xupper())
	  u->setxupper(RTe->xupper());
      if (first || RTe->ylower() < u->ylower())
	  u->setylower(RTe->ylower());
      if (first || RTe->yupper() > u->yupper())
	  u->setyupper(RTe->yupper());
      first = 0;
    }

    return u;
}


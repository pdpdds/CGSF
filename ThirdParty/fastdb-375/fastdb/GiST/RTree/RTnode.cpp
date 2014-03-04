// -*- Mode: C++ -*-

//         RTnode.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RTree/RTnode.cpp,v 1.1 2007/07/18 20:45:47 knizhnik Exp $


#include <string.h>
#include "RT.h"

GiSTnode *
RTnode::PickSplit()
{
  int i;
  int j;
  double max = -1, tmp;
  int seed1 = -1, seed2 = -1;
  RTkey cover1, cover2, *expand1, *expand2;
  RTnode *rightnode;
  RTentry *tmpentry1, *tmpentry2;
  int rightdeletes = 0, leftdeletes = 0;
  int *rightvec, *leftvec;

  // Guttman's poly-time split algorithm
  // first, pick furthest-distance seeds
  for (i = 0; i < NumEntries(); i++)
    for (j = i+1; j < NumEntries(); j++) {
      tmpentry1 = (RTentry *)((*this)[i].Ptr());
      tmpentry2 = (RTentry *)((*this)[j].Ptr());
      tmp = ((RTkey)tmpentry1->bbox()).dist(tmpentry2->bbox());
      if (tmp > max) {
	seed1 = i;
	seed2 = j;
	max = tmp;
      }
    }
  tmpentry1 = (RTentry *)((*this)[seed1].Ptr());
  tmpentry2 = (RTentry *)((*this)[seed2].Ptr());
  cover1 = tmpentry1->bbox();
  cover2 = tmpentry2->bbox();

  // copy current node into rightnode; we'll delete from the nodes
  // as appropriate
  rightnode = (RTnode *)Copy();

  // now assign entries to the appropriate deletion vectors
  rightvec = new int[NumEntries()];
  leftvec = new int[NumEntries()];
  for (i = 0; i < NumEntries(); i++) {
    if (i == seed1) {
      rightvec[rightdeletes++] = i;
      continue;
    }
    if (i == seed2) {
      leftvec[leftdeletes++] = i;
      continue;
    }
    tmpentry1 = (RTentry *)((*this)[i].Ptr());
    expand1 = cover1.expand(tmpentry1->bbox());
    expand2 = cover2.expand(tmpentry1->bbox());
    if (expand2->area() > expand1->area()) 
      leftvec[leftdeletes++] = i;
    else
      rightvec[rightdeletes++] = i;
    delete expand1;
    delete expand2;
  }

  // given the deletion vectors, do bulk deletes
  DeleteBulk(leftvec, leftdeletes);
  rightnode->DeleteBulk(rightvec, rightdeletes);
  delete leftvec;
  delete rightvec;

  // Return the right node
  return rightnode;
}

GiSTentry * 
RTnode::Union() const
{
  RTentry *u = new RTentry;
  int first = 1;

  u->InitKey();
  for (int i=0; i<NumEntries(); i++) {
      RTentry *rte = (RTentry*) (*this)[i].Ptr();
      if (first || rte->xlower() < u->xlower())
	  u->setxlower(rte->xlower());
      if (first || rte->xupper() > u->xupper())
	  u->setxupper(rte->xupper());
      if (first || rte->ylower() < u->ylower())
	  u->setylower(rte->ylower());
      if (first || rte->yupper() > u->yupper())
	  u->setyupper(rte->yupper());
      first = 0;
  }

  return u;
}



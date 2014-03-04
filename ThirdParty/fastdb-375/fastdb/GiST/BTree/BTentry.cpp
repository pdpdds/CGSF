// -*- Mode: C++ -*-

//         BTentry.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/BTree/BTentry.cpp,v 1.1 2007/07/18 20:45:45 knizhnik Exp $
//
//  B-Tree Entry Class

#include "BT.h"

BTkey NegInf(BTkey::NEG_INF), PosInf(BTkey::POS_INF);

int BTkey::Compare(const BTkey& k) const {
  if (type == k.type && type != NORMAL)
    return 0;
  else if (type == NEG_INF || k.type == POS_INF)
    return (-1);
  else if (type == POS_INF || k.type == NEG_INF)
    return 1;
  else {
    for (int i = 0; i < len && i < k.len; i++) {
      if (key[i] != k.key[i])
	return(key[i] - k.key[i]);
      else if (len == i+1 || k.len == i+1)
	return (len - k.len);
      else continue;
    }
	return 0; // make NT compiler happy
  }
}

GiSTcompressedEntry 
BTentry::Compress() const {
  GiSTcompressedEntry compressedEntry;

  // Compress the key
  if (Position() == 0 && !IsLeaf())
    compressedEntry.keyLen = 0;
  else {
    compressedEntry.key = new char[LowerBound().len];
    memcpy(compressedEntry.key, LowerBound().key, LowerBound().len);
    compressedEntry.keyLen = LowerBound().len;
  }

  // Copy the pointer
  compressedEntry.ptr = ptr;

  return compressedEntry;
}

void 
BTentry::Decompress(const GiSTcompressedEntry entry)
{
  const GiSTnode *node = Node();
  assert(node->IsA() == BTNODE_CLASS);

  // Decompress the key
  if (!IsLeaf()) {
    if (Position() == 0) // Leftmost
      SetLowerBound(NegInf);
    else {
      SetLowerBound(entry.key, entry.keyLen);
    }

    if (Position() == node->NumEntries()-1)
      SetUpperBound(PosInf);
    else {
      GiSTcompressedEntry nextEntry = node->Entry(Position()+1);
      SetUpperBound(nextEntry.key, nextEntry.keyLen);
    }
  } else { // node->IsLeaf()
    SetLowerBound(entry.key, entry.keyLen);
    SetUpperBound(LowerBound());
  }

  // Copy the pointer
  ptr = entry.ptr;
}

GiSTpenalty *
BTentry::Penalty(const GiSTentry &newEntry) const
{
  GiSTpenalty* p = new GiSTpenalty;
  assert(newEntry.IsA() == BTENTRY_CLASS);

  const BTentry& e = (const BTentry &) newEntry;

  if (e.UpperBound() < LowerBound())
    *p = 1;

  else if (e.LowerBound() > UpperBound())
    *p = 1;

  else
	  *p = 0;
  
  return p;
}

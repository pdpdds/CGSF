// -*- Mode: C++ -*-

//          GiSTentry.h
//
// Copyright (c) 1996, Regents of the University of California


#ifndef GISTENTRY_H
#define GISTENTRY_H

#include <string.h> // for memcpy
#include "GiSTdefs.h"
#include "GiSTpredicate.h"

class GiSTnode;

// GiSTcompressedEntry is the compressed format of a key-pointer pair.
class GiSTcompressedEntry {
public:
  char *key;
  int  keyLen;
  GiSTpage ptr;

  GiSTcompressedEntry() : key(NULL), keyLen(0), ptr(0) {}
  const GiSTcompressedEntry operator = (const GiSTcompressedEntry c) {
    key = c.key;
    keyLen = c.keyLen;
    ptr = c.ptr;
    return(*this);
  }
};

// GiSTpenalty is the token returned by the Penalty method.  We don't
// just use doubles here because it can be useful to use more
// complex Penalty comparisons (e.g. to support multivalued Penalties that
// break ties in a single value).
class GiSTpenalty {
public:
  GiSTpenalty() : amount(0) {}
  GiSTpenalty(const double d): amount(d) {}
  virtual double operator = (const double d) { 
    amount = d; 
    return amount;
  }
  virtual int operator < (const GiSTpenalty &p) const {
    if (amount < p.amount)
      return(1);
    else 
      return(0);
  }

  virtual ~GiSTpenalty() {}

protected:
  double amount;
};

// GiSTentry: a key/pointer pair inside a node.
// The key is a pointer to a GiSTobject, which can be used to
// point to just about anything you like.
class GiSTentry : public GiSTobject {
public:
  GiSTentry() : node(NULL), level(0) {}
  GiSTentry(GiSTpage ptr) : node(NULL), ptr(ptr) {}
  GiSTentry(const GiSTentry& e) : node(e.node), ptr(e.ptr), 
  position(e.position), level(e.level) {}

  virtual ~GiSTentry() {
    if (key)
      delete key;
  }

  // The following methods must be overridden by descendant classes
  virtual GiSTpenalty *Penalty(const GiSTentry &key) const = 0;
  virtual int CompressedLength() const = 0; // length of compressed key
#ifdef PRINTING_OBJECTS
  virtual void Print(ostream& os) const = 0;
#endif
  virtual GiSTobject *Copy() const = 0;
  // Default compression is just to copy the key (i.e. no compression).
  // Since the copy method is virtual, the appropriate Copy method is called.
  // Besides compressing the key, Compress also sets the pointer.
  virtual GiSTcompressedEntry Compress() const {
      GiSTcompressedEntry compressedEntry;
      compressedEntry.key = (char *)key->Copy();
      compressedEntry.keyLen = CompressedLength();
      compressedEntry.ptr = ptr;
      return compressedEntry;
  }
  // Besides decompressing the key, Decompress also sets the pointer.
  virtual void Decompress(const GiSTcompressedEntry entry) {
      key = (GiSTobject *)new char[entry.keyLen];
      memcpy(key, entry.key, entry.keyLen);
      ptr = entry.ptr;
  }

  // If you have ordered keys, this can be overridden a la qsort's compare
  virtual int Compare(const GiSTentry& entry) const { return 0; }

  // access to the protected info
  GiSTpage Ptr() const { return ptr; }
  void SetPtr(GiSTpage p) { ptr = p; }
  GiSTobject *Key() const { return key; }

  GiSTobjid IsA() const { return GISTENTRY_CLASS; }
  int IsLeaf() const { return level == 0; }
  int Level() const { return level; }
  void SetLevel(int l) { level = l; }

  void SetPosition(int pos) { position = pos; }
  int Position() const { return position; }

  GiSTnode *Node() const { return node; }
  void SetNode(GiSTnode *n) { node = n; }


protected:
  GiSTnode *node;
  GiSTpage ptr;
  int position;
  int level;
  GiSTobject *key;
};

#endif

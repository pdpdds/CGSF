// -*- Mode: C++ -*-

//           BTentry.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/BTree/BTentry.h,v 1.1 2007/07/18 20:45:45 knizhnik Exp $

#ifndef BTENTRY_H
#define BTENTRY_H

#include <string.h>
#include <iostream.h>

#include "GiSTdefs.h"
#include "GiSTentry.h"

class BTpredicate;
class BTnode;
class BTentry;

// BTkey is a simple container for a string
// it is the form for query keys, but NOT for keys in entries.
// Keys in entries are BTintkeys (below).
class BTkey : GiSTobject
{
public:
    enum BTkeyType { NORMAL, NEG_INF, POS_INF } type;
    char *key;
    int len;
    BTkey(const BTkeyType type) : type(type), key(NULL), len(0) {}
    BTkey() {
	type = NORMAL;
	key = NULL;
	len = 0;
    }

    void Set(const char *s, const int n) {
	type = NORMAL;
	key = new char[n];
	len = n;
	memcpy(key, s, n);
    }
    BTkey(const char *s, const int n) {
	Set(s, n);
    }
    BTkey& operator = (const BTkey& k) {
	if (key) delete key;
	if (k.len)
	    key = new char[k.len];
	else
	    key = NULL;
	len = k.len;
	type = k.type;
	memcpy(key, k.key, k.len);
	return *this;
    }
    BTkey(const BTkey& k) {
	key = NULL;
	*this = k;
    }
    ~BTkey() {
	if (key) delete key;
    }
    int Compare(const BTkey& k) const;
    int operator <  (const BTkey& k) const { return Compare(k) <  0; }
    int operator <= (const BTkey& k) const { return Compare(k) <= 0; }
    int operator == (const BTkey& k) const { return Compare(k) == 0; }
    int operator != (const BTkey& k) const { return Compare(k) != 0; }
    int operator >= (const BTkey& k) const { return Compare(k) >= 0; }
    int operator >  (const BTkey& k) const { return Compare(k) >  0; }

    void Print(ostream& os) const {
	switch (type) {
	  case NEG_INF:
	    os << "-inf";
	    break;
	  case POS_INF:
	    os << "+inf";
	    break;
	  default:
	    os << '"';
	    for (int i=0; i<len; i++)
		os << key[i];
	    os << '"';
	    break;
	}
    }
};

inline ostream& operator<< (ostream& os, const BTkey& k) {
    k.Print(os);
    return os;
}

// BTintkey: a pair of strings.  The key type for BTentry.
class BTintkey : public GiSTobject
{
public:
    // constructors, destructors, assignment, etc.
    BTintkey() {}
    BTintkey(const BTintkey& k) {
        lowerBound = k.lowerBound;
	upperBound = k.upperBound;
    }
    BTintkey(const BTkey& lo, const BTkey& hi):
      lowerBound(lo), upperBound(hi) {}
    GiSTobject *Copy() const { return new BTintkey(*this); }
    ~BTintkey() { }
    GiSTobjid IsA() const { return BTKEY_CLASS; }
  
    // range comparison methods
    int operator == (const BTintkey& k) const { 
      return (lowerBound.Compare(k.lowerBound) == 0
	      && upperBound.Compare(k.upperBound) == 0); 
    }
    // shouldn't really get called, but just in case...
    int Compare(const BTintkey& k ) const {
        assert(k.IsA() == BTKEY_CLASS);
	const BTintkey& k2 = (const BTintkey&) k;
	return lowerBound.Compare(k2.lowerBound);
    }
private:
    friend BTentry;
    BTkey lowerBound, upperBound;
};

class GiSTpredicate;

class BTentry : public GiSTentry
{
friend BTpredicate;
  private:
    // cast key member to class BTkey *.  Prevents compiler warnings.
    BTintkey& Key() const { return *(BTintkey *)key; }
  public:
    // constructors, destructors, etc.
    BTentry() {
      key = (GiSTobject *)new BTintkey;
    }
    BTentry(BTintkey k, GiSTpage p) {
      key = (GiSTobject *)new BTintkey(k);
      ptr = p;
    }
    BTentry(BTkey val, GiSTpage p) {
      key = (GiSTobject *)new BTintkey(val,val);
      ptr = p;
    }
    BTentry(const BTentry& e) : GiSTentry(e) {
      key = (GiSTobject *)new BTintkey(*(BTintkey *)(e.key));
    }
    ~BTentry() { }
    GiSTobjid IsA() const { return BTENTRY_CLASS; }
    GiSTobject *Copy() const { return new BTentry(*this); }
    int IsEqual(const GiSTobject& obj) const {
	if (obj.IsA() != BTENTRY_CLASS)
	    return 0;
	const BTentry& bte = (const BTentry&) obj;
	return key == bte.key &&
		ptr == bte.ptr;
    }

    // basic GiST methods
    GiSTpenalty *Penalty(const GiSTentry &newEntry) const;
    // Besides compressing the key, Compress also sets the pointer.
    GiSTcompressedEntry Compress() const;
    // Besides decompressing the key, Decompress also sets the pointer.
    void Decompress(const GiSTcompressedEntry entry);

    // Other methods we're required to supply
    int CompressedLength() const {
	if (!IsLeaf() && Position() == 0)
	    return 0;
	return Key().lowerBound.len;
    }

    // I/O methods
#ifdef PRINTING_OBJECTS
    void Print(ostream& os) const {
	if (IsLeaf())
	    os << Key().lowerBound << "->" << Ptr() << endl;
	else
	    os << "[" << Key().lowerBound << "," << Key().upperBound << "]->" << Ptr() << endl;
    }
#endif

    // access to private members
    const BTkey& LowerBound() const { return Key().lowerBound; }
    const BTkey& UpperBound() const { return Key().upperBound; }
    void SetLowerBound(const BTkey& k) { Key().lowerBound = k; }
    void SetLowerBound(const char *c, const int l) { Key().lowerBound.Set(c,l); }
    void SetUpperBound(const BTkey& k) { Key().upperBound = k; }
    void SetUpperBound(const char *c, const int l) { Key().upperBound.Set(c,l); }

    // this shouldn't really get called, but just in case...
    int Compare(const GiSTentry& entry) const {
	assert(entry.IsA() == BTENTRY_CLASS);
	const BTentry& e = (const BTentry&) entry;
	return Key().Compare(*(BTintkey *)(e.key));
    }

    // This seems to be dead code...
    // GiSTpredicate *Predicate() const;
};

#endif



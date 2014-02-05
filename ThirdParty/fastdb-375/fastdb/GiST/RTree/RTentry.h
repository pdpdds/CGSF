// -*- Mode: C++ -*-

//           RTentry.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RTree/RTentry.h,v 1.1 2007/07/18 20:45:47 knizhnik Exp $

#ifndef RTENTRY_H
#define RTENTRY_H

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <iostream.h>

#include "GiSTdefs.h"
#include "GiSTentry.h"

#ifndef MIN
#define MIN(x, y) ((x < y) ? (x) : (y))
#define MAX(x, y) ((x > y) ? (x) : (y))
#endif

class RTpredicate;
class RTnode;
class RTentry;

// RTkey is a rectangle represented as 4 doubles:
//   xlow, xhigh, ylow and yhigh
// We also throw in some standard geo operators to support various 
// query predicates, etc.

class RTkey : GiSTobject
{
public:
  friend RTentry;
  // constructors, destructors, assignment, etc.
  RTkey() {
    xl = yl = xh = yh = 0;
  }
  RTkey(const RTkey& k) {
    xl = k.xl;
    xh = k.xh;
    yl = k.yl;
    yh = k.yh;
  }
  RTkey(const double xl, const double xh, const double yl, const double yh):
    xl(xl), xh(xh), yl(yl), yh(yh) {
    }
  RTkey(char *s);
  GiSTobject *Copy() const { return new RTkey(*this); }

  ~RTkey() {
  }
  GiSTobjid IsA() { return RTKEY_CLASS; }
  
  RTkey& operator = (const RTkey& k) {
    xl = k.xl;
    xh = k.xh;
    yl = k.yl;
    yh = k.yh;
    return *this;
  }

  // bounding-box property and comparison methods
  int operator == (const RTkey& k) {
    return (xl == k.xl && xh == k.xh && yl == k.yl && yh == k.yh);
  }

  int overlap (const RTkey& k) {
    return(((xh >= k.xh && xl <= k.xh) ||
            (k.xh >= xh && k.xl <= xh)) &&
           ((yh >= k.yh && yl <= k.yh) ||
            (k.yh >= yh && k.yl <= yh)) );
  }

  int contained (const RTkey& k) {
    return((xh <= k.xh && xl >= k.xl &&
            yh <= k.yh && yl >= k.yl));
  }

  int contain(const RTkey& k) {
    return((xh >= k.xh && xl <= k.xl &&
            yh >= k.yh && yl <= k.yl));
  }

  double area() { return((xh-xl)*(yh-yl)); }

  // distance between the centers of two bounding boxes
  double dist(const RTkey& b) {
    return(sqrt(pow(fabs(((b.xh + b.xl)/2)-((xh + xl)/2)), 2) 
		+ pow(fabs(((b.yh + b.yl) - 2)-((yh + yl)/2)),2)));
  }

  // methods to form new bounding boxes out of existing ones
  RTkey *intersect(const RTkey& k) {
    RTkey *result;
    
    if (!overlap(k))
      return((RTkey *)NULL);  
    result = new RTkey(MAX(xl,k.xl), MIN(xh,k.xh), MAX(yl,k.yl), 
                       MIN(yh,k.yh));
      
    return(result);
  }

  // expand this bounding box to contain k
  RTkey *expand(const RTkey& k) {
    RTkey *result;

    result = new RTkey(MIN(xl, k.xl), MAX(xh, k.xh), MIN(yl, k.yl),
                       MAX(yh, k.yh));
    return(result);
  }

  // I/O methods
  void Print(ostream& os) const {
    os << '"';
    os << "(" << xl << "," << xh << "," << yl << "," << yh << ")";
    os << '"';
  }

  double xl; // x coordinate of left edge
  double xh; // x coordinate of right edge
  double yl; // y coordinate of bottom edge
  double yh; // y coordinate of top edge
};

inline ostream& operator<< (ostream& os, const RTkey& k) {
    k.Print(os);
    return os;
}

class GiSTpredicate;

class RTentry : public GiSTentry
{
friend RTpredicate;
  private:
    // cast key member to class RTkey *.  Prevents compiler warnings.
    RTkey &Key() const { return *(RTkey *)key; }
  public:
  // constructors, destructors, etc.
    RTentry() { }
    RTentry(const RTkey& k, const GiSTpage p) {
      key = (GiSTobject *)new RTkey(k);
      ptr = p;
    }
    RTentry(const RTentry& e) : GiSTentry(e) {
      key = (GiSTobject *)new RTkey(*(RTkey *)(e.key));
    }
    GiSTobjid IsA() const { return RTENTRY_CLASS; }
    GiSTobject *Copy() const { return new RTentry(*this); }
    void InitKey() { key = new RTkey; }
    int IsEqual(const GiSTobject& obj) const {
        if (obj.IsA() != RTENTRY_CLASS)
            return 0;
        const RTentry& rte = (const RTentry&) obj;
        return ((*((RTkey *)key) == (*((RTkey *)(rte.key)))));
    }

    // basic GiST methods
    GiSTpenalty *Penalty(const GiSTentry &newEntry) const;

    // Other methods we're required to supply
    int CompressedLength() const {
      return sizeof(RTkey);
    }

    // I/O methods
#ifdef PRINTING_OBJECTS
    void Print(ostream& os) const {
        key->Print(os);
        os << "->" << Ptr() << endl;
    }
#endif
    // access to private members
    const RTkey& bbox() const { return Key(); }
    double xlower() { return Key().xl; }
    double xupper() { return Key().xh; }
    double ylower() { return Key().yl; }
    double yupper() { return Key().yh; }
    void setxlower(double d) { Key().xl = d; }
    void setxupper(double d) { Key().xh = d; }
    void setylower(double d) { Key().yl = d; }
    void setyupper(double d) { Key().yh = d; }

};

#endif


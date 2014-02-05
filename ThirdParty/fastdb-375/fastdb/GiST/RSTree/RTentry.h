// -*- Mode: C++ -*-

//           RTentry.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RSTree/RTentry.h,v 1.1 2007/07/18 20:45:47 knizhnik Exp $

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

// RTpenalty is a vector of 3 doubles, to support two tie-breakers.
// See the RTentry::Penalty method for more detail.
class RTpenalty : public GiSTpenalty
{
  
public:
  RTpenalty() {
    amount = 0;
    amount2 = 0;
    amount3 = 0;
  }
  RTpenalty(const double d) : amount2(0), amount3(0) {};
  RTpenalty(const double d1, const double d2, const double d3) {
    amount = d1;
    amount2 = d2;
    amount3 = d3;
  }
  double operator = (const double d) { 
    amount = d; amount2 = amount3 = 0; 
    return(amount);
  }
  int operator < (const RTpenalty &p) const {
    if (amount < p.amount) return(1);
    if (amount > p.amount) return(0);
    if (amount2 < p.amount2) return(1);
    if (amount2 > p.amount2) return(0);
    if (amount3 < p.amount3) return(1);
    return(0);
  }

  double amount2;
  double amount3;
};

// RTkey is a rectangle represented as 4 doubles:
//   xlow, xhigh, ylow and yhigh
// We also throw in some standard geo operators to support various 
// query predicates, etc.

class RTkey : GiSTobject
{
public:
  friend RTentry;
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

  double dist(const RTkey& b) {
    return(sqrt(pow(fabs(((b.xh + b.xl)/2)-((xh + xl)/2)), 2) 
		+ pow(fabs(((b.yh + b.yl) - 2)-((yh + yl)/2)),2)));
  }

  RTkey *intersect(const RTkey& k) {
    RTkey *result;
    
    if (!overlap(k))
      return((RTkey *)NULL);  
    result = new RTkey(MAX(xl,k.xl), MIN(xh,k.xh), MAX(yl,k.yl), 
                       MIN(yh,k.yh));
      
    return(result);
  }

  RTkey *expand(const RTkey& k) {
    RTkey *result;

    result = new RTkey(MIN(xl, k.xl), MAX(xh, k.xh), MIN(yl, k.yl),
                       MAX(yh, k.yh));
    return(result);
  }

  void Print(ostream& os) const {
    os << '"';
    os << "(" << xl << "," << xh << "," << yl << "," << yh << ")";
    os << '"';
  }

  double xl;
  double xh;
  double yl;
  double yh;
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
    RTkey &Key() const { return *(RTkey *)key; }
  public:
    RTentry() {}

    RTentry(const RTkey& k, const GiSTpage p) {
      key = (GiSTobject *)new RTkey(k);
      ptr = p;
    }
    RTentry(const RTentry& e) : GiSTentry(e) {
      key = (GiSTobject *)new RTkey(*(RTkey *)(e.key));
    }

    ~RTentry() { 
      if (key)
	delete key;
      key = NULL;
    }

    // The methods we're required to supply
    GiSTpenalty *Penalty(const GiSTentry &newEntry) const;
    int CompressedLength() const {
      return sizeof(RTkey);
    }
#ifdef PRINTING_OBJECTS
    void Print(ostream& os) const {
        key->Print(os);
        os << "->" << Ptr() << endl;
    }
#endif

    GiSTobject *Copy() const { return new RTentry(*this); }

    GiSTobjid IsA() const { return RTENTRY_CLASS; }

    int IsEqual(const GiSTobject& obj) const {
        if (obj.IsA() != RTENTRY_CLASS)
            return 0;
        const RTentry& rte = (const RTentry&) obj;
        return ((*((RTkey *)key) == (*((RTkey *)(rte.key)))));
    }
    const RTkey& bbox() const { return Key(); }
    void InitKey() { key = new RTkey; }

    double xlower() { return Key().xl; }
    double xupper() { return Key().xh; }
    double ylower() { return Key().yl; }
    double yupper() { return Key().yh; }
    void setxlower(double d) { Key().xl = d; }
    void setxupper(double d) { Key().xh = d; }
    void setylower(double d) { Key().yl = d; }
    void setyupper(double d) { Key().yh = d; }

    double OverlapArea(const RTkey &k) const;
};

#endif

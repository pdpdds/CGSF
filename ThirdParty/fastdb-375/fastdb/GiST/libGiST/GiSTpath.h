// -*- Mode: C++ -*-

//         GiSTpath.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiSTpath.h,v 1.1 2007/07/18 20:45:50 knizhnik Exp $
#ifndef GISTPATH_H
#define GISTPATH_H

// GiSTpath objects store the path to a node from the root of the tree.
// The page number of each node visited is stored.  For simplicity,
// GiSTpaths are stored as an array with max GIST_MAX_LEVELS elements.
// This can be expanded if you want a tree with more than 16 levels
// (not likely unless you have big keys, or small pages!)

#define GiSTRootPage 1

#define GIST_MAX_LEVELS 16

class GiSTpath : public GiSTobject
{
    GiSTpage pages[GIST_MAX_LEVELS];
    int len;
  public:
    GiSTpath() {
	len = 0;
    }
    GiSTpath(const GiSTpath& path) {
	len = path.len;
	for (int i=0; i<path.len; i++) pages[i] = path.pages[i];
    }
#ifdef PRINTING_OBJECTS
    void Print(ostream& os) const {
	os << '[';
	for (int i=0; i<len; i++) {
	    if (i) os << ' ';
	    os << pages[i];
	}
	os << ']';
    }
#endif
  //    int Level() const {
  //	return len-1;
  //    }
    void Clear() {
	len = 0;
    }
    void MakeParent() {
	len--;
    }
    void MakeSibling(GiSTpage page) {
	pages[len-1] = page;
    }
    void MakeChild(GiSTpage page) {
	pages[len++] = page;
    }
    void MakeRoot() {
	len=1;
	pages[0]=GiSTRootPage;
    }
    void SplitRoot(GiSTpage page) {
	for (int i=len; i>1; i--) pages[i] = pages[i-1];
	pages[1] = page;
	pages[0] = GiSTRootPage;
    }
    GiSTpath& operator = (const GiSTpath& path) {
	len = path.len;	
	for (int i=0; i<path.len; i++) pages[i] = path.pages[i];
	return *this;
    }
    int operator == (const GiSTpath& p) const {
        if (len != p.len)
	    return(0);
        for (int i=0; i < p.len; i++)
	    if (pages[i] != p.pages[i])
		return(0);
	return(1);
    }
    GiSTpage Page() const { return pages[len-1]; }
    GiSTpage Parent() const { return len >= 2 ? pages[len-2] : 0; }
    int IsRoot() const { return len==1; }
};

#endif

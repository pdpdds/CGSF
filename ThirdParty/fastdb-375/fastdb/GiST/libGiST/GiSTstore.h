// -*- Mode: C++ -*-

//          GiSTstore.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiSTstore.h,v 1.1 2007/07/18 20:45:50 knizhnik Exp $

#ifndef GISTSTORE_H
#define GISTSTORE_H

#include "GiSTdefs.h"

// GiSTstore is the parent class for storage of GiST pages.  GiSTfile is 
// provided as an example descendant of GiSTstore.

class GiSTstore {
public:
  GiSTstore() : isOpen(0) {}
  virtual void Create(const char *filename) = 0;
  virtual void Open(const char *filename) = 0;
  virtual void Close() = 0;
  virtual void Read(GiSTpage page, char *buf) = 0;
  virtual void Write(GiSTpage page, const char *buf) = 0;
  virtual GiSTpage Allocate() = 0;
  virtual void Deallocate(GiSTpage page) = 0;
  virtual void Sync() = 0;
  virtual int IsOpen() { return isOpen; }
  virtual int PageSize() const = 0;
  virtual ~GiSTstore() { }
protected:
  void SetOpen(int o) { isOpen = o; }
private:
  int isOpen;
};

#endif

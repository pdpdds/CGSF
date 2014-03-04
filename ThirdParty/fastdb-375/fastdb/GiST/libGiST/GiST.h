// -*- Mode: C++ -*-

//         GiST.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiST.h,v 1.1 2007/07/18 20:45:49 knizhnik Exp $

#ifndef GIST_H
#define GIST_H

#include "GiSTdefs.h"
#include "GiSTentry.h"
#include "GiSTlist.h"
#include "GiSTnode.h"
#include "GiSTstore.h"
#include "GiSTpredicate.h"
#include "GiSTcursor.h"

class GiST : public GiSTobject
{
public:
  GiST();

  // a likely candidate for overloading
  GiSTobjid IsA() const { return GIST_CLASS; }

  void Create(const char *filename);
  void Open(const char *filename);
  void Close();



  GiSTcursor *Search(const GiSTpredicate &query) const;
  void Insert(const GiSTentry& entry);
  void Delete(const GiSTpredicate& pred);
  void Sync();
  GiSTstore *Store() { return store; }

  int IsOpen() { return isOpen; }
  virtual int IsOrdered() const { return 0; }
  virtual int ForcedReinsert() const { return 0; }

#ifdef PRINTING_OBJECTS
  void DumpNode(ostream& os, GiSTpath path) const;
  void Print(ostream& os) const;
#endif

  ~GiST();

protected:
  // The following must be overriden by descendant classes
  virtual GiSTstore *CreateStore() const = 0;
  virtual GiSTnode  *CreateNode()  const = 0;

  // These are default supports for Forced Reinsert
  virtual GiSTlist<GiSTentry*> RemoveTop(GiSTnode *node);
  virtual double RemoveRatio() const { return 0.3; }

  // Reads/writes nodes
  GiSTnode *ReadNode(const GiSTpath& path) const;
  void WriteNode(GiSTnode *node);


private:
  void InsertHelper(const GiSTentry &entry, int level, int *lvl_split = NULL);
  GiSTnode *ChooseSubtree(GiSTpage page, const GiSTentry &entry, int level);
  void Split(GiSTnode **node, const GiSTentry& entry);
  void AdjustKeys(GiSTnode *node, GiSTnode **parent);
  int CondenseTree(GiSTnode *leaf);
  void ShortenTree();
  void OverflowTreatment(GiSTnode *node, const GiSTentry& entry, 
			 int *lvl_split);
  GiSTnode *NewNode(GiST *tree) const {
      GiSTnode *retval = CreateNode();
      retval->SetTree(tree);
      return(retval);
  }

  GiSTstore *store;
  int isOpen;

  int debug;

  friend GiSTcursor;
};

#endif

// -*- Mode: C++ -*-

//         GiST.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiST.cpp,v 1.1 2007/07/18 20:45:49 knizhnik Exp $

#include <string.h>

#include "GiST.h"
#include "GiSTpath.h" // for GiSTRootPage


TruePredicate truePredicate;

GiST::GiST()
{
    isOpen = 0;
    debug = 0;
}

void 
GiST::Create(const char *filename)
{
    GiSTpage page;

    if (IsOpen()) return;

    store = CreateStore();

    store->Create(filename);
    if (!store->IsOpen())
	return;

    page = store->Allocate();

    GiSTnode *node = NewNode(this);
    node->Path().MakeRoot();

    WriteNode(node);

    delete node;

    isOpen = 1;
}

void 
GiST::Open(const char *filename)
{
    if (IsOpen())
	return;

    store = CreateStore();

    store->Open(filename);
    if (!store->IsOpen())
	return;

    isOpen = 1;
}

void 
GiST::Close()
{
    if (IsOpen()) {
	store->Close();
	isOpen = 0;
    }
}

void 
GiST::Insert(const GiSTentry &entry)
{
    InsertHelper(entry, 0);
}

void 
GiST::InsertHelper(const GiSTentry &entry, 
		   int level, // level of tree at which to insert
		   int *splitvec) // a vector to trigger Split
                                  // instead of forced reinsert
{
    GiSTnode *leaf;
    int overflow = 0;

    leaf = ChooseSubtree(GiSTRootPage, entry, level);
    leaf->Insert(entry);
    if (leaf->IsOverFull(*store)) {
        if (ForcedReinsert() & !leaf->Path().IsRoot() 
	    && (!splitvec || !splitvec[level])) {
	    // R*-tree-style forced reinsert
	    int split[GIST_MAX_LEVELS];
	    for (int i=0; i < GIST_MAX_LEVELS; i++) 
	      split[i] = 0;
	    OverflowTreatment(leaf, entry, split);
	    overflow = 1;
	}
	else {
	    Split(&leaf, entry);
	}
	if (leaf->IsOverFull(*store)) {
	    // we only should get here if we reinserted, and the node 
	    // re-filled
	    assert(overflow);
	    leaf->DeleteEntry(entry.Position());
	    Split(&leaf, entry);
	}
    }
    else
        WriteNode(leaf);
    if (!overflow)
        AdjustKeys(leaf, NULL);
    delete leaf;
}

void 
GiST::OverflowTreatment(GiSTnode *node, const GiSTentry& entry, 
			int *splitvec)
{
    GiSTlist<GiSTentry*> deleted;

    // remove the "top" p entries from the node
    deleted = RemoveTop(node);
    WriteNode(node);

    // AdjustKeys 
    AdjustKeys(node, NULL);

    // note that we've seen this level already
    splitvec[node->Level()] = 1;

    // for each of the deleted entries, call InsertHelper at this level
    while (!deleted.IsEmpty()) {
      GiSTentry *tmpentry = deleted.RemoveFront();
      InsertHelper(*tmpentry, node->Level(), splitvec);
      delete tmpentry;
    }
}


GiSTlist<GiSTentry*>
GiST::RemoveTop(GiSTnode *node)
{
    GiSTlist<GiSTentry*> deleted;
    int count = node->NumEntries();

    // default: remove the first ones on the page
    int num_rem = (int)((count + 1)*RemoveRatio() + 0.5);
    for (int i = num_rem - 1; i >= 0; i--) {
        deleted.Append((GiSTentry *)(*node)[i].Ptr()->Copy());
        node->DeleteEntry(i);
    }
    return(deleted);
}

void 
GiST::AdjustKeys(GiSTnode *node, GiSTnode **parent)
{
    GiSTnode *P;

    if (node->Path().IsRoot())
	return;

    // Read in node's parent
    if (parent == NULL) {
	GiSTpath parent_path = node->Path();
	parent_path.MakeParent();
	P = ReadNode(parent_path);
	parent = &P;
    } else
	P = *parent;

    // Get the old entry pointing to node
    GiSTentry *entry = P->SearchPtr(node->Path().Page());
    assert(entry != NULL);

    // Get union of node
    GiSTentry *actual = node->Union();
    actual->SetPtr(node->Path().Page());

    if (!entry->IsEqual(*actual)) {
	int pos = entry->Position();
	P->DeleteEntry(pos);
	P->InsertBefore(*actual, pos);
	// A split may be necessary.
	// XXX: should we do Forced Reinsert here too?
	if (P->IsOverFull(*store)) {
	    Split(parent, *actual);
	    GiSTpage page = node->Path().Page();
	    node->Path() = P->Path();
	    node->Path().MakeChild(page);
	} else {
	    WriteNode(P);
	    AdjustKeys(P, NULL);
	}
    }

    if (parent == &P) delete P;

    delete actual;
    delete entry;
}

void 
GiST::Sync()
{
    store->Sync();
}

void 
GiST::Delete(const GiSTpredicate& pred)
{
    GiSTcursor *c = Search(pred);
    int condensed;


    GiSTentry *e;

    do {
      if (c == NULL) return;
      e = c->Next();

      GiSTpath path = c->Path();
      delete c;

      if (e == NULL) return;

      // Read in the node that this belongs to
      GiSTnode *node = ReadNode(path);
      node->DeleteEntry(e->Position());
      WriteNode(node);

      condensed = CondenseTree(node);
      delete node;

      if (condensed) {
	ShortenTree();
	// because the tree changed, we need to search all over again!
	// XXX - this is inefficient!  users may want to avoid condensing.
	c = Search(pred);
      }
    } while (e != NULL);
}

void
GiST::ShortenTree()
{
    GiSTpath path;

    // Shorten the tree if necessary
    // (This should only be done if root actually changed!)

    path.MakeRoot();
    GiSTnode *root = ReadNode(path);

    if (!root->IsLeaf() && root->NumEntries() == 1) {
	path.MakeChild((*root)[0]->Ptr());
	GiSTnode *child = ReadNode(path);
	store->Deallocate(path.Page());
	child->SetSibling(0);
	child->Path().MakeRoot();
	WriteNode(child);
	delete child;
    }

    delete root;
}

// handle underfull leaf nodes
int
GiST::CondenseTree(GiSTnode *node)
{
    GiSTlist<GiSTentry*> Q;
    int deleted = 0;

    // Must be condensing a leaf
    assert(node->IsLeaf());

    while (!node->Path().IsRoot()) {

	GiSTpath parent_path;
	parent_path = node->Path();
	parent_path.MakeParent();

	GiSTnode *P = ReadNode(parent_path);

	GiSTentry *En;

	En = P->SearchPtr(node->Path().Page());
	assert(En != NULL);

	// Handle under-full node
	if (node->IsUnderFull(*store)) {
	    
	    if (!IsOrdered()) {
		GiSTlist<GiSTentry*> list = node->Search(truePredicate);
		while (!list.IsEmpty()) {
		    GiSTentry *e = list.RemoveFront();
		    Q.Append(e);
		}
		P->DeleteEntry(En->Position());
		WriteNode(P);
		deleted = 1;
		AdjustKeys(P, NULL);
	    }

	    else {
		// Try to borrow entries, else coalesce with a neighbor
		// Have to look at left sibling???

		GiSTpage neighbor_page = P->SearchNeighbors(node->Path().Page());
		GiSTpath neighbor_path = node->Path();
		neighbor_path.MakeSibling(neighbor_page);


		if (neighbor_page != 0) {
		    GiSTnode *neighbor;

		    // If neighbor is RIGHT sibling...
		    if (node->Sibling() == neighbor_page) {
			neighbor = ReadNode(neighbor_path);
		    } else {
			neighbor = node;
			node = ReadNode(neighbor_path);
		    }

		    GiSTentry *e = P->SearchPtr(node->Path().Page());

		    node->Coalesce(*neighbor, *e);

		    delete e;

		    // If not overfull, coalesce, kill right node
		    if (!node->IsOverFull(*store)) {
			node->SetSibling(neighbor->Sibling());
			WriteNode(node);

			// Delete the neighbor from parent
			GiSTentry *e = P->SearchPtr(neighbor->Path().Page());
			P->DeleteEntry(e->Position());
			WriteNode(P);
			delete e;

			store->Deallocate(neighbor->Path().Page());
			deleted = 1;
		    }
		    // If overfull, split (same as borrowing)
		    else {
			GiSTnode *node2 = node->PickSplit();

			node2->Path() = neighbor->Path();
			node2->SetSibling(neighbor->Sibling());

			WriteNode(node);
			WriteNode(node2);

			AdjustKeys(node2, &P);

			delete node2;
			deleted = 1;
		    }

		    delete neighbor;
		}
	    }
	}

	// Adjust covering predicate
	if (!deleted) AdjustKeys(node, &P);

	parent_path = node->Path();
	parent_path.MakeParent();

	delete node;

	// Propagate deletes
	if (!deleted) break;

	node = P;
    }

    // Re-insert orphaned entries
    while (!Q.IsEmpty()) {
	GiSTentry *e = Q.RemoveFront();
	InsertHelper(*e, e->Level());
	delete e;
    }
    return(deleted);
}

GiSTnode* 
GiST::ChooseSubtree(GiSTpage page,
		    const GiSTentry &entry,
		    int level)
{
    GiSTnode *node;
    GiSTpath path;

    for (;;) {
	path.MakeChild(page);
	node = ReadNode(path);
	if (level == node->Level() || node->IsLeaf())
	    break;
	page = node->SearchMinPenalty(entry);
	delete node;
    }

    return node;
}

void 
GiST::Split(GiSTnode **node, const GiSTentry& entry)
{
    int went_left = 0, new_root = 0;

    if ((*node)->Path().IsRoot()) {
	new_root = 1;
	(*node)->Path().MakeChild(store->Allocate());
    }

    GiSTnode *node2 = (*node)->PickSplit();
    node2->Path().MakeSibling(store->Allocate());

    GiSTentry *e = (*node)->SearchPtr(entry.Ptr());
    if (e != NULL) {
	went_left = 1;
	delete e;
    }

    node2->SetSibling((*node)->Sibling());
    (*node)->SetSibling(node2->Path().Page());

    WriteNode(*node);
    WriteNode(node2);

    GiSTentry *e1 = (*node)->Union();
    GiSTentry *e2 = node2->Union();
    e1->SetPtr((*node)->Path().Page());
    e2->SetPtr(node2->Path().Page());

    // Create new root if root is being split
    if (new_root) {
	GiSTnode *root = NewNode(this);
	root->SetLevel((*node)->Level() + 1);
	root->InsertBefore(*e1, 0);
	root->InsertBefore(*e2, 1);
	root->Path().MakeRoot();
	WriteNode(root);
	delete root;
    } else {
	// Insert entry for N' in parent
	GiSTpath parent_path = (*node)->Path();
	parent_path.MakeParent();

	GiSTnode *parent = ReadNode(parent_path);

	// Find the entry for N in parent
	GiSTentry *e = parent->SearchPtr((*node)->Path().Page());
	assert(e != NULL);

	// Insert the new entry right after it
	int pos = e->Position();
	parent->DeleteEntry(pos);
	parent->InsertBefore(*e1, pos);
	parent->InsertBefore(*e2, pos+1);
	delete e;

	if (!parent->IsOverFull(*store))
	    WriteNode(parent);
	else {
	    Split(&parent, went_left ? *e1 : *e2);
	    GiSTpage page = (*node)->Path().Page();
	    (*node)->Path() = parent->Path();
	    (*node)->Path().MakeChild(page);
	    page = node2->Path().Page();
	    node2->Path() = (*node)->Path();
	    node2->Path().MakeSibling(page);
	}
	    
	delete parent;
    }

    if (!went_left) {
	delete *node;
	*node = node2;
    } else
	delete node2;

    delete e1;
    delete e2;
}

GiSTnode* 
GiST::ReadNode(const GiSTpath& path) const
{
    char *buf = new char[store->PageSize()];

    GiSTnode *node = NewNode((GiST *)this);
    store->Read(path.Page(), buf);
    node->Unpack(buf);

#ifdef PRINTING_OBJECTS
    if (debug) {
	cout << "READ PAGE " << path.Page() << ":\n";
	node->Print(cout);
    }
#endif

    node->Path() = path;

	delete buf;

    return node;
}

void 
GiST::WriteNode(GiSTnode *node)
{
    char *buf = new char[store->PageSize()];
    
    // make Purify happy
    memset(buf, 0, store->PageSize());

#ifdef PRINTING_OBJECTS
    if (debug) {
	cout << "WRITE PAGE " << node->Path().Page() << ":\n";
	node->Print(cout);
    }
#endif
    node->Pack(buf);
    store->Write(node->Path().Page(), buf);
    delete buf;
}

#ifdef PRINTING_OBJECTS

void 
GiST::DumpNode(ostream& os, GiSTpath path) const
{
#ifdef PRINTING_OBJECTS
    GiSTnode *node = ReadNode(path);
    

    node->Print(os);
    if (!node->IsLeaf()) {
	GiSTlist<GiSTentry*> list = node->Search(truePredicate);
	while (!list.IsEmpty()) {
	    GiSTentry *e = list.RemoveFront();
	    path.MakeChild(e->Ptr());
	    DumpNode(os, path);
	    path.MakeParent();
	    delete e;
	}
    }
    delete node;
#endif
}

void 
GiST::Print(ostream& os) const
{
    GiSTpath path;

    path.MakeRoot();
    DumpNode(os, path);
}

#endif

GiSTcursor* 
GiST::Search(const GiSTpredicate &query) const
{
    return new GiSTcursor(*this, query);
}

GiST::~GiST()
{
    Close();
    delete store;
}

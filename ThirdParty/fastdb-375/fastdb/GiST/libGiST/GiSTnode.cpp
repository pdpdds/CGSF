// -*- Mode: C++ -*-

//          GiSTnode.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiSTnode.cpp,v 1.1 2007/07/18 20:45:50 knizhnik Exp $

#include <string.h>

#include "GiST.h"

const int ALLOC_INCR = 32;

extern "C" int 
GiSTintcmp(const void *x, const void *y)
{
    int i = *(int *)x;
    int j = *(int *)y;
    return(i-j);
}

int 
GiSTnode::Size() const
{
    int size = GIST_PAGE_HEADER_SIZE + sizeof(GiSTlte);
    int fixlen = FixedLength();

    if (fixlen)
	size += numEntries * (fixlen + sizeof(GiSTpage));
    else for (int i=0; i<numEntries; i++)
	size += (sizeof(GiSTlte) +
		 sizeof(GiSTpage) +
		 entries[i]->CompressedLength());
    return size;
}

GiSTnode::GiSTnode()
{
    packedNode = NULL;
    sibling = 0;
    level = 0;
    numEntries = 0;
    maxEntries = 0;
    entries = NULL;
    tree = NULL;
}

GiSTnode::GiSTnode(const GiSTnode &node)
{
    maxEntries = node.maxEntries;
    numEntries = node.numEntries;
    level = node.level;
    sibling = node.sibling;
    tree = node.tree;

    if (node.packedNode) {
	packedNode = new char[tree->Store()->PageSize()];
	memcpy(packedNode, node.packedNode, tree->Store()->PageSize());
    } else
	packedNode = NULL;
    
    if (maxEntries)
	entries = new GiSTentryptr[maxEntries];
    else
	entries = NULL;
    for (int i=0; i<numEntries; i++) {
      entries[i] = (GiSTentry*) node.entries[i]->Copy();
      ((GiSTentry *)entries[i])->SetNode(this);
    }

    path = node.path;
}

void 
GiSTnode::Expand(int index)
{
    int newMaxEntries = maxEntries + ALLOC_INCR;
    if (newMaxEntries < index + 1) newMaxEntries = index + 1 + ALLOC_INCR;
    GiSTentryptr *newEntries = new GiSTentryptr[newMaxEntries];
	int i;


    for (i=numEntries; i<newMaxEntries; i++)
	newEntries[i] = NULL;
    for (i=0; i<numEntries; i++)
	newEntries[i] = entries[i];
    if (entries != NULL)
	delete entries;
    entries = newEntries;
    maxEntries = newMaxEntries;
}

GiSTentryptr& 
GiSTnode::operator [] (int index)
{
    assert(index >= 0);
    if (index >= maxEntries)
	Expand(index);
    return entries[index];
}

const GiSTentryptr& 
GiSTnode::operator [] (int index) const
{
    static GiSTentryptr e;

    assert(index >= 0);
    if (index >= maxEntries)
	return e;
    return entries[index];
}

void 
GiSTnode::InsertBefore(const GiSTentry& entry, int index)
{
    assert(index <= NumEntries());

    GiSTentry *e = (GiSTentry*) entry.Copy();
    e->SetLevel(Level());
    e->SetPosition(index);
    e->SetNode(this);

    // Move everything else over
    for (int i = NumEntries(); i > index; i--) {
	GiSTentry *e = (*this)[i-1];
	e->SetPosition(i);
	(*this)[i] = e;
    }
    
    // Stick the entry in
    (*this)[index] = e;

    // Bump up the count
    numEntries++;
}

void 
GiSTnode::Insert(const GiSTentry& entry)
{
    // Find out where to insert it
    int i;
    for (i=0; i<NumEntries(); i++)
	if ((*this)[i]->Compare(entry) > 0)
	    break;

    // Do the deed
    InsertBefore(entry, i);
}

void 
GiSTnode::DeleteEntry(int index)
{
    int j;

    assert(index < numEntries);

    // free up the memory in the entry to be deleted
    if (entries[index].Ptr())
      delete entries[index].Ptr();

    // Remove the entry
    for (j=index; j<numEntries-1; j++) {
	entries[j] = entries[j+1];
	entries[j]->SetPosition(j);
    }

    numEntries--;
}

void
GiSTnode::DeleteBulk(int vec[], int veclen)
{
  int i;

  qsort(vec, veclen, sizeof(int), GiSTintcmp);
  for (i = veclen - 1; i >= 0; i--)
    DeleteEntry(vec[i]);
}

void 
GiSTnode::Pack(char *page) const
{
  // Pack the header
  GiSTheader *h  = (GiSTheader *) page;
  h->level     = Level();
  h->numEntries = NumEntries();
  h->sibling    = Sibling();

  int fixlen = FixedLength();
  GiSTlte *ltable = (GiSTlte *) (page+tree->Store()->PageSize());
  GiSTlte ltptr = GIST_PAGE_HEADER_SIZE;

  for (int i=0; i<numEntries; i++) {
      GiSTcompressedEntry compressedEntry = (*this)[i]->Compress();

      if (fixlen)
	  assert(fixlen == compressedEntry.keyLen);

      // Copy the entry onto the page
      if (compressedEntry.keyLen > 0)
	  memcpy(page+ltptr,
		 compressedEntry.key,
		 compressedEntry.keyLen);

      memcpy(page+ltptr+compressedEntry.keyLen,
	     &compressedEntry.ptr,
	     sizeof(GiSTpage));

      // Be tidy
      if (compressedEntry.key)
	delete compressedEntry.key;

      // Enter a pointer to the entry in the line table
      if (!fixlen) *--ltable = ltptr;

      int entryLen = compressedEntry.keyLen + sizeof(GiSTpage);
      ltptr += entryLen;
  }

  // Store extra line table entry so we know last entry's length
  *--ltable = ltptr;
}

void 
GiSTnode::Unpack(const char *page)
{
    const GiSTheader *h = (const GiSTheader *) page;

    Reset();

    SetLevel(h->level);
    SetSibling(h->sibling);

    if (!packedNode)
	packedNode = new char[tree->Store()->PageSize()];
    memcpy(packedNode, page, tree->Store()->PageSize());

    Expand(h->numEntries);
    SetNumEntries(h->numEntries);

    for (int i=0; i<h->numEntries; i++) {
        GiSTcompressedEntry tmpentry = Entry(i);
	GiSTentry *e = CreateEntry();
	e->SetLevel(Level());
	e->SetPosition(i);
	e->SetNode(this);
	e->Decompress(tmpentry);
	// be tidy
	if (tmpentry.key)
	  delete tmpentry.key;

	// Append the body with the entry
	entries[i] = e;
    }
}

// SearchMinPenalty returns where a new entry should be inserted.
GiSTpage 
GiSTnode::SearchMinPenalty(const GiSTentry &newEntry) const
{
    GiSTentry *minEntry = NULL;
    GiSTpenalty *minPenalty = NULL;

    for (int i=0; i<numEntries; i++) {
	GiSTentry *e = (*this)[i];
	assert(e->Node() == this);
	GiSTpenalty *penalty = e->Penalty(newEntry);
	if (minEntry == NULL || (*penalty) < (*minPenalty)) {
	    minEntry = e;
	    if (minPenalty) delete minPenalty;
	    minPenalty = penalty;
	}
	else 
	  delete penalty;
    }
    delete minPenalty;
    return minEntry->Ptr();
}

void 
GiSTnode::Coalesce(const GiSTnode &source,
		   const GiSTentry& entry) // entry is the entry in the
                                           // parent node that points to this
{
    // Coalesce by one-by-one insertion
    //   Take each entry from the source node
    //   and insert it into this node.
    for (int i=0; i<source.numEntries; i++) {
	GiSTentry& e = source[i];
	InsertBefore(e, NumEntries());
    }
}

GiSTcompressedEntry 
GiSTnode::Entry(int entryPos) const
{
  // Look up the line table
  GiSTlte keyPhysicalPos, nextKeyPhysicalPos;

  int fixlen = FixedLength();

  if (!fixlen) {
      memcpy(&keyPhysicalPos,
	     packedNode+tree->Store()->PageSize()-(entryPos+1)*sizeof(GiSTlte),
	     sizeof(GiSTlte));

      memcpy(&nextKeyPhysicalPos,
	     packedNode+tree->Store()->PageSize()-(entryPos+2)*sizeof(GiSTlte),
	     sizeof(GiSTlte));
  } else {
      keyPhysicalPos = GIST_PAGE_HEADER_SIZE+(sizeof(GiSTpage)+fixlen)*entryPos;
      nextKeyPhysicalPos = keyPhysicalPos+sizeof(GiSTpage)+fixlen;
  }

  // Allocate and set up the return key
  GiSTcompressedEntry entry;
  entry.keyLen = nextKeyPhysicalPos - keyPhysicalPos - sizeof(GiSTpage);
  if (entry.keyLen > 0) {
    entry.key = new char[entry.keyLen];
    memcpy(entry.key,
	   packedNode+keyPhysicalPos, entry.keyLen);
  }
  memcpy(&entry.ptr,
	 packedNode+keyPhysicalPos+entry.keyLen, sizeof(GiSTpage));
  return entry;
}

GiSTlist<GiSTentry*> 
GiSTnode::Search(const GiSTpredicate &query) const
{
    GiSTlist<GiSTentry*> list;

    for (int i=0; i<numEntries; i++) {
	GiSTentry *e = (*this)[i];
	if (query.Consistent(*e))
	    list.Append((GiSTentry*)e->Copy());
    }
    return list;
}

GiSTentry* 
GiSTnode::SearchPtr(GiSTpage ptr) const
{
    for (int i=0; i<numEntries; i++) {
	GiSTentry *e = (*this)[i];
	if (e->Ptr() == ptr)
	    return (GiSTentry*) e->Copy();
    }
    return NULL;
}

#ifdef PRINTING_OBJECTS
void 
GiSTnode::Print(ostream& os) const
{
  os << path << " #Entries: " << NumEntries() << ", ";

  os << "Level " << Level();
  if (IsLeaf())
    os << "(Leaf)";
  else
    os << "(Internal)";

  os << ", Sibling: " << Sibling();
  os << ", Size: " << Size() << "/" << tree->Store()->PageSize() << endl;

  for (int i = 0; i<numEntries; i++)
      (*this)[i]->Print(os);
}
#endif

GiSTpage 
GiSTnode::SearchNeighbors(GiSTpage ptr) const
{
    for (int i=0; i<numEntries; i++)
	if ((*this)[i]->Ptr() == ptr) {
	    // Is there a right neighbor?
	    if (i != numEntries-1)
		return (*this)[i+1]->Ptr();
	    // Is there a left neighbor?
	    if (i != 0)
		return (*this)[i-1]->Ptr();
	    break;
	}
    return 0;
}

GiSTnode* 
GiSTnode::PickSplit()
{
    // Create the right node.  Make it an exact copy of this one.
    GiSTnode *node = (GiSTnode*) Copy();
    int half = numEntries/2;

    // Delete the first N/2 entries from the right node.
    int i;
    node->numEntries = 0;
    for (i=0; i<numEntries-half; i++) {
	node->entries[i] = node->entries[i+half];
	node->entries[i]->SetPosition(i);
	node->numEntries++;
    }

    // Delete the last N/2 entries from the left node.
    numEntries = half;

    // Return the right node.
    return node;
}

void 
GiSTnode::Reset()
{
    if (entries != NULL) {
        for (int i=0; i<numEntries; i++) {
	    delete entries[i].Ptr();
	}
	delete entries;
	entries = NULL;
    }
    if (packedNode) {
	delete packedNode;
	packedNode = NULL;
    }
    numEntries = maxEntries = 0;
}

GiSTnode::~GiSTnode()
{
    Reset();
}

int 
GiSTnode::IsUnderFull(const GiSTstore &store) const
{
    return Size() < store.PageSize() / 2;
}

int 
GiSTnode::IsOverFull(const GiSTstore &store) const
{
    return Size() > store.PageSize();
}

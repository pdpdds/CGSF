// Mode: -*- C++ -*-

//          GiSTcursor.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiSTcursor.cpp,v 1.1 2007/07/18 20:45:49 knizhnik Exp $

#include "GiST.h"

GiSTcursor::GiSTcursor(const GiST& gist,
		       const GiSTpredicate& query) : gist(gist)
{
    this->query = (GiSTpredicate*) query.Copy();
    first = 1;
    lastlevel = -1;
}

GiSTentry* 
GiSTcursor::Next()
{
    GiSTpage page;

    while (first || !stack.IsEmpty()) {
	if (first) {
	    page = GiSTRootPage;
	    first = 0;
	} else {
	    assert(lastlevel >= 0);
	    GiSTentry *entry = stack.RemoveFront();
	    if (entry->IsLeaf())
		return entry;

	    // Pop off the stack
	    for (int i=0; i < entry->Level() - lastlevel; i++)
		path.MakeParent();

	    page = entry->Ptr();
	    
	    delete entry;
	}

	// Entry was a pointer to another node
	path.MakeChild(page);

	GiSTnode *node = gist.ReadNode(path);
	lastlevel = node->Level();

	GiSTlist<GiSTentry*> list = node->Search(*query);

	while (!list.IsEmpty()) {
	    GiSTentry *entry = list.RemoveRear();
	    stack.Prepend(entry);
	}

	delete node;
    }

    // The search is over...
    return NULL;
}

GiSTcursor::~GiSTcursor()
{
    if (query != NULL)
	delete query;
    while (!stack.IsEmpty())
	delete stack.RemoveFront();
}

const GiSTpath& 
GiSTcursor::Path() const
{
    return path;
}

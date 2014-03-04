// -*- Mode: C++ -*- ----------------------------------------
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/BTree/command.h,v 1.1 2007/07/18 20:45:46 knizhnik Exp $
//
//  command.h
//  Test Program: Command Parser .h
//
// ----------------------------------------------------------

#ifndef COMMAND_H
#define COMMAND_H

#include "GiSTdb.h"

class Record { 
  public:
    int value;

    TYPE_DESCRIPTOR((FIELD(value)));
};

void CommandCreate(const char *method,
		   const char *table);

void CommandDrop(const char *table);

void CommandOpen(const char *method, const char *table);

void CommandClose(const char *table);

void CommandSelect(const char *table,
		   const GiSTpredicate& pred);

void CommandDelete(const char *table,
		   const GiSTpredicate& pred);

void CommandInsert(const char *table,
		   const BTkey& key,
		   int ptr);

void CommandDump(const char *table, GiSTpage page);

void CommandQuit();

void CommandDebug();

void CommandHelp();

#endif

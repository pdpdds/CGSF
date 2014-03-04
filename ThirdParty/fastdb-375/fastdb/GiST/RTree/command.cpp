// -*- Mode: C++ -*-

//         command.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/RTree/command.cpp,v 1.1 2007/07/18 20:45:48 knizhnik Exp $

#include <stdio.h>
#include <fstream.h>
#ifdef UNIX
#include <unistd.h>
#include <ctype.h>
#endif

#include "RT.h"
#include "command.h"

struct Table
{
    char *name;
    GiST *gist;
};

#define MAX_TABLES 10
#define NOT_FOUND (-1)

dbDatabase db;

REGISTER(Record);

Table tables[MAX_TABLES];
int numTables = 0;
int debug = 0;

int 
GetTable(const char *name)
{
    int i;

    for (i=0; i<numTables; i++)
	if (!strcmp(tables[i].name, name))
	    break;
    return i == numTables ? NOT_FOUND : i;
}

void 
CommandCreate(const char *method,
	      const char *table)
{
    if (numTables == MAX_TABLES) {
	cout << "This progam can only handle "<<MAX_TABLES<<" open tables.\n";
	return;
    }

    if (GetTable(table) != NOT_FOUND) {
	cerr << "Table already open!\n";
	return;
    }

	GiST *gist;
    if (!strcmp(method, "rtree"))
		gist = new RT(db);
	else {
		cerr << "The only supported method is rtree.\n";
		return;
    }

    gist->Create(table);
    if (!gist->IsOpen()) {
	cout << "Error opening table.\n";
	delete gist;
	return;
    }

    cout << "Table " << table << " created as type " << method << ".\n";

    tables[numTables].name = strdup(table);
    tables[numTables].gist = gist;
    numTables++;
}

void 
CommandDrop(const char *table)
{
    if (GetTable(table) != NOT_FOUND) {
	cerr << "Cannot drop an open table.\n";
	return;
    }
    cout << "Table " << table << " dropped.\n";
}

void 
CommandOpen(const char *method, const char *table)
{
    GiST *gist;
    if (numTables == MAX_TABLES) {
	cout << "This progam can only handle "<<MAX_TABLES<<" open tables.\n";
	return;
    }

    if (GetTable(table) != NOT_FOUND) {
	cout << "Table already open!\n";
	return;
    }

    if (!strcmp(method, "rtree"))
      gist = new RT(db);
    else {
      cerr << "The only supported method is rtree.\n";
      return;
    }

    gist->Open(table);

    if (!gist->IsOpen()) {
	delete gist;
	cout << "Error opening table.\n";
	return;
    }

    cout << "Table " << table << " opened.\n";

    tables[numTables].gist = gist;
    tables[numTables].name = strdup(table);
    numTables++;
}

void 
CommandClose(const char *table)
{
    int i;

    i = GetTable(table);
    if (i == NOT_FOUND) {
	cerr << "Table is not open.\n";
	return;
    }

    cout << "Table " << table << " closed.\n";
    delete tables[i].gist;

    for ( ; i < numTables-1; i++)
	tables[i] = tables[i+1];
    numTables--;
}

void 
CommandSelect(const char *table,
	      const GiSTpredicate& pred)
{
    int i;

    i = GetTable(table);
    if (i == NOT_FOUND) {
	cerr << "Table is not open!\n";
	return;
    }

    GiST *gist = tables[i].gist;

    GiSTcursor *c = gist->Search(pred);
    GiSTentry *e;
    dbCursor<Record> cursor;
    while ((e = c->Next()) != NULL) {
	dbReference<Record> ref(e->Ptr());
	cursor.at(ref);
	cout << cursor->value << "\n";
	delete e;
    }
    delete c;
}

void 
CommandDelete(const char *table,
	      const GiSTpredicate& pred)
{
    int i;

    if ((i = GetTable(table)) == NOT_FOUND) {
	cerr << "Table not open!\n";
	return;
    }

    if (i != NOT_FOUND)
	tables[i].gist->Delete(pred);
}

void 
CommandInsert(const char *table,
	      const RTkey& key,
	      int value)
{
    int i;

    if ((i = GetTable(table)) == NOT_FOUND) {
	cerr << "Table not open!\n";
	return;
    }

    Record rec;
    rec.value = value;
    dbReference<Record> ref = insert(rec);
    GiST *gist = tables[i].gist;
    gist->Insert(RTentry(key, ref.getOid()));
    cout << "(" << key << ", " << value << ") inserted into " << table << ".\n";
}

void 
CommandQuit()
{
    for (int i=0; i<numTables; i++)
	delete tables[i].gist;
    cout << "Goodbye.\n";
    db.close();
    exit(0);
}

void 
CommandPrompt()
{
    cout << "RTree> ";
	cout.flush();
}

void 
CommandDebug()
{
    debug = !debug;
    cout << "Debugging Output ";
    cout << (debug ? "ON\n" : "OFF\n");
}

void 
CommandHelp()
{
    ifstream is("RTree.help");
    char c;

    while (is.get(c)) cout << c;
}

void 
CommandDump(const char *table, GiSTpage page)
{
    int i;

    if ((i = GetTable(table)) == NOT_FOUND) {
	cout << "No such table is open.\n";
	return;
    }

    GiSTpath path;
    path.MakeRoot();
#ifdef PRINTING_OBJECTS
    tables[i].gist->DumpNode(cout, path);
#endif
}

int yyparse();

int main()
{
    cout << "**  RTree: An R-Tree based on Generalized Search Trees\n";
    cout << "Type \"help\" for a list of commands.\n\n";
    if (db.open("tstrtree")) { 
      CommandPrompt();
      yyparse();
      return 0;
    } else { 
      return 1;
    }
}






//-< TESTTL.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     23-Aug-2010  K.A. Knizhnik  * / [] \ *
//                          Last update: 23-Aug-2010  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Transaction logging demo
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>

USE_FASTDB_NAMESPACE

const int nRecords = 1000000;

class Employee {
  public:
    char const* name;
    int age;

    TYPE_DESCRIPTOR((KEY(name, INDEXED), FIELD(age)));
};

class Developer;

class Manager : public Employee {
  public:
    dbArray< dbReference<Developer> > team;

    TYPE_DESCRIPTOR((SUPERCLASS(Employee), OWNER(team, manager)));
};

class Developer :    public Employee {
  public:
    dbReference<Manager> manager;

    TYPE_DESCRIPTOR((SUPERCLASS(Employee), RELATION(manager, team)));
};

REGISTER(Manager);
REGISTER(Developer);


int main(int argc, char* argv[])
{
    dbDatabase db;
    dbFileTransactionLogger tl;
    Developer d;
    Manager m;
    size_t n;    
        
    // Open database and transaction logger
    db.open(_T("testtl"));
    tl.open(_T("testtl.log"), dbFile::truncate|dbFile::no_sync);
    db.setTransactionLogger(&tl);

    // Insert some manager
    m.name = "John Smith";
    dbReference<Manager> manager = insert(m);        
    
    // Add one developer in his team...
    d.name = "Peter Green";
    d.age = 39;
    d.manager = manager;
    insert(d);
    
    // ... and yet another developer
    d.name = "Greg Tomas";
    d.age = 25;
    d.manager = manager;
    insert(d);
    db.commit();
    
    // Update daveloper's data
    dbCursor<Developer> developers(dbCursorForUpdate);
    developers.select("name='Greg Tomas'");
    developers->age = 35;
    developers.update();
    db.commit();
    
    // Add new developer to the team        
    d.name = "Tom Dron";
    d.age = 37;
    d.manager = manager;
    insert(d);
    db.commit();
    
    // Create new team
    m.name = "Mark Overgard";
    manager = insert(m);        
    
    d.name = "Tim Brown";
    d.age = 23;
    d.manager = manager;
    insert(d);
    db.commit();
    
    // And remove this team
    dbCursor<Manager> managers(dbCursorForUpdate);
    managers.at(manager);
    managers.remove();        
    db.commit();

    // Close database and transaction logger
    db.close();
    tl.close();
    
    // Remove and reopen database 
    remove("testtl.fdb");
    db.open(_T("testtl"));

    // Restore database from transaction log
    tl.open(_T("testtl.log"), dbFile::read_only);
    tl.restore(db, n);
    printf("Restore %d transactions\n", (int)n);
    tl.close();
    
    // Print list of developers
    developers.select();
    n = 0;
    do { 
        printf("%s %d\n", developers->name, developers->age);
        n += 1;
    } while (developers.next());
    assert(n == 3);
    db.commit();

    // Continue logging at the end of file
    tl.open(_T("testtl.log"), dbFile::no_sync);
    db.setTransactionLogger(&tl);
    
    // Remove some developer
    developers.select("name='Peter Green'");
    developers.remove();
    db.commit();
    
    // Close database and transaction logger
    db.close();
    tl.close();
    // Remove and reopen database 
    remove("testtl.fdb");
    db.open(_T("testtl"));

    // Restore database from transaction log
    tl.open(_T("testtl.log"), dbFile::read_only);
    tl.restore(db, n);
    printf("Restore %d transactions\n", (int)n);
    tl.close();
    
    // Print list of developers
    developers.select();
    n = 0;
    do { 
        printf("%s %d\n", developers->name, developers->age);
        n += 1;
    } while (developers.next());
    assert(n == 2);

    db.close();
}




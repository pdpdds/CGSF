//-< BUGDB.H  >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     27-Mar-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 25-Jun-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example of database Web publishing: Bug Tracking Database
//-------------------------------------------------------------------*--------*

#ifndef __BUGDB_H__
#define __BUGDB_H__

#include "fastdb.h"
#include "wwwapi.h"

USE_FASTDB_NAMESPACE

class Bug;
class Software;

class Person
{
  public:
    char const* sName;
    char const* sEmailAddress;
    char const* sPassword;

    dbArray<dbReference<Software> >  setUsedSoftware;
    dbArray<dbReference<Software> >  setProjects;
    dbArray<dbReference<Bug> >       setReportedBugs;
    dbArray<dbReference<Bug> >       setAssignedBugs;

    enum PersonStatus { 
        isUser, 
        isEngineer,
        isAdministrator
    };
    int1 status;
    int4 nReports;

    void print(WWWconnection& con) const;
    
    bool checkPassword(char const* password) const { 
        return strcmp(sPassword, password) == 0;
    }


    TYPE_DESCRIPTOR((KEY(sName, INDEXED),
                     FIELD(sEmailAddress),
                     FIELD(sPassword),
                     RELATION(setReportedBugs, pReportedBy),
                     RELATION(setAssignedBugs, pAssignedTo),
                     RELATION(setUsedSoftware, setUsers), 
                     RELATION(setProjects, setEngineers), 
                     FIELD(status),FIELD(nReports)));
};


class Version 
{
  public:
    char const* sLabel;
    int         majorVersionNumber;
    int         minorVersionNumber;
    dbDateTime  released;
    char const* sComment;
    dbArray<dbReference<Bug> > setBugs;
    dbReference<Version>       pNext;

    int   getVersion() const; 
    char* getVersionString() const;

    void print(WWWconnection& con) const;

    TYPE_DESCRIPTOR((FIELD(sLabel),
                     FIELD(majorVersionNumber),
                     FIELD(minorVersionNumber),
                     FIELD(released),
                     FIELD(sComment),
                     RELATION(setBugs, pVersion),
                     FIELD(pNext)));
};


class Software
{
  public:
    char const*                   sName;
    dbArray<dbReference<Person> > setEngineers;
    dbArray<dbReference<Person> > setUsers;
    dbReference<Version>          pVersions;
    dbArray<dbReference<Bug> >    setBugs;

    int     getLastVersion() const;
    char*   getLastVersionString() const;

    void print(WWWconnection& con) const;
    
    TYPE_DESCRIPTOR((KEY(sName, INDEXED),
                     FIELD(pVersions),
                     RELATION(setEngineers, setProjects),
                     RELATION(setUsers, setUsedSoftware),
                     RELATION(setBugs, pSoftware)));
};

enum eCATEGORY
{
    CATEGORY_NOT_SPECIFIED,
    CRASH,
    PROGRAM_HANGS,
    UI_DISPLAY,
    UI_BEHAVIOR,
    CALCULATION,
    ERROR_HANDLING,
    PERFORMANCE,
    LICENSING,
    INSTALLATION,
    DOCUMENTATION,
    ENHANCEMENT,
    HOW_TO_QUESTION
};
    
extern char const* const eCATEGORY_STRING[];
    
enum eSTATUS
{
    STATUS_NOT_SPECIFIED,
    STATUS_OPENED,
    STATUS_FIXED,
    STATUS_CLOSED,
    STATUS_PENDING_Person,
    STATUS_PENDING_USER,
    STATUS_POSTPONED,
    STATUS_IRREPRODUCIBLE,
    STATUS_WITHDRAWN,
    STATUS_AS_DESIGNED
};

extern char const* const eSTATUS_STRING[];
    
enum eFIXING_PRIORITY
{
    PRIORITY_NOT_SPECIFIED,
    FIX_IMMEDIATELY,
    FIX_BEFORE_NEXT_BUILD_RELEASE,
    FIX_BEFORE_NEXT_MINOR_RELEASE,
    FIX_BEFORE_NEXT_MAJOR_RELEASE,
    FIX_IF_POSSIBLE,
    OPTIONAL
};

extern char const* const eFIXING_PRIORITY_STRING[];

enum eSEVERITY
{
    SEVERITY_NOT_SPECIFIED,
    FATAL,
    SERIOUS,
    MINOR,
    LAST_SEVERITY
};

extern char const* const eSEVERITY_STRING[]; 


class Report
{
  public:
    char const*         sDescription;
    dbReference<Person> pAuthor;
    dbReference<Report> pNext;
    dbDateTime          creationDate;
    int1                status;
    int4                index;

    void print(WWWconnection& con) const;

    TYPE_DESCRIPTOR((FIELD(sDescription),FIELD(pAuthor),FIELD(pNext),FIELD(creationDate),
                     FIELD(status),FIELD(index)));
};


class Bug 
{
  public:
    char const* sOneLineSummary;
    char const* sHardwarePlatform;
    char const* sOperatingSystem;
    
    int4        bugId;
    int4        nReports;
    int1        eCategory;
    int1        eFixingPriority;
    int1        eSeverity;
    
    dbReference<Person>        pReportedBy;
    dbReference<Person>        pAssignedTo;
    
    dbReference<Software>      pSoftware;
    dbReference<Version>       pVersion;

    dbReference<Report>        pReportHistory;
    dbReference<Report>        pWorkArounds;
    dbArray<dbReference<Bug> > setSimilarBugs;

    void print(WWWconnection& con) const;

    TYPE_DESCRIPTOR((KEY(bugId, HASHED),
                     FIELD(sOneLineSummary),
                     FIELD(sOperatingSystem), 
                     FIELD(sHardwarePlatform),
                     RELATION(pSoftware, setBugs),
                     RELATION(pVersion, setBugs), 
                     RELATION(pReportedBy, setReportedBugs),
                     RELATION(pAssignedTo, setAssignedBugs),
                     RELATION(setSimilarBugs, setSimilarBugs),
                     FIELD(nReports),
                     FIELD(eCategory),
                     FIELD(eFixingPriority),
                     FIELD(eSeverity),
                     FIELD(pReportHistory), 
                     FIELD(pWorkArounds)));
};


class BugSequencer {
  public:
    int                nBugs;

    TYPE_DESCRIPTOR((FIELD(nBugs)));
};

#endif

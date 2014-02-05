//-< CLIDB.H >-------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:      7-Nov-99    K.A. Knizhnik  * / [] \ *
//                          Last update:  7-Nov-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example of Web database publishing: Clients Database
//-------------------------------------------------------------------*--------*

#ifndef __CLIDB_H__
#define __CLIDB_H__

#include "fastdb.h"
#include "wwwapi.h"

USE_FASTDB_NAMESPACE

class History;
class Manager;
class Segment;

class Client { 
  public:
    char const* organization;
    char const* phone;
    char const* phone2;
    char const* fax;
    char const* email;
    char const* www;
    char const* person;
    char const* person2;
    char const* lpr;
    char const* addr;
    char const* info;
    char const* status;
    int         clientId;
    dbReference<Segment> segment;
    dbArray<dbReference<History> > history;

    void print(WWWconnection& con) const;
    
    TYPE_DESCRIPTOR((KEY(clientId, INDEXED),
                     FIELD(organization),
                     FIELD(phone),
                     FIELD(phone2),
                     FIELD(fax),
                     FIELD(email),
                     FIELD(www),
                     FIELD(person),
                     FIELD(person2),
                     FIELD(lpr),
                     FIELD(addr),
                     FIELD(info),
                     FIELD(status),
                     RELATION(segment, clients),
                     OWNER(history, client)));
};

class History { 
  public:
    int         messageId;
    char const* message;
    dbDateTime  date;
    dbReference<Manager> manager;
    dbReference<Client>  client;
    
    void print(WWWconnection& con) const;

    TYPE_DESCRIPTOR((KEY(messageId, INDEXED),
                     FIELD(message),
                     FIELD(date),
                     RELATION(client, history),
                     RELATION(manager, messages)));
};

class Manager { 
  public:
    char const* name;
    char const* IPaddr;
    int         flags;
    dbReference<Segment> segment;
    dbArray<dbReference<History> > messages;

    static bool loginFromAnyHost;

    enum { 
        isMinAdmin      = 1,
        isAdministrator = 2
    };
    
    void print(WWWconnection& con) const;

    TYPE_DESCRIPTOR((KEY(name, INDEXED),
                     FIELD(IPaddr),
                     FIELD(flags),
                     RELATION(segment, managers),
                     RELATION(messages, manager)));
};

class Segment { 
  public:
    char const* name;
    dbArray<dbReference<Client> >  clients;
    dbArray<dbReference<Manager> > managers;
 
    void print(WWWconnection& con) const;

    TYPE_DESCRIPTOR((KEY(name, INDEXED),
                     OWNER(clients, segment),
                     OWNER(managers, segment)));
};

class Sequencer { 
  public:
    int lastClientId;
    int lastMessageId;

    TYPE_DESCRIPTOR((FIELD(lastClientId),
                     FIELD(lastMessageId)));
};

class ThreadContext { 
  public:
    dbCursor<Client>  clients;
    dbCursor<Manager> managers;
    dbCursor<Segment> segments;
    dbCursor<History> messages;
    dbCursor<Sequencer> sequencer;

    dbQuery qClient;
    dbQuery qManager;
    dbQuery qClients;
    dbQuery qManagers;
    dbQuery qSegment;
    dbQuery qMessage;
    dbQuery qMessages;
    
    bool  managerForm;

    int   clientId;
    int   messageId;
    char* name;
    dbArray<dbReference<Client> >*  clientsArray;
    dbArray<dbReference<Manager> >* managersArray;
    dbArray<dbReference<History> >* messagesArray;
    
    ThreadContext();
    static ThreadContext& of(WWWconnection& con);
};

#endif

//-< CLIDB.CPP >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:      7-Nov-99    K.A. Knizhnik  * / [] \ *
//                          Last update:  7-Nov-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example of Web database publishing: Clients Database
//-------------------------------------------------------------------*--------*

#include "clidb.h"

dbDatabase db;

//- Client ------------------------------------------------

void Client::print(WWWconnection& con) const 
{ 
    con << TAG << "<OPTION VALUE=" << clientId << ">" << organization 
        << "</OPTION>";
}
    
REGISTER(Client);


//- Manager ------------------------------------------------

bool Manager::loginFromAnyHost;

void Manager::print(WWWconnection& con) const 
{ 
    con << TAG << "<OPTION VALUE=\"" << name << "\">" << name 
        << "</OPTION>";
}
    
REGISTER(Manager);

//- Segment ------------------------------------------------

void Segment::print(WWWconnection& con) const 
{ 
    con << TAG << "<OPTION VALUE=\"" << name << "\">" << name 
        << "</OPTION>";
}
    
REGISTER(Segment);

//- History ------------------------------------------------

void History::print(WWWconnection& con) const 
{     
    ThreadContext& the = ThreadContext::of(con);
    char buf[64];
    
    con << TAG << "<OPTION VALUE=" << messageId << ">"  
        << date.asString(buf, sizeof buf)
        << " " << (the.managerForm 
                   ? the.clients.at(client)->organization 
                   : manager != null ? the.managers.at(manager)->name : "")
        << "</OPTION>";
}
    
REGISTER(History);

//---- ClientSequencer -------------------------------------

REGISTER(Sequencer);


template<class T>
void print(WWWconnection& con, dbCursor<T>& cursor) {
    do { 
        cursor->print(con);
    } while(cursor.next());
}

template<class T>
void print(WWWconnection& con, dbArray<dbReference<T> > const& arr) {
    dbCursor<T> cursor;
    for (int i = 0, n = (int)arr.length(); i < n; i++) { 
        cursor.at(arr[i])->print(con);
    } 
}

//---- Thread Context --------------------------------------------------

ThreadContext& ThreadContext::of(WWWconnection& con)
{
    if (con.userData == NULL) { 
        con.userData = new ThreadContext;
    }
    return *(ThreadContext*)con.userData;
}

ThreadContext::ThreadContext() 
{
    qClient = "clientId=",clientId;
    qManager = "name=",&name;
    qClients = "start from",clientsArray,"order by organization";
    qManagers = "start from",managersArray,"order by name";
    qSegment = "name=",&name;
    qMessage = "messageId=",messageId;
    qMessages = "start from",messagesArray,
                "order by",dbDateTime::descent("date");
}

//--- HTML specific part -------------------------------------------

#define HTML_HEAD "Content-type: text/html\r\n\r\n\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\"><HTML><HEAD>"

#define BODY "<BODY BGCOLOR=\"#c0c0c0\">"

#define EMPTY_LIST "<OPTION>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</OPTION></SELECT><BR>"

void mainMenuReference(WWWconnection& con)
{
    char* manager = con.get("manager");
    if (manager != NULL) { 
        con << TAG 
            << "<P><HR><CENTER><A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress() << "&page=managerForm&manager=" 
            << URL << manager << "\">Back to main menu</A></CENTER>";
    }
    con << TAG << "</BODY></HTML>";
}

bool error(WWWconnection& con, char const* msg)
{
    con << TAG << 
        HTML_HEAD "<TITLE>ClientDB error</TITLE></HEAD><BODY>"
        "<H1><FONT COLOR=\"#FF0000\">"
        << msg << "</FONT></H1></BODY></HTML>";
    mainMenuReference(con);
    return true;
}



bool addManagerForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Add new manager</TITLE></HEAD>"
        BODY
        "<H2>Add manager</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"addManager\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << con.get("manager") <<
        "\">"
        "<INPUT TYPE=hidden NAME=\"segment\" VALUE=\"" << con.get("segment") <<
        "\"><TABLE>"
        "<TR><TH ALIGN=LEFT>Manager name</TH>"
        "<TD><INPUT TYPE=text NAME=\"name\" SIZE=30></TD></TR>"
        "<TR><TH ALIGN=LEFT>Host IP address</TH>"
        "<TD><INPUT TYPE=text NAME=\"ipaddr\" SIZE=16 VALUE=\"*\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Administrator rights</TH>"
        "<TD><INPUT TYPE=CHECKBOX NAME=\"miniadmin\" VALUE=\"on\"></TD></TR>"
        "</TABLE>"
        "<INPUT TYPE=submit VALUE=\"Add\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\"></FORM>";
    mainMenuReference(con);
    return true;
}

bool addClientForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Add new client</TITLE></HEAD>"
        BODY
        "<H2>Add client</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"addClient\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << con.get("manager") <<
        "\">"
        "<INPUT TYPE=hidden NAME=\"segment\" VALUE=\"" << con.get("segment") <<
        "\"><TABLE>"
        "<TR><TH ALIGN=LEFT>Organization</TH>"
        "<TD><INPUT TYPE=text NAME=\"organization\" SIZE=40></TD></TR>"
        "<TR><TH ALIGN=LEFT>Phone</TH>"
        "<TD><INPUT TYPE=text NAME=\"phone\" SIZE=20></TD></TR>"
        "<TR><TH ALIGN=LEFT>Phone2</TH>"
        "<TD><INPUT TYPE=text NAME=\"phone2\" SIZE=20></TD></TR>"
        "<TR><TH ALIGN=LEFT>Fax</TH>"
        "<TD><INPUT TYPE=text NAME=\"fax\" SIZE=20></TD></TR>"
        "<TR><TH ALIGN=LEFT>E-Mail</TH>"
        "<TD><INPUT TYPE=text NAME=\"email\" SIZE=30></TD></TR>"
        "<TR><TH ALIGN=LEFT>WWW address</TH>"
        "<TD><INPUT TYPE=text NAME=\"www\" SIZE=30></TD></TR>"
        "<TR><TH ALIGN=LEFT>Person</TH>"
        "<TD><INPUT TYPE=text NAME=\"person\" SIZE=40></TD></TR>"
        "<TR><TH ALIGN=LEFT>Person2</TH>"
        "<TD><INPUT TYPE=text NAME=\"person2\" SIZE=40></TD></TR>"
        "<TR><TH ALIGN=LEFT>CEO</TH>"
        "<TD><INPUT TYPE=text NAME=\"lpr\" SIZE=40></TD></TR>"
        "<TR><TH ALIGN=LEFT>Address</TH>"
        "<TD><INPUT TYPE=text NAME=\"addr\" SIZE=40></TD></TR>"
        "<TR><TH ALIGN=LEFT>Information</TH>"
        "<TD><INPUT TYPE=text NAME=\"info\" SIZE=40></TD></TR>"
        "<TR><TH ALIGN=LEFT>Status</TH>"
        "<TD><INPUT TYPE=text NAME=\"status\" SIZE=40></TD></TR>"
        "</TABLE><INPUT TYPE=submit VALUE=\"Add\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\"></FORM>";
    mainMenuReference(con);
    return true;
}


bool addSegmentForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Add new segment</TITLE></HEAD>"
        BODY
        "<H2>Add segment</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"addSegment\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << con.get("manager") <<
        "\"><TABLE>"
        "<TR><TH ALIGN=LEFT>Segment name </TH>"
        "<TD><INPUT TYPE=text NAME=\"name\" SIZE=30></TD></TR>"
        "</TD></TR></TABLE><P>"
        "<INPUT TYPE=submit VALUE=\"Add\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\"></FORM>";
    mainMenuReference(con);
    return true;
}

bool addHistoryForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Add message</TITLE></HEAD>"
        BODY
        "<H2>Add message</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"addHistory\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << con.get("manager") <<
        "\"><INPUT TYPE=hidden NAME=\"client\" VALUE=" << con.get("client") <<
        "\"><TEXTAREA NAME=\"message\" ROWS=10 COLS=60></TEXTAREA><P>"
        "<INPUT TYPE=submit VALUE=\"Add\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\"></FORM>";
    mainMenuReference(con);
    return true;
}


bool editManagerForm(WWWconnection& con)
{
    if (con.get("new")) { 
        return addManagerForm(con);
    }
    ThreadContext& the = ThreadContext::of(con);
    the.name = con.get("name");
    if (the.name == NULL) { 
        return error(con, "No manager was selected");
    }
    if (the.managers.select(the.qManager) == 0) { 
        return error(con, "No such manager");
    }
    
    con << TAG << 
        HTML_HEAD "<TITLE>Edit manager form</TITLE></HEAD>"
        BODY
        "<H2>Update manager</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editManager\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << con.get("manager") <<
        "\"><INPUT TYPE=hidden NAME=\"oldName\" VALUE=\"" << the.name << "\">"
        "<TABLE><TR><TH ALIGN=LEFT>Manager name</TH>"
        "<TD><INPUT TYPE=text NAME=\"name\" SIZE=30 VALUE=\"" 
        << the.managers->name << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Host IP address</TH>"
        "<TD><INPUT TYPE=text NAME=\"ipaddr\" SIZE=16 VALUE=\""
        << the.managers->IPaddr << "\"</TD></TR>"
        "<TR><TH ALIGN=LEFT>Administrator rights</TH>"
        "<TD><INPUT TYPE=CHECKBOX NAME=\"miniadmin\" VALUE=\"on\""
        << ((the.managers->flags & Manager::isMinAdmin)
            ? "CHECKED" : "") << "></TD></TR></TABLE>"
        "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Update\">&nbsp;"
        "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Remove\"></FORM><P>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editHistoryForm\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << con.get("manager") <<
        "\"><SELECT NAME=\"messageId\" SIZE=15>";
    the.managerForm = true;
    if (the.managers->messages.length() != 0) { 
        print(con, the.managers->messages);
        con << TAG << 
            "</SELECT><BR><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\"></FORM>";
    } else { 
        con << TAG << EMPTY_LIST "</FORM>";
    }
    mainMenuReference(con);
    return true;
}

bool editClientForm(WWWconnection& con)
{
    if (con.get("new")) { 
        return addClientForm(con);
    }
    ThreadContext& the = ThreadContext::of(con);
    char* client = con.get("client");
    if (client == NULL) { 
        return error(con, "No client was selected");
    }
    the.clientId = atoi(client);
    if (the.clients.select(the.qClient) == 0) { 
        return error(con, "No such client");
    }
    the.name = con.get("manager");
    if (the.managers.select(the.qManager) == 0) { 
        return error(con, "No such manager");
    }
    con << TAG << 
        HTML_HEAD "<TITLE>Edit client form</TITLE></HEAD>"
        BODY
        "<H2>Update client</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editClient\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << the.name << "\">"
        "<INPUT TYPE=hidden NAME=\"client\" VALUE=" << client << "\">"
        "<TABLE>"
        "<TR><TH ALIGN=LEFT>Organization</TH>"
        "<TD><INPUT TYPE=text NAME=\"organization\" SIZE=40 VALUE=\""
        << the.clients->organization << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Phone</TH>"
        "<TD><INPUT TYPE=text NAME=\"phone\" SIZE=20 VALUE=\""
        << the.clients->phone << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Phone2</TH>"
        "<TD><INPUT TYPE=text NAME=\"phone2\" SIZE=20 VALUE=\""
        << the.clients->phone2 << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Fax</TH>"
        "<TD><INPUT TYPE=text NAME=\"fax\" SIZE=20 VALUE=\""
        << the.clients->fax << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>E-Mail</TH>"
        "<TD><INPUT TYPE=text NAME=\"email\" SIZE=30 VALUE=\""
        << the.clients->email << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>WWW address</TH>"
        "<TD><INPUT TYPE=text NAME=\"www\" SIZE=30 VALUE=\""
        << the.clients->www << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Person</TH>"
        "<TD><INPUT TYPE=text NAME=\"person\" SIZE=40 VALUE=\""
        << the.clients->person << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Person2</TH>"
        "<TD><INPUT TYPE=text NAME=\"person2\" SIZE=40 VALUE=\""
        << the.clients->person2 << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>CEO</TH>"
        "<TD><INPUT TYPE=text NAME=\"lpr\" SIZE=40 VALUE=\""
        << the.clients->lpr << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Address</TH>"
        "<TD><INPUT TYPE=text NAME=\"addr\" SIZE=40 VALUE=\""
        << the.clients->addr << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Information</TH>"
        "<TD><INPUT TYPE=text NAME=\"info\" SIZE=40 VALUE=\""
        << the.clients->info << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Status</TH>"
        "<TD><INPUT TYPE=text NAME=\"status\" SIZE=40 VALUE=\""
        << the.clients->status << "\"></TD></TR></TABLE><P>";
    if (the.managers->flags & (Manager::isAdministrator|Manager::isMinAdmin)) {
        con << TAG 
            << "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Update\">&nbsp;";
    }
    if (the.managers->flags & Manager::isAdministrator) {
        con << TAG << "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Remove\">";
    }
    con << TAG << "</FORM><P>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editHistoryForm\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << the.name << "\">"
        "<INPUT TYPE=hidden NAME=\"client\" VALUE=" << client << "\">"
        "<SELECT SIZE=10 NAME=\"messageId\">";
    the.managerForm = false;
    the.messagesArray = &the.clients->history;
    if (the.messages.select(the.qMessages) != 0) { 
        print(con, the.messages);
        con << TAG << 
            "</SELECT><BR><INPUT TYPE=submit VALUE=\"Select\">&nbsp;";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << 
        "<INPUT TYPE=submit NAME=\"new\" VALUE=\"New message\"></FORM>";
    mainMenuReference(con);
    return true;
}


bool editSegmentForm(WWWconnection& con)
{
    if (con.get("new")) { 
        return addSegmentForm(con);
    }
    ThreadContext& the = ThreadContext::of(con);
    the.name = con.get("name");
    if (the.name == NULL) { 
        return error(con, "No segment was selected");
    }
    if (the.segments.select(the.qSegment) == 0) { 
        return error(con, "No such segment");
    }
    char* manager = con.get("manager");

    con << TAG << 
        HTML_HEAD "<TITLE>Edit segment</TITLE></HEAD>"
        BODY
        "<H2>Update segment</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editSegment\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << manager << "\">"
        "<INPUT TYPE=hidden NAME=\"oldName\" VALUE=\"" << the.name << "\">"
        "<TABLE><TR><TH ALIGN=LEFT>Segment name </TH>"
        "<TD><INPUT TYPE=text NAME=\"name\" SIZE=30 VALUE=\""
        << the.segments->name << "\"></TD></TR>"
        "</TD></TR></TABLE><P>"
        "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Update\">&nbsp;"
        "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Remove\"></FORM><P>"

        "<H2>Select manager</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editManagerForm\">"
        "<INPUT TYPE=hidden NAME=\"segment\" VALUE=\"" << the.name << "\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << manager << "\">"
        "<SELECT NAME=\"name\" SIZE=10>";
    the.managersArray = &the.segments->managers;
    if (the.managers.select(the.qManagers) != 0) { 
        print(con, the.managers);
        con << TAG << 
            "</SELECT><BR><INPUT TYPE=submit VALUE=\"Select\">&nbsp;";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "<INPUT TYPE=submit NAME=\"new\" VALUE=\"New\"></FORM><P>"
        "<H2>Select client</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editClientForm\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << manager << "\">"
        "<INPUT TYPE=hidden NAME=\"segment\" VALUE=\"" << the.name << "\">"
        "<SELECT NAME=\"client\" SIZE=10>";
    the.clientsArray = &the.segments->clients;
    if (the.clients.select(the.qClients) != 0) { 
        print(con, the.clients);
        con << TAG << 
            "</SELECT><BR><INPUT TYPE=submit VALUE=\"Select\">&nbsp;";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "<INPUT TYPE=submit NAME=\"new\" VALUE=\"New\"></FORM>";

    mainMenuReference(con);
    return true;
}


bool editHistoryForm(WWWconnection& con)
{
    if (con.get("new")) { 
        return addHistoryForm(con);
    }
    ThreadContext& the = ThreadContext::of(con);
    char buf[64];
    char* messageId = con.get("messageId");
    if (messageId == NULL) { 
        return error(con, "No message was selected");
    }
    the.messageId = atoi(messageId);
    if (the.messages.select(the.qMessage) == 0) { 
        return error(con, "No such message");
    }
    the.name = con.get("manager");
    the.clients.at(the.messages->client); 
    if (the.messages->manager != null) { 
        the.managers.at(the.messages->manager);
    }
    con << TAG << 
        HTML_HEAD "<TITLE>Edit message</TITLE></HEAD>"
        BODY
        "<H2>Edit message</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editHistory\">"
        "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << the.name << "\">"
        "<INPUT TYPE=hidden NAME=\"messageId\" VALUE=" << messageId << "\">"
        "<TABLE><TR><TH ALIGN=LEFT>Client</TH>"
        "<TD><A HREF=\"" << con.getStub() << "?socket=" << con.getAddress()
        << "&page=editClientForm&manager=" << URL << the.name
        << "&client=" << the.clients->clientId << "\">" 
        << the.clients->organization << "</A></TD></TR>";
    if (the.messages->manager != null) { 
        con << TAG << 
            "<TR><TH ALIGN=LEFT>Manager</TH><TD><A HREF=\""
            << con.getStub() << "?socket=" << con.getAddress()
            << "&page=editManagerForm&manager=" << URL << the.name
            << "&name=" << URL << the.managers->name << "\">" 
            << the.managers->name << "</A></TD></TR>";
    }
    con << TAG << 
        "<TR><TH ALIGN=LEFT>Date</TH>"
        "<TD><INPUT TYPE=text SIZE=30 VALUE=\"" 
        << the.messages->date.asString(buf, sizeof buf) << "\"></TD></TR>"
        "</TABLE><P>"
        "<TEXTAREA NAME=\"message\" ROWS=10 COLS=60>" 
        << the.messages->message << "</TEXTAREA><P>";
    if (the.managers.select(the.qManager) == 0) { 
        return error(con, "No such manager");
    }
    if ((the.managers->flags & (Manager::isAdministrator|Manager::isMinAdmin)) 
        || the.managers.currentId() == the.messages->manager) 
    {
        con << TAG 
            << "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Update\">&nbsp;"
               "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Remove\">";
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}


bool managerForm(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    the.name = con.get("manager");
    if (the.managers.select(the.qManager) == 0) { 
        return error(con, "No such manager");
    }
    if (the.managers->flags & Manager::isAdministrator) { 
        con << TAG << 
            HTML_HEAD "<TITLE>Administrator menu</TITLE></HEAD>"
            BODY
            "<H2>Administrator menu</H2>"
            "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\""<< con.getAddress() <<
            "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editManager\">"
            "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << the.name << "\">"
            "<INPUT TYPE=hidden NAME=\"oldName\" VALUE=\"" << the.name << "\">"
            "<TABLE><TR><TH ALIGN=LEFT>Manager name</TH>"
            "<TD><INPUT TYPE=text NAME=\"name\" SIZE=30 VALUE=\"" 
            << the.managers->name << "\"></TD></TR>"
            "<TR><TH ALIGN=LEFT>Manager host IP address</TH>"
            "<TD><INPUT TYPE=text NAME=\"ipaddr\" SIZE=16 VALUE=\""
            << the.managers->IPaddr << "\"</TD></TR></TABLE>"
            "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Change\"></FORM><P>"
            "<H2>Select segment</H2>"
            "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\""<< con.getAddress() <<
            "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editSegmentForm\">"
            "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << the.name << "\">"
            "<SELECT SIZE=15 NAME=\"name\">";
        if (the.segments.select() != 0) { 
            print(con, the.segments);
            con << TAG << 
                "</SELECT><BR><INPUT TYPE=submit VALUE=\"Select\">&nbsp;";
        } else { 
            con << TAG << EMPTY_LIST;
        }
        con << TAG << 
            "<INPUT TYPE=submit NAME=\"new\" VALUE=\"New segment\"></FORM>"
            "<P><HR>"
            "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\""<< con.getAddress() <<
            "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"shutdown\">"
            "<CENTER><INPUT TYPE=submit VALUE=\"Shutdown server\"></CENTER>"
            "</FORM>";
    } else { 
        con << TAG << 
            HTML_HEAD "<TITLE>Manager menu</TITLE></HEAD>"
            BODY
            "<H2>Manager menu</H2>"
            "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\""<< con.getAddress() <<
            "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editManager\">"
            "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << the.name << "\">"
            "<INPUT TYPE=hidden NAME=\"oldName\" VALUE=\"" << the.name << "\">";
        if (the.managers->flags & Manager::isMinAdmin) {        
            con << TAG <<"<INPUT TYPE=hidden NAME=\"miniadmin\" VALUE=\"on\">";
        }
        the.segments.at(the.managers->segment);
        con << TAG << 
            "<TABLE><TR><TH ALIGN=LEFT>Manager name</TH>"
            "<TD><INPUT TYPE=text NAME=\"name\" SIZE=30 VALUE=\"" 
            << the.managers->name << "\"></TD></TR>"
            "<TR><TH ALIGN=LEFT>Manager host IP address</TH>"
            "<TD><INPUT TYPE=text NAME=\"ipaddr\" SIZE=16 VALUE=\""
            << the.managers->IPaddr << "\"</TD></TR></TABLE>"
            "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Change\"></FORM><P>"
            "<H2>Select client</H2>"
            "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\""<< con.getAddress() <<
            "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"editClientForm\">"
            "<INPUT TYPE=hidden NAME=\"manager\" VALUE=\"" << the.name << "\">"
            "<INPUT TYPE=hidden NAME=\"segment\" VALUE=\"" 
            << the.segments->name << "\">"
            "<SELECT SIZE=15 NAME=\"client\">";
        the.clientsArray = &the.segments->clients;
        if (the.clients.select(the.qClients) != 0) { 
            print(con, the.clients);
            con << TAG << 
                "</SELECT><BR><INPUT TYPE=submit VALUE=\"Select\">&nbsp;";
        } else { 
            con << TAG << EMPTY_LIST;
        }
        con << TAG << "<INPUT TYPE=submit NAME=\"new\" VALUE=\"New\"></FORM>";
    }
    con << TAG << "</BODY></HTML>";
    return true;
}


bool login(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    the.name = con.get("name");
    if (the.managers.select(the.qManager) == 0) { 
        return error(con, "No such manager");
    } 
    if (!Manager::loginFromAnyHost 
        && strcmp(the.managers->IPaddr, "*") != 0 
        && strcmp(the.managers->IPaddr, con.getPeer()) != 0) 
    { 
        return error(con, "You can not login from this host");
    } 
    con.addPair("manager", the.name);    
    return managerForm(con);
}

bool shutdown(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>ClientDB message</TITLE></HEAD><BODY>"
        "<H1>ClientDB server is terminated</H1></BODY></HTML>";
    return false;
}

bool addManager(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    Manager manager;
    the.name = con.get("segment");
    if (the.segments.select(the.qSegment, dbCursorForUpdate) == 0) { 
        return error(con, "No such segment");
    } 
    the.name = con.get("name");
    if (the.managers.select(the.qManager) != 0) { 
        return error(con, "Manager with such name already exists");
    }
    manager.name = con.get("name");
    manager.IPaddr = con.get("ipaddr");
    manager.flags = con.get("miniadmin") ? Manager::isMinAdmin : 0;
    manager.segment = the.segments.currentId();
    insert(manager);
    return managerForm(con);
}

bool addClient(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    Client client;
    the.sequencer.select(dbCursorForUpdate);
    the.name = con.get("segment");
    if (the.segments.select(the.qSegment) == 0) { 
        return error(con, "No such segment");
    } 
    client.organization = con.get("organization");
    client.phone = con.get("phone");
    client.phone2 = con.get("phone2");
    client.fax = con.get("fax");
    client.email = con.get("email");
    client.www = con.get("www");
    client.person = con.get("person");
    client.person2 = con.get("person2");
    client.lpr = con.get("lpr");
    client.addr = con.get("addr");
    client.info = con.get("info");
    client.status = con.get("status");
    client.clientId = ++the.sequencer->lastClientId;
    client.segment = the.segments.currentId();
    the.sequencer.update();
    insert(client);
    return managerForm(con);
}

bool addSegment(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    Segment segment;
    segment.name = the.name = con.get("name");
    if (the.segments.select(the.qSegment, dbCursorForUpdate) != 0) { 
        return error(con, "Segment with such name already exists");
    }
    insert(segment);
    return managerForm(con);
}

bool addHistory(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    the.sequencer.select(dbCursorForUpdate);
    the.clientId = atoi(con.get("client"));
    if (the.clients.select(the.qClient) == 0) { 
        error(con, "No such client");
        return true;
    }
    History his;
    his.messageId = ++the.sequencer->lastMessageId;
    his.message = con.get("message");
    his.client = the.clients.currentId();
    the.name = con.get("manager");
    if (the.managers.select(the.qManager) == 0) { 
        error(con, "No such manger");
        return true;
    }
    his.manager = the.managers.currentId();
    his.date = dbDateTime::current();
    insert(his);
    the.sequencer.update();
    return managerForm(con);
}

bool editManager(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    the.name = con.get("oldName");
    if (the.managers.select(the.qManager, dbCursorForUpdate) == 0) { 
        error(con, "No such manager");
        return true;
    } 
    if (strcmp(con.get("action"), "Remove") == 0) { 
        the.managers.remove(); 
        return managerForm(con);
    }
    the.managers->name = con.get("name");
    the.managers->IPaddr = con.get("ipaddr");
    if (con.get("miniadmin")) { 
        the.managers->flags |= Manager::isMinAdmin;
    } else { 
        the.managers->flags &= ~Manager::isMinAdmin;
    }
    the.managers.update();
    return managerForm(con);
}

bool editClient(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    the.clientId = atoi(con.get("client"));
    if (the.clients.select(the.qClient, dbCursorForUpdate) == 0) { 
        error(con, "No such client");
        return true;
    } 
    if (strcmp(con.get("action"), "Remove") == 0) { 
        the.clients.remove();
    } else { 
        the.clients->organization = con.get("organization");
        the.clients->phone = con.get("phone");
        the.clients->phone2 = con.get("phone2");
        the.clients->fax = con.get("fax");
        the.clients->email = con.get("email");
        the.clients->www = con.get("www");
        the.clients->person = con.get("person");
        the.clients->person2 = con.get("person2");
        the.clients->lpr = con.get("lpr");
        the.clients->addr = con.get("addr");
        the.clients->info = con.get("info");
        the.clients->status = con.get("status");
        the.clients.update();
    }
    return managerForm(con);
}

bool editSegment(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    the.name = con.get("oldName");
    if (the.segments.select(the.qSegment, dbCursorForUpdate) == 0) { 
        error(con, "No such segment");
        return true;
    } 
    if (strcmp(con.get("action"), "Remove") == 0) { 
        the.segments.remove();
    } else { 
        the.segments->name = con.get("name");
        the.segments.update();
    }
    return managerForm(con);
}

bool editHistory(WWWconnection& con)
{
    ThreadContext& the = ThreadContext::of(con);
    the.messageId = atoi(con.get("messageId"));
    if (the.messages.select(the.qMessage, dbCursorForUpdate) == 0) { 
        error(con, "No such message");
        return true;
    } 
    if (strcmp(con.get("action"), "Remove") == 0) { 
        the.messages.remove();
    } else { 
        the.messages->message = con.get("message");
        the.messages.update();
    }
    return managerForm(con);
}


WWWapi::dispatcher dispatchTable[] = { 
    {"addManagerForm", addManagerForm},
    {"addClientForm", addClientForm},
    {"addSegmentForm", addSegmentForm},
    {"addHistoryForm", addHistoryForm},
    {"editManagerForm", editManagerForm},
    {"editClientForm", editClientForm},
    {"editSegmentForm", editSegmentForm},
    {"editHistoryForm", editHistoryForm},
    {"managerForm", managerForm},
    {"login", login},
    {"shutdown", shutdown},
    {"addManager", addManager},
    {"addClient", addClient},
    {"addSegment", addSegment},
    {"addHistory", addHistory},
    {"editManager", editManager},
    {"editClient", editClient},
    {"editSegment", editSegment},
    {"editHistory", editHistory}
};

#ifdef USE_EXTERNAL_HTTP_SERVER    
CGIapi wwwServer(db, itemsof(dispatchTable), dispatchTable);
char* defaultAddress = "localhost:6101";
socket_t::socket_domain domain = socket_t::sock_local_domain;
#else
#ifdef USE_QUEUE_MANAGER
HTTPapi wwwServer(db, itemsof(dispatchTable), dispatchTable, true);
#else
HTTPapi wwwServer(db, itemsof(dispatchTable), dispatchTable, false);
#endif
char* defaultAddress = "localhost:80";
socket_t::socket_domain domain = socket_t::sock_global_domain;
#endif

int main(int argc, char* argv[])
{
    char* address = (argc > 1) ? argv[1] : defaultAddress;
    if (!wwwServer.open(address, domain)) { 
        fprintf(stderr, "Failed to open WWW session\n");
        return EXIT_FAILURE;
    }
    if (!db.open(_T("clidb"))) { 
        fprintf(stderr, "Failed to open database\n");
        return EXIT_FAILURE;
    }
    dbCursor<Sequencer> sequencer;
    if (sequencer.select() == 0) { 
        Sequencer seq;
        seq.lastClientId = 0;
        seq.lastMessageId = 0;
        insert(seq);
        
        Manager administrator;
        administrator.name = "administrator";
        administrator.IPaddr = "*";
        administrator.flags = Manager::isAdministrator;
        insert(administrator);
    }
    db.commit();
    if (argc > 2 && strcmp(argv[2], "login_from_any_host") == 0) { 
        Manager::loginFromAnyHost = true;
    }
#ifdef USE_QUEUE_MANAGER
    QueueManager qmgr(wwwServer, db);
    qmgr.start();
#else
    WWWconnection con;
    while (wwwServer.connect(con) && wwwServer.serve(con));
#endif
    db.close(); 
    printf("End of session\n");
    return EXIT_SUCCESS;
}

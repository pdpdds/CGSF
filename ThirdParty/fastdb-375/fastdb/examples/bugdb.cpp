//-< BUGDB.CPP  >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     27-Mar-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 30-Jun-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example of database Web publishing: Bug Tracking Database
//-------------------------------------------------------------------*--------*

#include "bugdb.h"

//#define USE_EXTERNAL_HTTP_SERVER 1

char const* const eCATEGORY_STRING[] = 
{
    "",
    "CRASH",
    "PROGRAM_HANGS",
    "UI_DISPLAY",
    "UI_BEHAVIOR",
    "CALCULATION",
    "ERROR_HANDLING",
    "PERFORMANCE",
    "LICENSING",
    "INSTALLATION", 
    "DOCUMENTATION",
    "ENHANCEMENT",
    "HOW_TO_QUESTION",
    NULL
};

char const* const eSTATUS_STRING[] = { 
    "",
    "OPENED",
    "FIXED",
    "CLOSED",
    "PENDING_ENGINEER",
    "PENDING_USER",
    "POSTPONED",
    "IRREPRODUCIBLE",
    "WITHDRAWN",
    "AS_DESIGNED",
    NULL
};

char const* const eSEVERITY_STRING[] = { 
    "",
    "FATAL",
    "SERIOUS",
    "MINOR",
    NULL
};

char const* const eFIXING_PRIORITY_STRING[] = { 
    "",
    "FIX_IMMEDIATELY",
    "FIX_BEFORE_NEXT_BUILD_RELEASE",
    "FIX_BEFORE_NEXT_MINOR_RELEASE",
    "FIX_BEFORE_NEXT_MAJOR_RELEASE",
    "FIX_IF_POSSIBLE",
    "OPTIONAL",
    NULL
};

dbDatabase db;

dbCursor<Bug>          allBugs;
dbCursor<Bug>          bugs(dbCursorForUpdate);
dbCursor<Report>       reports(dbCursorForUpdate);
dbCursor<Person>       persons(dbCursorForUpdate);
dbCursor<Software>     products(dbCursorForUpdate);
dbCursor<Version>      versions(dbCursorForUpdate);
dbCursor<BugSequencer> sequencer(dbCursorForUpdate);

dbQuery qBug;
dbQuery qReport;
dbQuery qAllReports;
dbQuery qVersion;
dbQuery qAllVersions;
dbQuery qPerson;
dbQuery qSoftware;

//
// Query paramters
//
char* key;
int   bugId; 
int   reportId;
dbReference<Report>  firstReport;
dbReference<Version> initialVersion;
int   majorVersion;
int   minorVersion;


//- Person ------------------------------------------------

void Person::print(WWWconnection& con) const 
{ 
    con << TAG << "<OPTION VALUE=\"" << sName << "\">" << sName << "</OPTION>";
}
    
REGISTER(Person);

//------- Version ---------------------------------------

void Version::print(WWWconnection& con) const 
{ 
    char buf[64];
    con << TAG << "<OPTION VALUE=\"" << getVersionString() << 
        "\">Version " << getVersionString() << " " << sLabel << 
        " " << released.asString(buf, sizeof buf) << "</OPTION>";
}

int Version::getVersion() const 
{ 
    return majorVersionNumber*100 + minorVersionNumber; 
}

char* Version::getVersionString() const 
{
    static char buf[16];
    sprintf(buf, "%d.%02d", majorVersionNumber, minorVersionNumber);
    return buf;
}

REGISTER(Version);

//----- Software -------------------------------------

int Software::getLastVersion() const 
{ 
    if (pVersions == null) { 
        return 0;
    }
    versions.at(pVersions);
    return versions->getVersion();
}

char* Software::getLastVersionString() const 
{ 
    if (pVersions == null) { 
        return "";
    }
    versions.at(pVersions);
    return versions->getVersionString();
}

void Software::print(WWWconnection& con) const 
{ 
    con << TAG << "<OPTION VALUE=\"" << sName << "\">" << sName << "</OPTION>";
}

REGISTER(Software);

//----- Report -------------------------------------------

void Report::print(WWWconnection& con) const 
{ 
    char buf[64];
    if (pAuthor != null) { 
        persons.at(pAuthor);
        con << TAG << "<OPTION VALUE=" << index << ">" << persons->sName << " "
            << creationDate.asString(buf, sizeof buf) << "</OPTION>";
    } else { 
        con << TAG << "<OPTION VALUE=" << index << ">" << "Report from "
            << creationDate.asString(buf, sizeof buf) << "</OPTION>";
    }
}

REGISTER(Report);

//--- Bug -----------------------------------------

void Bug::print(WWWconnection& con) const 
{
    con << TAG << "<OPTION VALUE=" << bugId << ">" << sOneLineSummary 
        << "</OPTION>";
}
    
REGISTER(Bug);

//---- BugSequencer -------------------------------------

REGISTER(BugSequencer);

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

//--- HTML specific part -------------------------------------------

#define HTML_HEAD "Content-type: text/html\r\n\r\n\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\"><HTML><HEAD>"

#define BODY "<BODY BGCOLOR=\"#c0c0c0\">"

#define EMPTY_LIST "<OPTION>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</OPTION></SELECT>"

void mainMenuReference(WWWconnection& con)
{
    char* myself = con.get("myself");
    if (myself != NULL) { 
        con << TAG 
            << "<P><HR><CENTER><A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress() << "&page=userForm&myself=" 
            << URL << myself <<  "&name=" << URL << myself
            << "\">Back to main menu</A></CENTER>";
    }
    con << TAG << "</BODY></HTML>";
}

void error(WWWconnection& con, char const* msg)
{
    con << TAG << 
        HTML_HEAD "<TITLE>BUGDB error</TITLE></HEAD><BODY>"
        "<H1><FONT COLOR=\"#FF0000\">"
        << msg << "</FONT></H1></BODY></HTML>";
    mainMenuReference(con);
}


void message(WWWconnection& con, char const* msg)
{
    con << TAG << 
        HTML_HEAD "<TITLE>BUGDB message</TITLE></HEAD><BODY>"
        "<H1><FONT COLOR=\"#004000\">"
        << msg << "</FONT></H1></BODY></HTML>";
    mainMenuReference(con);
}


bool addUserForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Enter new user</TITLE></HEAD>"
        BODY
        "<H2>Add user</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"addUser\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><TABLE>"
        "<TR><TH ALIGN=LEFT>User name:</TH>"
        "<TD><INPUT TYPE=text NAME=\"name\" SIZE=30</TD></TR>"
        "<TR><TH ALIGN=LEFT>E-mail:</TH>"
        "<TD><INPUT TYPE=text NAME=\"email\" SIZE=30 </TD></TR></TABLE><P>"
        "<INPUT TYPE=submit VALUE=\"Add\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\"></FORM>";
    mainMenuReference(con);
    return true;
}

bool addEngineerForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Enter new engineer</TITLE></HEAD>"
        BODY
        "<H2>Add engineer</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"addEngineer\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><TABLE>"
        "<TR><TH ALIGN=LEFT>Engineer name:</TH>"
        "<TD><INPUT TYPE=text NAME=\"name\" SIZE=30</TD></TR>"
        "<TR><TH ALIGN=LEFT>E-mail:</TH>"
        "<TD><INPUT TYPE=text NAME=\"email\" SIZE=30 </TD></TR></TABLE><P>"
        "<INPUT TYPE=submit VALUE=\"Add\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\"></FORM>";
    mainMenuReference(con);
    return true;
}

bool addSoftwareForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Enter new software product</TITLE></HEAD>"
        BODY
        "<H2>Add software product</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"addSoftware\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><TABLE>"
        "<TR><TH ALIGN=LEFT>Software name:</TH>"
        "<TD><INPUT TYPE=text NAME=\"software\" SIZE=40</TD></TR>"
        "<TR><TH ALIGN=LEFT>Version number:</TH>"
        "<TD><INPUT TYPE=text NAME=\"version\" SIZE=8</TD></TR>"
        "<TR><TH ALIGN=LEFT>Version label:</TH>"
        "<TD><INPUT TYPE=text NAME=\"label\" SIZE=20</TD></TR>"
        "<TR><TH ALIGN=LEFT>Version comment:</TH>"
        "<TD><INPUT TYPE=text NAME=\"comment\" SIZE=40</TD></TR>"
        "</TABLE><P><INPUT TYPE=submit VALUE=\"Add\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\"></FORM>";
    mainMenuReference(con);
    return true;
}

bool selectSoftwareForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Select software product</TITLE></HEAD>"
        BODY
        "<H2>Select software product</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"softwareForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><SELECT SIZE=15 NAME=\"software\">";
    if (products.select() > 0) { 
        print(con, products);
        con << TAG << 
           "</SELECT><BR><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}

bool removeSoftwareForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Remove software product</TITLE></HEAD>"
        BODY
        "<H2>Remove software product</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"removeSoftware\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><SELECT SIZE=15 NAME=\"software\">";
    if (products.select() != 0) { 
        print(con, products);
        con << TAG << "</SELECT><BR><INPUT TYPE=\"submit\" VALUE=\"Remove\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}

bool selectPersonForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Select a person</TITLE></HEAD>"
        BODY
        "<H2>Select a person</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"userForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><SELECT SIZE=20 NAME=\"name\">";
    if (persons.select() != 0) { 
        print(con, persons);
        con << TAG << "</SELECT><BR><INPUT TYPE=submit VALUE=\"Select\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}

bool removePersonForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Remove a person</TITLE></HEAD>"
        BODY
        "<H2>Remove a person</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"removePerson\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><SELECT SIZE=20 NAME=\"name\">";
    if (persons.select() != 0) { 
        print(con, persons);
        con << TAG << "</SELECT><BR><INPUT TYPE=submit VALUE=\"Remove\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}

bool selectBugForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Select a bug</TITLE></HEAD>"
        BODY
        "<H2>Select a bug</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"bugForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><SELECT SIZE=15 NAME=\"bug\">";
    if (bugs.select() != 0) { 
        print(con, bugs);
        con << TAG << 
           "</SELECT><BR><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}

bool removeBugForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Remove a bug</TITLE></HEAD>"
        BODY
        "<H2>Remove a bug</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"removeBug\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><SELECT SIZE=15 NAME=\"bug\">";
    if (bugs.select() != 0) { 
        print(con, bugs);
        con << TAG << "</SELECT><BR><INPUT TYPE=submit VALUE=\"Remove\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}

bool changePasswordForm(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>Change password</TITLE></HEAD>"
        BODY
        "<H2>Change password</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"changePassword\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><INPUT TYPE=hidden NAME=\"name\" VALUE=\"" << con.get("name") << 
        "\"><TABLE>"
        "<TR><TH ALIGN=LEFT>New password:</TH>"
        "<TD><INPUT TYPE=password NAME=\"password\" SIZE=20</TD></TR>"
        "<TR><TH ALIGN=LEFT>Re-type password:</TH>"
        "<TD><INPUT TYPE=password NAME=\"password2\" SIZE=20</TD></TR>"
        "</TABLE><P>"
        "<INPUT TYPE=submit VALUE=\"Change\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\">"
        "</FORM>";
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}
        
bool shutdown(WWWconnection& con)
{
    con << TAG << 
        HTML_HEAD "<TITLE>BUGDB message</TITLE></HEAD><BODY>"
        "<H1>BUGDB server is terminated</H1></BODY></HTML>";
    return false;
}

bool userForm(WWWconnection& con);
bool userGroupForm(WWWconnection& con);
bool softwareForm(WWWconnection& con);

bool addUser(WWWconnection& con)
{
    Person person;
    person.sName = key = con.get("name");   
    person.sEmailAddress = con.get("email");
    person.sPassword = "";
    person.status = Person::isUser;
    person.nReports = 0;
    if (persons.select(qPerson) != 0) { 
        error(con, "Person already exists");
        return true;
    }
    insert(person);
    return userForm(con);
}

bool addEngineer(WWWconnection& con)
{
    Person person;
    person.sName = key = con.get("name");   
    person.sEmailAddress = con.get("email");
    person.sPassword = "";
    person.status = Person::isEngineer;
    person.nReports = 0;
    if (persons.select(qPerson) != 0) { 
        error(con, "Person already exists");
        return true;
    }
    insert(person);
    return userForm(con);
}

bool removePerson(WWWconnection& con)
{
    key = con.get("name");
    if (key == NULL) { 
        error(con, "No person was selected");
        return true;
    }
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
    } else if (persons->nReports > 0 
               || persons->setReportedBugs.length() > 0) 
    {
        error(con, "It is not possible to delete person who is author "
              "of existed bug reports");
    } else {
        persons.remove();
        message(con, "Person is removed");
    }
    return true;
}

bool addSoftware(WWWconnection& con)
{
    Software software;
    Version  version;
    software.sName = key = con.get("software");
    if (products.select(qSoftware) != 0) { 
        error(con, "Software product already exists");
        return true;
    }
    char* versionStr = con.get("version");
    if (sscanf(versionStr, "%d.%d", &version.majorVersionNumber, 
               &version.minorVersionNumber) != 2) 
    { 
        error(con, "Bad version number (MAJOR.MINOR expected)");
        return true;
    }  
    version.sComment = con.get("comment");
    version.sLabel = con.get("label");
    version.released = dbDateTime::current();
    software.pVersions = insert(version);
    insert(software);
    con.addPair("action", "Select");
    return softwareForm(con);
}
                
bool removeSoftware(WWWconnection& con)
{
    key = con.get("software");
    if (products.select(qSoftware) == 0) { 
        error(con, "No such software product");
        return true;
    }
    if (products->setBugs.length() != 0) { 
        error(con, "Can not remove software with non-empty reported bug list");
        return true;
    }
    products.remove();
    message(con, "Software product is removed");
    return true;
}

bool removeBug(WWWconnection& con)
{
    char* bug = con.get("bug");
    if (bug == NULL) { 
        error(con, "No bug was selected");
    } else { 
        bugId = atoi(bug);
        if (bugs.select(qBug) == 0) { 
            error(con, "No such bug");
        } else { 
            if (bugs->pReportHistory != null ||
                bugs->pWorkArounds != null)
            {
                error(con, "Can not remove bug with existed reports");
                return true;
            }
            bugs.remove();
            message(con, "Bug is removed");
        }
    }
    return true;
}

bool changePassword(WWWconnection& con)
{
    char* password = con.get("password");
    char* password2 = con.get("password2");
    if (strcmp(password, password2) != 0) { 
        error(con, "Passwords are not equal");
    } else { 
        key = con.get("name");
        if (persons.select(qPerson) == 0) { 
            error(con, "No such person");
        } else { 
            persons->sPassword = password;
            persons.update();
            message(con, "Password changed");
        }
    }
    return true;
}

bool updatePerson(WWWconnection& con)
{
    char* name = con.get("name");
    key = name;
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
        return true;
    } else { 
        char* newName = con.get("newname");
        char* eMail = con.get("email");
        if (eMail != NULL) { 
            persons->sEmailAddress = eMail;
        }
        if (newName != NULL) { 
            persons->sName = newName;
            con.addPair("name", newName);
            if (strcmp(name, con.get("myself")) == 0) { 
                con.addPair("myself", newName);
            }
        }
        persons.update();
    }
    return userForm(con);
}

bool login(WWWconnection& con)
{
    char* name = con.get("name");
    key = con.get("name");
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
        return true;
    } 
    if (!persons->checkPassword(con.get("password"))) { 
        error(con, "Incorrect password");
        return true;
    } 
    con.addPair("myself", name);    
    return userForm(con);
}

bool bugQueryForm(WWWconnection& con)
{
    int i;
    con << TAG << 
        HTML_HEAD "<TITLE>Query to locate bug</TITLE></HEAD>"
        BODY
        "<H2>Bug query</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"bugQuery\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><TABLE>"
        "<TR><TH ALIGN=LEFT>Description substring:</TH>"
        "<TD><INPUT TYPE=text NAME=\"summary\" SIZE=30</TD></TR>"
        "<TR><TH ALIGN=LEFT>Category:</TH>"
        "<TD><SELECT NAME=\"category\" SIZE=1>"
        "<OPTION VALUE=0 SELECTED></OPTION>";
    for (i = 1; eCATEGORY_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION VALUE=" << i << ">"
            << eCATEGORY_STRING[i] << "</OPTION>";
    }
    con << TAG << "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Severity:</TH>"
        "<TD><SELECT NAME=\"severity\" SIZE=1>"
        "<OPTION VALUE=0 SELECTED></OPTION>";
    for (i = 1; eSEVERITY_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION VALUE=" << i << ">"
            << eSEVERITY_STRING[i] << "</OPTION>";
    }
    con << TAG << "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Fixing priority:</TH>"
        "<TD><SELECT NAME=\"priority\" SIZE=1>"
        "<OPTION VALUE=0 SELECTED></OPTION>";
    for (i = 1; eFIXING_PRIORITY_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION VALUE=" << i << ">"
            << eFIXING_PRIORITY_STRING[i] << "</OPTION>";
    }
    con << TAG << 
        "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Platform:</TH>"
        "<TD><INPUT TYPE=text NAME=\"platform\"</TD></TR>"
        "<TR><TH ALIGN=LEFT>OS</TH>"
        "<TD><INPUT TYPE=text NAME=\"os\"</TD></TR>"
        "<TR><TH ALIGN=LEFT>Software:</TH>"
        "<TD><INPUT TYPE=text NAME=\"software\"</TD></TR>"
        "<TR><TH ALIGN=LEFT>Assigned to:</TH>"
        "<TD><INPUT TYPE=text NAME=\"engineer\"</TD></TR>"
        "<TR><TH ALIGN=LEFT>Reported by:</TH>"
        "<TD><INPUT TYPE=text NAME=\"user\"</TD></TR>"
        "<TR><TH ALIGN=LEFT>Major version number:</TH>"
        "<TD>from <INPUT TYPE=text NAME=\"minmajor\" SIZE=4>"
        " to <INPUT TYPE=text NAME=\"maxmajor\" SIZE=4</TD></TR>"
        "<TR><TH ALIGN=LEFT>Minor version number:</TH>"
        "<TD>from <INPUT TYPE=text NAME=\"minminor\" SIZE=4</TD>"
        " to <INPUT TYPE=text NAME=\"maxminor\" SIZE=4</TD></TR></TABLE><P>"
        "<INPUT TYPE=submit VALUE=\"Search\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\">"
        "</FORM></BODY></HTML>";
    return true;
}


bool bugQuery(WWWconnection& con) 
{
    char* p;
    dbQuery query;
    query.reset();
    p = con.get("software");
    if (*p != '\0') { 
        query.add("pSoftware.sName like").add(p);
    }
    int4 category = atoi(con.get("category"));
    if (category != 0) { 
        query.And("eCategory=").add(category);
   }
    int4 severity = atoi(con.get("severity"));
    if (severity != 0) { 
        query.And("eSeverity=").add(severity);
    }
    int4 priority = atoi(con.get("priority"));
    if (priority != 0) { 
        query.And("eFixingPriority=").add(priority);
    }
    p = con.get("os");
    if (*p != '\0') { 
        query.And("sOperatingSystem like").add(p);
    }
    p = con.get("platform");
    if (*p != '\0') { 
        query.And("sHardwarePlatform like").add(p);
    }
    p = con.get("engineer");
    if (*p != '\0') { 
        query.And("pAssignedTo is not null and pAssignedTo.sName like").add(p);
    }
    p = con.get("user");
    if (*p != '\0') { 
        query.And("pReportedBy.sName like").add(p);
    }
    p = con.get("summary");
    if (*p != '\0') { 
        query.And("sOneLineSummary like").add(p);
    }
    p = con.get("minmajor");
    int minMajorVersionNumber = (*p == '\0') ? 0 : atoi(p);
    p = con.get("maxmajor");
    int maxMajorVersionNumber = (*p == '\0') ? INT_MAX : atoi(p);
    p = con.get("minminor");
    int minMinorVersionNumber = (*p == '\0') ? 0 : atoi(p);
    p = con.get("maxminor");
    int maxMinorVersionNumber = (*p == '\0') ? INT_MAX : atoi(p);
    if (minMajorVersionNumber != 0) { 
        if (maxMajorVersionNumber != INT_MAX) { 
            query.And("pVersion.majorVersionNumber between")
                .add(minMajorVersionNumber)
                .add("and").add(maxMajorVersionNumber);
        } else { 
            query.And("pVersion.majorVersionNumber>=")
                .add(minMajorVersionNumber);
        }
    } else if (maxMajorVersionNumber != INT_MAX) {      
        query.And("pVersion.majorVersionNumber<=").add(maxMajorVersionNumber);
    }
    if (minMinorVersionNumber != 0) { 
        if (maxMinorVersionNumber != INT_MAX) { 
            query.And("pVersion.minorVersionNumber between")
                .add(minMinorVersionNumber)
                .add("and").add(maxMinorVersionNumber);
        } else { 
            query.And("pVersion.minorVersionNumber>=")
                .add(minMinorVersionNumber);
        }
    } else if (maxMinorVersionNumber != INT_MAX) {      
        query.And("pVersion.minorVersionNumber<=").add(maxMinorVersionNumber);
    }
    con << TAG << 
        HTML_HEAD "<TITLE>List of selected bugs</TITLE></HEAD>"
        BODY
        "<H2>Selected bugs</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << 
        "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"bugForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself") << 
        "\"><SELECT NAME=\"bug\" SIZE=20>";
    if (bugs.select(query) != 0) { 
        print(con, bugs);
        con << TAG << 
           "</SELECT><BR><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}


bool userForm(WWWconnection& con)
{
    char* name = con.get("name");
    if (name == NULL) { 
        error(con, "No person was selected");
        return true;
    }
    char* myself = con.get("myself");
    key = myself;
    if (persons.select(qPerson) == 0) { 
        error(con, "Person authorization failed");
        return true;
    }
    int selfStatus = persons->status;
    key = name;
    if (persons.select(qPerson) == 0) { 
        error(con, "Person not found");
        return true;
    }
    if (persons->status == Person::isAdministrator) { 
        con << TAG << 
            HTML_HEAD "<TITLE>BUGDB Administrator</TITLE></HEAD>"
            BODY        
            "<H2>Administrator menu</H2><FONT SIZE=\"+1\">"
            "<UL><LI><A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress() 
            << "&page=addUserForm&myself=" << URL << myself 
            << "\">Add user</A>"
            "<LI><A HREF=\"" << con.getStub() << "?socket=" << con.getAddress()
            << "&page=addEngineerForm&myself=" << URL << myself 
            << "\">Add engineer</A>"
            "<LI><A HREF=\"" << con.getStub() << "?socket=" << con.getAddress()
            << "&page=selectPersonForm&myself=" << URL << myself 
            << "\">Select person"
            "</A>"
            "<LI><A HREF=\"" << con.getStub() << "?socket=" << con.getAddress()
            << "&page=removePersonForm&myself=" << URL << myself 
            << "\">Remove person</A></UL>"

            "<UL><LI><A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress()
            << "&page=addSoftwareForm&myself=" << URL << myself 
            << "\">Add software product</A>"
            "<LI><A HREF=\"" << con.getStub() << "?socket=" << con.getAddress()
            << "&page=selectSoftwareForm&myself=" << URL << myself 
            << "\">Select software product</A>"
            "<LI><A HREF=\"" << con.getStub() << "?socket=" << con.getAddress()
            << "&page=removeSoftwareForm&myself=" << URL << myself 
            << "\">Remove software product</A></UL>"

            "<UL><LI><A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress()
            << "&page=selectBugForm&myself=" << URL << myself 
            << "\">Select bug</A>"
            "<LI><A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress()
            << "&page=removeBugForm&myself=" << URL << myself 
            << "\">Remove bug</A></UL>";
        if (selfStatus == Person::isAdministrator) { 
            con << TAG << 
                "<UL><LI><A HREF=\"" << con.getStub() << "?socket=" 
                << con.getAddress() << "&page=changePasswordForm"
                "&myself=administrator&name=" << URL << myself 
                << "\">Change password</A>"
                "<LI><A HREF=\"" << con.getStub() << "?socket=" 
                << con.getAddress()
                << "&page=shutdown\">Shutdown server</A></UL>";
        }
        con << TAG << "</FONT></BODY></HTML>";
        return true;
    }
    con <<
        HTML_HEAD "<TITLE>" << name << "</TITLE></HEAD>"
        BODY
        "<H2>" << name << "</H2><FONT SIZE=\"+1\">"
        "<UL><LI><A HREF=\"" << con.getStub() << "?socket="
        << con.getAddress() << "&page=createBugReportForm&myself=" 
        << URL << myself << "\">Create bug report</A>";
    if (persons->sEmailAddress[0] != '\0') { 
        con << TAG << 
            "<LI><A HREF=\"mailto:" << persons->sEmailAddress 
            << "\">Send e-mail</A>"
            "<LI><A HREF=\"" << con.getStub() << "?socket=" << con.getAddress()
            << "&page=bugQueryForm&myself=" << URL << myself 
            << "\">Find a bug</A>";
    }
    if (strcmp(myself, name) == 0 || selfStatus == Person::isAdministrator) { 
        con << TAG << "<LI><A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress() << "&page=changePasswordForm&myself=" 
            << URL << myself << "&name=" << URL << name << 
            "\">Change password</A>";
    }
    con << TAG << 
        "</UL></FONT><P><TABLE><TR><TH ALIGN=LEFT>Person name:</TH>"
        "<TD><FORM METHOD=POST ACTION=\""
        << con.getStub() << "\"><INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\""
        "updatePerson\"><INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself
        << "\"><INPUT TYPE=hidden NAME=\"name\" VALUE=\"" << name << "\">"
        "<INPUT TYPE=text NAME=\"newname\" SIZE=30 VALUE=\"" 
        << name << "\"><INPUT TYPE=submit VALUE=\"Change\"></FORM></TD></TR>"
        "<TR><TH ALIGN=LEFT>E-Mail:</TH>"
        "<TD><FORM METHOD=POST ACTION=\""
        << con.getStub() << "\"><INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\""
        "updatePerson\"><INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself
        << "\"><INPUT TYPE=hidden NAME=\"name\" VALUE=\"" << name << "\">"
        "<INPUT TYPE=text NAME=\"email\" SIZE=30 VALUE=\"" 
        << persons->sEmailAddress << "\">"
        "<INPUT TYPE=submit VALUE=\"Change\"></FORM></TD></TR>";
    if (persons->status != Person::isUser) { 
        con << TAG << "<TR><TH ALIGN=LEFT>Projects:</TH>"
            "<TD><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
            << con.getAddress() << "\"><INPUT TYPE=hidden NAME=\"page\" "
            "VALUE=\"softwareForm\">"
            "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
            "<INPUT TYPE=hidden NAME=\"name\" VALUE=\"" << name << "\">"
            "<SELECT NAME=\"software\" SIZE=1>";
        if (persons->setProjects.length() != 0) { 
            print(con, persons->setProjects);
            con << TAG << 
                "</SELECT><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">"
                "&nbsp;<INPUT TYPE=submit NAME=\"action\" VALUE=\"Detach\">";
        } else { 
            con << TAG << EMPTY_LIST;
        }
        if (products.select() != 0) { 
            con << TAG << 
                "</FORM></TD></TR>"
                "<TR><TH ALIGN=LEFT>Attach to project:</TH>"
                "<TD><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
                "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
                << con.getAddress() << "\"><INPUT TYPE=hidden NAME=\"page\" "
                "VALUE=\"attachToProject\">"
                "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself <<"\">"
                "<INPUT TYPE=hidden NAME=\"name\" VALUE=\"" << name << "\">"
                "<SELECT NAME=\"software\" SIZE=1>";
            print(con, products); 
            con << TAG << "</SELECT><INPUT TYPE=submit VALUE=\"Attach\">";
        } 
        con << TAG << 
            "</FORM></TD></TR>"
            "<TR><TH ALIGN=LEFT>Find a person:</TH>"
            "<TD><FORM METHOD=POST ACTION=\"" 
            << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
            << con.getAddress() << "\"><INPUT TYPE=hidden NAME=\"page\" "
            "VALUE=\"userForm\">"
            "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
            "<INPUT TYPE=text NAME=\"name\" SIZE=30>"
            "<INPUT TYPE=submit VALUE=\"Find\"></FORM></TD></TR>";
    } 
    con << TAG << "<TR><TH ALIGN=LEFT>Used software:</TH>"
        "<TD><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden NAME=\"page\" "
        "VALUE=\"softwareForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
        "<INPUT TYPE=hidden NAME=\"name\" VALUE=\"" << name << "\">"
        "<SELECT NAME=\"software\" SIZE=1>";
    if (persons->setUsedSoftware.length() != 0) { 
        print(con, persons->setUsedSoftware);
        con << TAG << 
            "</SELECT><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">"
            "&nbsp;<INPUT TYPE=submit NAME=\"action\" VALUE=\"Unregister\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    if (products.select() != 0) { 
        con << TAG << 
            "</FORM></TD></TR>"
            "<TR><TH ALIGN=LEFT>Register as software user:</TH>"
            "<TD><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
            << con.getAddress() << "\"><INPUT TYPE=hidden NAME=\"page\" "
            "VALUE=\"registerSoftware\">"
            "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
            "<INPUT TYPE=hidden NAME=\"name\" VALUE=\"" << name << "\">"
            "<SELECT NAME=\"software\" SIZE=1>";
        print(con, products);
        con << TAG << "</SELECT><INPUT TYPE=submit VALUE=\"Register\">";
    }
    con << TAG << "</FORM></TD></TR></TABLE><P>"
        "<B>Reported bugs:</B><BR>"     
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden NAME=\"page\" "
        "VALUE=\"bugForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
        "<INPUT TYPE=hidden NAME=\"name\" VALUE=\"" << name << "\">"
        "<SELECT NAME=\"bug\" SIZE=5>";
    if (persons->setReportedBugs.length() != 0) {
        print(con, persons->setReportedBugs);
        con << TAG << 
           "</SELECT><BR><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }   
    con << TAG << "</FORM><P>";
    if (persons->status != Person::isUser) { 
        con << TAG << 
            "<P><B>Assigned bugs:</B><BR>"      
            "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
            << con.getAddress() << "\"><INPUT TYPE=hidden "
            "NAME=\"page\" VALUE=\"bugForm\">"
            "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
            "<INPUT TYPE=hidden NAME=\"name\" VALUE=\"" << name << "\">"
            "<SELECT NAME=\"bug\" SIZE=5>";
        if (persons->setAssignedBugs.length() != 0) {
            print(con, persons->setAssignedBugs);
            con << TAG << 
                "</SELECT><BR>"
                "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">&nbsp;"
                "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Deassign\">";
        } else { 
            con << TAG << EMPTY_LIST;
        }
        con << TAG << "</FORM>";
    }
    if (strcmp(name, myself) == 0) { 
        con << TAG << "</BODY></HTML>";
    } else { 
        mainMenuReference(con);
    }
    return true;
}


bool createBugReportForm(WWWconnection& con)
{
    int i;
    sequencer.select();
    con << TAG << 
        HTML_HEAD "<TITLE>Bug</TITLE></HEAD>"
        BODY
        "<H2>Bug</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden "
        "NAME=\"page\" VALUE=\"createBugReport\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\""<<con.get("myself")<<"\">"
        "<INPUT TYPE=hidden NAME=\"bug\" VALUE=" << ++sequencer->nBugs << ">"
        "<TABLE><TH ALIGN=LEFT>Summary:</TH>"
        "<TD><INPUT TYPE=text NAME=\"summary\" SIZE=40></TD></TR>"
        "<TR><TH ALIGN=LEFT>Category:</TH>"
        "<TD><SELECT NAME=\"category\" SIZE=1>";
    for (i = 1; eCATEGORY_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION SELECTED VALUE=" << i << ">"
            << eCATEGORY_STRING[i] << "</OPTION>";
    }
    con << TAG << "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Severity:</TH>"
        "<TD><SELECT NAME=\"severity\" SIZE=1>";
    for (i = 1; eSEVERITY_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION SELECTED VALUE=" << i << ">"
            << eSEVERITY_STRING[i] << "</OPTION>";
    }
    con << TAG << 
        "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Priority:</TH>"
        "<TD><SELECT NAME=\"priority\" SIZE=1>";
    for (i = 1; eFIXING_PRIORITY_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION SELECTED VALUE=" << i << ">"
            << eFIXING_PRIORITY_STRING[i] << "</OPTION>";
    }
    con << TAG << 
        "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Software:</TH>"
        "<TD><SELECT NAME=\"software\" SIZE=1>";
    if (products.select() != 0) { 
        print(con, products);
    } 
    con << TAG <<
            "</SELECT></TD></TR>"
            "<TR><TH ALIGN=LEFT>Version:</TH>"
            "<TD><INPUT TYPE=text NAME=\"version\"></TD></TR>"
            "<TR><TH ALIGN=LEFT>Platform:</TH>"
            "<TD><INPUT TYPE=text NAME=\"platform\"</TD></TR>"
            "<TR><TH ALIGN=LEFT>OS:</TH>"
            "<TD><INPUT TYPE=text NAME=\"os\"></TD></TR></TABLE><P>"
            "<INPUT TYPE=submit VALUE=\"Submit\">&nbsp;"
            "<INPUT TYPE=reset></FORM>";
    mainMenuReference(con);
    sequencer.update();
    return true;
}

bool bugForm(WWWconnection& con);

bool createBugReport(WWWconnection& con)
{
    key = con.get("myself");
    if (persons.select(qPerson) == 0) { 
        error(con, "Author unknown");
        return true;
    }
    key = con.get("software");
    if (products.select(qSoftware) == 0) { 
        error(con, "No such software product");
        return true;
    }
    if (sscanf(con.get("version"), "%d.%d", &majorVersion, &minorVersion) != 2)
    {
        error(con, "Bad version format");
        return true;
    }
    initialVersion = products->pVersions;
    if (versions.select(qVersion) == 0) { 
        error(con, "No such software product version");
        return true;
    }
    
    Bug bug;
    bug.bugId = atoi(con.get("bug"));
    bug.sOneLineSummary = con.get("summary");
    bug.eCategory = atoi(con.get("category"));
    bug.eFixingPriority = atoi(con.get("priority"));
    bug.eSeverity = atoi(con.get("severity"));
    bug.sOperatingSystem = con.get("os");
    bug.sHardwarePlatform = con.get("platform");
    bug.pReportedBy = persons.currentId();
    bug.pAssignedTo = null;
    bug.pSoftware = products.currentId();
    bug.pVersion = versions.currentId();
    bug.nReports = 0;
    insert(bug);
    con.addPair("action", "Select");
    return bugForm(con);
}

bool bugForm(WWWconnection& con)
{
    int i;
    char* bugStr = con.get("bug"); 
    bugId = atoi(bugStr);
    if (bugs.select(qBug) == 0) { 
        error(con, "No bug was selected");
        return true;
    }
    char* myself = con.get("myself");
    if (strcmp(con.get("action"), "Remove") == 0) { 
        dbReference<Bug> pBug = bugs.currentId();
        bugId = atoi(con.get("relatedbug"));
        if (bugs.select(qBug) == 0) { 
            error(con, "No such bug");
            return true;
        }
        int i = rindex(bugs->setSimilarBugs, pBug);
        if (i < 0) { 
            error(con, "No such related bug");
            return true;
        }
        bugs->setSimilarBugs.remove(i);
        bugs.update();
        return bugForm(con);
    }
    key = myself;
    if (persons.select(qPerson) == 0) { 
        error(con, "No such user");
        return true;
    }
    if (strcmp(con.get("action"), "Deassign") == 0) { 
        int i = rindex(persons->setAssignedBugs, bugs.currentId());
        if (i < 0) { 
            error(con, "Bug was not assigned");
            return true;
        }
        persons->setAssignedBugs.remove(i);
        persons.update();
        con.addPair("name", myself);
        return userForm(con);
    }

    int personStatus = persons->status;
    products.at(bugs->pSoftware);
    versions.at(bugs->pVersion);
    con << TAG <<
        HTML_HEAD "<TITLE>Bug in " << products->sName << " v. " << 
        versions->getVersionString() << "</TITLE></HEAD>"
        BODY
        "<H2>Bug in " << products->sName << " v. "
        << versions->getVersionString() << "</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden "
        "NAME=\"page\" VALUE=\"updateBug\">"
        "<INPUT TYPE=hidden NAME=\"bug\" VALUE=" << bugStr << ">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
        "<TABLE><TH ALIGN=LEFT>Summary:</TH>"
        "<TD><INPUT TYPE=text NAME=\"summary\" SIZE=40 VALUE=\"" 
        << bugs->sOneLineSummary << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Category:</TH>"
        "<TD><SELECT NAME=\"category\" SIZE=1>"
        "<OPTION SELECTED VALUE=" << bugs->eCategory << ">"
        << eCATEGORY_STRING[bugs->eCategory] << "</OPTION>";
    for (i = 1; eCATEGORY_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION VALUE=" << i << ">" 
            << eCATEGORY_STRING[i] << "</OPTION>";
    }
    con << TAG << 
        "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Severity:</TH>"
        "<TD><SELECT NAME=\"severity\" SIZE=1>"
        "<OPTION SELECTED VALUE=" << bugs->eSeverity << ">"
        << eSEVERITY_STRING[bugs->eSeverity] << "</OPTION>";
    for (i = 1; eSEVERITY_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION  VALUE=" << i << ">"
            << eSEVERITY_STRING[i] << "</OPTION>";
    }
    con << TAG << 
        "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Priority:</TH>"
        "<TD><SELECT NAME=\"priority\" SIZE=1>"
        "<OPTION SELECTED VALUE=" << bugs->eFixingPriority << ">"
        << eFIXING_PRIORITY_STRING[bugs->eFixingPriority] << "</OPTION>";
    for (i = 1; eFIXING_PRIORITY_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION VALUE=" << i << ">"
            << eFIXING_PRIORITY_STRING[i] << "</OPTION>";
    }
    con << TAG << 
        "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Platform:</TH>"
        "<TD><INPUT TYPE=text NAME=\"platform\" VALUE=\""
        << bugs->sHardwarePlatform << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>OS:</TH>"
        "<TD><INPUT TYPE=text NAME=\"os\"VALUE=\""
        << bugs->sOperatingSystem << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Assigned to:</TH>"
        "<TD><SELECT SIZE=1 NAME=\"name\">";
    if (bugs->pAssignedTo != null) { 
        persons.at(bugs->pAssignedTo);
        con << TAG << "<OPTION SELECTED VALUE=\"" << persons->sName
            << "\">" <<  persons->sName << "</OPTION>";
    } else { 
        con << TAG << "<OPTION SELECTED VALUE=\"\"></OPTION>";
    }
    print(con, products->setEngineers); 
    con << TAG << "</SELECT></TD></TR>"
        "<TR><TH ALIGN=LEFT>Similar with:</TH>"
        "<TD><SELECT NAME=\"similar\" SIZE=1>"
        "<OPTION SELECTED VALUE=\"\"></OPTION>";
    allBugs.select();
    print(con, allBugs);
    con << TAG << "</SELECT></TD></TR></TABLE><BR>";

    if (personStatus != Person::isUser) { 
        con << TAG << 
            "<INPUT TYPE=submit NAME=\"action\" VALUE=\"Update\">&nbsp;"
            "<INPUT TYPE=reset VALUE=\"Reset\">";
    }
    con << TAG << "</FORM><P><FORM METHOD=POST ACTION=\"" << con.getStub()
        << "\"><INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=\"page\" VALUE=\"updateReportForm\">"
        "<INPUT TYPE=hidden NAME=\"bug\" VALUE=" << bugStr << ">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
        "<B>Report history:</B><BR><SELECT NAME=\"report\" SIZE=5>";
    firstReport = bugs->pReportHistory;
    if (reports.select(qAllReports) != 0) { 
        print(con, reports);
        con << TAG << 
            "</SELECT><BR><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">"
            "&nbsp;<INPUT TYPE=submit NAME=\"action\" VALUE=\"Add\">";
        if (personStatus == Person::isAdministrator) { 
            con << TAG << 
                "&nbsp;<INPUT TYPE=submit NAME=\"action\" VALUE=\"Remove\">";
        }
    } else { 
        con << TAG << EMPTY_LIST 
            "<BR><INPUT TYPE=submit  NAME=\"action\" VALUE=\"Add\">";
    }
    con << TAG << "</FORM><P>";

    con << TAG << 
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden "
        "NAME=\"page\" VALUE=\"updateWorkAroundForm\">"
        "<INPUT TYPE=hidden NAME=\"bug\" VALUE=" << bugStr << ">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
        "<B>Work arounds:</B><BR><SELECT NAME=\"workaround\" SIZE=5>";
    firstReport = bugs->pWorkArounds;
    if (reports.select(qAllReports) != 0) { 
        print(con, reports);
        con << TAG << 
            "</SELECT><BR><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">"
            "&nbsp;<INPUT TYPE=submit NAME=\"action\" VALUE=\"Add\">";
        if (personStatus == Person::isAdministrator) { 
            con << TAG << 
                "&nbsp;<INPUT TYPE=submit NAME=\"action\" VALUE=\"Remove\">";
        }
    } else { 
        con << TAG << EMPTY_LIST 
            "<BR><INPUT TYPE=submit  NAME=\"action\" VALUE=\"Add\">";
    }
    con << TAG << "</FORM><P>";    

    if (bugs->setSimilarBugs.length() != 0) { 
        con << TAG << 
            "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
            << con.getAddress() << "\"><INPUT TYPE=hidden "
            "NAME=\"page\" VALUE=\"bugForm\">"
            "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
            "<INPUT TYPE=hidden NAME=\"relatedbug\" VALUE=" << bugStr << ">"
            "<B>Similar bugs:</B><BR><SELECT NAME=\"bug\" SIZE=1>";
        print(con, bugs->setSimilarBugs);
        con << TAG << 
            "</SELECT><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">";
        if (personStatus == Person::isAdministrator) { 
            con << TAG << 
                "&nbsp;<INPUT TYPE=submit NAME=\"action\" VALUE=\"Remove\">";
        }
        con << TAG << "</FORM><P>";
    }
    con << TAG << 
        "</FORM><P>"
        "<FONT SIZE=\"+1\"><UL>";
    if (personStatus == Person::isUser) { 
        if (bugs->pAssignedTo != null) { 
            persons.at(bugs->pAssignedTo);
            con << TAG << "<LI>Assigned to <A HREF=\"mailto:"  
                << persons->sEmailAddress << "\">" 
                << persons->sName << "</A>";
        }
        persons.at(bugs->pReportedBy);
        con << TAG << "<LI>Reported by <A HREF=\"mailto:"
            << persons->sEmailAddress << "\">" 
            << persons->sName << "</A></OL></FONT>";
    } else { 
        if (bugs->pAssignedTo != null) { 
            persons.at(bugs->pAssignedTo);
            con << TAG << "<LI>Assigned to <A HREF=\"" << con.getStub() 
                << "?socket=" << con.getAddress() 
                << "&page=userForm&myself=" << URL << myself 
                << "&name=" << URL << persons->sName << "\">" 
                << persons->sName << "</A>";
        }
        persons.at(bugs->pReportedBy);
        con << TAG 
            << "<LI>Reported by <A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress() 
            << "&page=userForm&myself=" << URL << myself 
            << "&name=" << URL << persons->sName << "\">" 
            << persons->sName << "</A></OL></FONT>";
    }
    mainMenuReference(con);
    return true;
}

bool updateBug(WWWconnection& con)
{
    char* bugStr = con.get("bug");
    bugId = atoi(bugStr);
    if (bugs.select(qBug) == 0) { 
        error(con, "No such bug");
        return true;
    } 
    char* similar = con.get("similar");
    if (*similar != '\0') { 
        int id = atoi(similar);
        if (id != bugId) { 
            bugId = id;
            if (allBugs.select(qBug) != 0) { 
                if (rindex(bugs->setSimilarBugs, allBugs.currentId()) < 0) { 
                    bugs->setSimilarBugs.append(allBugs.currentId());
                }
            }
        }
    }
    key = con.get("name");
    if (*key != '\0') { 
        if (persons.select(qPerson) == 0 ||
            persons->status == Person::isUser) 
        { 
            error(con, "No such engineer");
            return true;
        }
        bugs->pAssignedTo = persons.currentId();
    }
    bugs.update();
    return bugForm(con);
}


bool addReportForm(WWWconnection& con)
{
    char* bugStr = con.get("bug");
    bugId = atoi(bugStr);
    if (bugs.select(qBug) == 0) { 
        error(con, "No such bug");
        return true;
    }
    con << TAG << 
        HTML_HEAD "<TITLE>Bug report</TITLE></HEAD>"
        BODY
        "<H2>Bug report</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden "
        "NAME=\"page\" VALUE=\"addReport\">"
        "<INPUT TYPE=hidden NAME=\"bug\" VALUE=" << bugStr << ">"
        "<INPUT TYPE=hidden NAME=\"index\" VALUE=" << ++bugs->nReports
        << "><INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself")
        << "\"><B>Status: &nbsp;</B><SELECT SIZE=1 NAME=\"status\">";
    for (int i = 1; eSTATUS_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION VALUE=" << i << ">" << eSTATUS_STRING[i] 
            << "</OPTION>";
    }
    con << TAG << 
        "</SELECT><P>"
        "<B>Bug description:</B><P>"
        "<TEXTAREA COLS=40 ROWS=5 NAME=\"description\"></TEXTAREA><P>"
        "<INPUT TYPE=submit VALUE=\"Add\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\"></FORM>";
    bugs.update();
    mainMenuReference(con);
    return true;
}

bool addReport(WWWconnection& con)
{
    bugId = atoi(con.get("bug"));
    if (bugs.select(qBug) == 0) { 
        error(con, "No such bug");
        return true;
    }
    key = con.get("myself");
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
        return true;
    }
    reportId = atoi(con.get("index"));
    firstReport = bugs->pReportHistory;
    if (reports.select(qReport) == 0) { 
        Report report;
        report.pAuthor = persons.currentId();
        persons->nReports += 1;
        report.sDescription = con.get("description");
        report.index = reportId;
        report.pNext = bugs->pReportHistory;
        report.status = atoi(con.get("status"));
        report.creationDate = dbDateTime::current();
        bugs->pReportHistory = insert(report);
        persons.update();
        bugs.update();
    }
    con.addPair("action", "Select");
    return bugForm(con);
}

bool addWorkAroundForm(WWWconnection& con)
{
    char* bugStr = con.get("bug");
    bugId = atoi(bugStr);
    if (bugs.select(qBug) == 0) { 
        error(con, "No such bug");
        return true;
    }
    con << TAG << 
        HTML_HEAD "<TITLE>Work around</TITLE></HEAD>"
        BODY
        "<H2>Work around</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden "
        "NAME=\"page\" VALUE=\"addWorkAround\">"
        "<INPUT TYPE=hidden NAME=\"bug\" VALUE=" << bugStr << ">"
        "<INPUT TYPE=hidden NAME=\"index\" VALUE=" << ++bugs->nReports
        << "><INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << con.get("myself")
        << "\"><B>Status: &nbsp;</B><SELECT SIZE=1 NAME=\"status\">";
    for (int i = 1; eSTATUS_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION VALUE=" << i << ">" << eSTATUS_STRING[i] 
            << "</OPTION>";
    }
    con << TAG << 
        "</SELECT><P>"
        "<B>Description:</B><P>"
        "<TEXTAREA COLS=40 ROWS=5 NAME=\"description\"></TEXTAREA><P>"
        "<INPUT TYPE=submit VALUE=\"Add\">&nbsp;"
        "<INPUT TYPE=reset VALUE=\"Reset\"></FORM>";
    bugs.update();
    mainMenuReference(con);
    return true;
}

bool addWorkAround(WWWconnection& con)
{
    bugId = atoi(con.get("bug"));
    if (bugs.select(qBug) == 0) { 
        error(con, "No such bug");
        return true;
    }
    key = con.get("myself");
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
        return true;
    }
    reportId = atoi(con.get("index"));
    firstReport = bugs->pWorkArounds;
    if (reports.select(qReport) == 0) { 
        Report report;
        report.pAuthor = persons.currentId();
        persons->nReports += 1;
        report.sDescription = con.get("description");
        report.index = reportId;
        report.pNext = bugs->pWorkArounds;
        report.status = atoi(con.get("status"));
        report.creationDate = dbDateTime::current();
        bugs->pWorkArounds = insert(report);
        persons.update();
        bugs.update();
    }
    con.addPair("action", "Select");
    return bugForm(con);
}

bool updateReportForm(WWWconnection& con)
{
    if (strcmp(con.get("action"), "Add") == 0) { 
        return addReportForm(con);
    }
    char* bugStr = con.get("bug");
    bugId = atoi(bugStr);
    if (bugs.select(qBug) == 0) { 
        error(con, "No such bug");
        return true;
    }
    char* report = con.get("report");
    if (report == NULL) { 
        error(con, "No report was selected");
        return true;
    }
    int index = atoi(report);
    dbReference<Report> prev, curr = null, next = bugs->pReportHistory;
    do  { 
        prev = curr;
        if (next == null) { 
            error(con, "No such report");
            return true;
        } 
        reports.at(next);
        curr = next;
        next = reports->pNext;
    } while (reports->index != index);

    if (strcmp(con.get("action"), "Remove") == 0) { 
        reports.remove();
        bugs->nReports -= 1;
        if (prev == null) { 
            bugs->pReportHistory = next;
        } else { 
            reports.at(prev);
            reports->pNext = next;
            reports.update();
        }
        bugs.update();
        con.addPair("action", "Select");
        return bugForm(con);
    }
    char date[64];
    reports->creationDate.asString(date, sizeof date);
    char* myself = con.get("myself");
    key = myself;
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
        return true;
    }
    int personStatus = persons->status;
    persons.at(reports->pAuthor);
    con << TAG << 
        HTML_HEAD "<TITLE>Bug report from " << date << "</TITLE></HEAD>"
        BODY
        "<H2>Bug report from " << date << "</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden "
        "NAME=\"page\" VALUE=\"updateReport\">"
        "<INPUT TYPE=hidden NAME=\"bug\" VALUE=" << bugStr << ">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself <<
        "\"><INPUT TYPE=hidden NAME=\"report\" VALUE=" << index << ">"
        "<B>Created by ";
    if (personStatus == Person::isUser) { 
        con << TAG << "<A HREF=\"mailto:"  
            << persons->sEmailAddress << "\">" 
            << persons->sName << "</A>";
    } else { 
        con << TAG << 
            "<A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress() 
            << "&page=userForm&myself=" << URL << myself 
            << "&name=" << URL << persons->sName << "\">" 
            << persons->sName << "</A>";
    }
    con << TAG << "<P>Status: </B><SELECT SIZE=1 NAME=\"status\">"
        "<OPTION SELECTED VALUE=" << reports->status << ">" 
        << eSTATUS_STRING[reports->status] << "</OPTION>";
    for (int i = 1; eSTATUS_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION VALUE=" << i << ">" << eSTATUS_STRING[i] 
            << "</OPTION>";
    }
    con << TAG <<
        "</SELECT><P>"
        "<B>Bug description:</B><BR>"
        "<TEXTAREA COLS=40 ROWS=5 NAME=\"description\">"
        << reports->sDescription << "</TEXTAREA><P>";
    if (personStatus != Person::isUser) { 
        con << TAG << 
            "<INPUT TYPE=submit VALUE=\"Update\">&nbsp;"
            "<INPUT TYPE=reset VALUE=\"Reset\">";
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}

bool updateWorkAroundForm(WWWconnection& con)
{
    if (strcmp(con.get("action"), "Add") == 0) { 
        return addWorkAroundForm(con);
    }
    char* bugStr = con.get("bug");
    bugId = atoi(bugStr);
    if (bugs.select(qBug) == 0) { 
        error(con, "No such bug");
        return true;
    }
    char* workaround = con.get("workaround");
    int index = atoi(workaround);
    dbReference<Report> prev, curr = null, next = bugs->pWorkArounds;
    do  { 
        prev = curr;
        if (next == null) { 
            error(con, "No such report");
            return true;
        } 
        reports.at(next);
        curr = next;
        next = reports->pNext;
    } while (reports->index != index);

    if (strcmp(con.get("action"), "Remove") == 0) { 
        reports.remove();
        bugs->nReports -= 1;
        if (prev == null) { 
            bugs->pWorkArounds = next;
        } else { 
            reports.at(prev);
            reports->pNext = next;
            reports.update();
        }
        bugs.update();
        con.addPair("action", "Select");
        return bugForm(con);
    }
    char date[64];
    reports->creationDate.asString(date, sizeof date);
    char* myself = con.get("myself");
    key = myself;
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
        return true;
    }
    int personStatus = persons->status;
    persons.at(reports->pAuthor);
    con << TAG << 
        HTML_HEAD "<TITLE>Work around " << date << "</TITLE></HEAD>"
        BODY
        "<H2>Work around " << date << "</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden "
        "NAME=\"page\" VALUE=\"updateWorkAround\">"
        "<INPUT TYPE=hidden NAME=\"bug\" VALUE=" << bugStr << ">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself <<
        "\"><INPUT TYPE=hidden NAME=\"workaround\" VALUE=" << index <<
        "><B>Created by ";
    if (personStatus == Person::isUser) { 
        con << TAG << "<A HREF=\"mailto:"  
            << persons->sEmailAddress << "\">" 
            << persons->sName << "</A>";
    } else { 
        con << TAG << 
            "<A HREF=\"" << con.getStub() << "?socket=" 
            << con.getAddress() 
            << "&page=userForm&myself=" << URL << myself 
            << "&name=" << URL << persons->sName << "\">" 
            << persons->sName << "</A>";
    }
    con << TAG << "<P>Status: </B><SELECT SIZE=1 NAME=\"status\">"
        "<OPTION SELECTED VALUE=" << reports->status << ">" 
        << eSTATUS_STRING[reports->status] << "</OPTION>";
    for (int i = 1; eSTATUS_STRING[i] != NULL; i++) { 
        con << TAG << "<OPTION VALUE=" << i << ">" << eSTATUS_STRING[i] 
            << "</OPTION>";
    }
    con << TAG << 
        "</SELECT><P>"
        "<B>Bug description:</B><BR>"
        "<TEXTAREA COLS=40 ROWS=5 NAME=\"description\">"
        << reports->sDescription << "</TEXTAREA><P>";
    if (personStatus != Person::isUser) { 
        con << TAG << 
            "<INPUT TYPE=submit VALUE=\"Update\">&nbsp;"
            "<INPUT TYPE=reset VALUE=\"Reset\">";
    }
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}

bool updateReport(WWWconnection& con)
{
    bugId = atoi(con.get("bug"));
    if (bugs.select(qBug) == 0) { 
        error(con, "No such bug");
        return true;
    }
    reportId = atoi(con.get("report"));
    firstReport = bugs->pReportHistory;
    if (reports.select(qReport) == 0) {  
        error(con, "No report was selected");
        return true;
    }
    reports->sDescription = con.get("description");
    reports->status = atoi(con.get("status"));
    reports.update();
    con.addPair("action", "Select");
    return bugForm(con);
}

bool updateWorkAround(WWWconnection& con)
{
    bugId = atoi(con.get("bug"));
    if (bugs.select(qBug) == 0) { 
        error(con, "No such bug");
        return true;
    }
    reportId = atoi(con.get("workaround"));
    firstReport = bugs->pWorkArounds;
    if (reports.select(qReport) == 0) {  
        error(con, "No report was selected");
        return true;
    }
    reports->sDescription = con.get("description");
    reports->status = atoi(con.get("status"));
    reports.update();
    con.addPair("action", "Select");
    return bugForm(con);
}


bool attachToProject(WWWconnection& con)
{
    key = con.get("name");
    if (persons.select(qPerson) == 0 || persons->status == Person::isUser) { 
        error(con, "No such engineer");
    } else { 
        key = con.get("software");
        if (products.select(qSoftware) == 0) { 
            error(con, "No such software product");
        } else { 
            if (rindex(products->setEngineers, persons.currentId()) >= 0) { 
                error(con, "Engineer already attached to the project");
            } else { 
                products->setEngineers.append(persons.currentId());
                products.update();
                return userForm(con);
            }
        }
    }
    return true;
}


bool registerSoftware(WWWconnection& con)
{
    key = con.get("name");
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
    } else { 
        key = con.get("software");
        if (products.select(qSoftware) == 0) { 
            error(con, "No such software product");
        } else { 
            if (rindex(products->setUsers, persons.currentId()) >= 0) { 
                error(con, "User already registered this software");
            } else { 
                products->setUsers.append(persons.currentId());
                products.update();
                return userForm(con);
            }
        }
    }
    return true;
}


bool softwareForm(WWWconnection& con)
{
    char* software = con.get("software");
    if (software == NULL) { 
        error(con, "No software product was selected");
        return true;
    }
    key = software;
    if (products.select(qSoftware) == 0) { 
        error(con, "No such software product");
        return true;
    }
    if (strcmp(con.get("action"), "Detach") == 0) { 
        key = con.get("name");
        if (persons.select(qPerson) == 0) { 
            error(con, "No such person");
            return true;
        }
        int i = rindex(persons->setProjects, products.currentId());
        if (i < 0) {
            error(con, "Person was not attached to the project");
            return true;
        }
        persons->setProjects.remove(i);
        persons.update();
        return userForm(con);
    }
    if (strcmp(con.get("action"), "Unregister") == 0) { 
        key = con.get("name");
        if (persons.select(qPerson) == 0) { 
            error(con, "No such person");
            return true;
        }
        int i = rindex(persons->setUsedSoftware, products.currentId());
        if (i < 0) {
            error(con, "Person was not registered");
            return true;
        }
        persons->setProjects.remove(i);
        persons.update();
        return userForm(con);
    }
    char* myself = con.get("myself");
    key = myself;
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
        return true;
    }
    int personStatus = persons->status;
    con << TAG << 
        HTML_HEAD "<TITLE>" << software << "</TITLE></HEAD>"
        BODY
        "<H2>" << software << "</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden "
        "NAME=\"page\" VALUE=\"updateSoftware\">"
        "<INPUT TYPE=hidden NAME=\"software\" VALUE=\"" << software << "\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
        "<TABLE><TR><TH ALIGN=LEFT>Product name:</TH>"
        "<TD><INPUT TYPE=text NAME=\"newname\" VALUE=\"" << software << "\">"
        "</TD></TR>";
    if (products->pVersions != null) { 
        versions.at(products->pVersions);
        con << TAG << 
            "<TR><TH ALIGN=LEFT>Current version:</TH>"
            "<TD><INPUT TYPE=text NAME=\"version\" SIZE=8 VALUE=\"" 
            << versions->getVersionString() << "\"></TD></TR>"
            "<TR><TH ALIGN=LEFT>Current version label:</TH>"
            "<TD><INPUT TYPE=text NAME=\"label\" SIZE=20 VALUE=\""
            << versions->sLabel << "\"></TD></TR>"
            "<TR><TH ALIGN=LEFT>Current version comment:</TH>"
            "<TD><INPUT TYPE=text NAME=\"comment\" SIZE=40 VALUE=\""
            << versions->sComment << "\"></TD></TR>";
    } else { 
        con << TAG << 
            "<TR><TH ALIGN=LEFT>Current version:</TH>"
            "<TD><INPUT TYPE=text NAME=\"version\" SIZE=8></TD></TR>"
            "<TR><TH ALIGN=LEFT>Current version label:</TH>"
            "<TD><INPUT TYPE=text NAME=\"label\" SIZE=20></TD></TR>"
            "<TR><TH ALIGN=LEFT>Current version comment:</TH>"
            "<TD><INPUT TYPE=text NAME=\"comment\" SIZE=40></TD></TR>";
    }
    con << TAG << "</TABLE><BR>";
    if (personStatus != Person::isUser) { 
        con << TAG << 
            "<INPUT TYPE=submit VALUE=\"Update\">&nbsp;<INPUT TYPE=reset>";
    }
    con << TAG << "</FORM><P>"
        "<TABLE><TR><TH ALIGN=LEFT>Engineers:</TH>"
        "<TD><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress()
        << "\"><INPUT TYPE=HIDDEN NAME=\"page\" VALUE=\"userForm\">"
        "<INPUT TYPE=HIDDEN NAME=\"myself\" VALUE=\"" << myself <<
        "\"><SELECT NAME=\"name\" SIZE=1>";
    if (products->setEngineers.length() != 0) { 
        print(con, products->setEngineers);     
        con << TAG << "</SELECT>";
        if (personStatus != Person::isUser) { 
            con << TAG << "<INPUT TYPE=submit VALUE=\"Select\">";
        }
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG <<
        "</FORM></TD></TR>"
        "<TR><TH ALIGN=LEFT>Users:</TH>"
        "<TD><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() 
        << "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"userForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself <<
        "\"><SELECT NAME=\"name\" SIZE=1>";
    if (products->setUsers.length() != 0) { 
        print(con, products->setUsers);         
        con << TAG << "</SELECT>";
        if (personStatus != Person::isUser) { 
            con << TAG << "<INPUT TYPE=submit VALUE=\"Select\">";
        }
    } else { 
        con << TAG << EMPTY_LIST;
    }    
    con << TAG << 
        "</FORM></TD></TR>"
        "<TR><TH ALIGN=LEFT>Bugs:</TH>"
        "<TD><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() 
        << "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"bugForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself <<
        "\"><SELECT NAME=\"bug\" SIZE=1>";
    if (products->setBugs.length() != 0) { 
        print(con, products->setBugs);
        con << TAG << 
            "</SELECT><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << 
        "</FORM></TD></TR>"
        "<TR><TH ALIGN=LEFT>Versions:</TH><TD>"
        "<FORM METHOD=POST ACTION=\""<<con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() 
        << "\"><INPUT TYPE=hidden NAME=\"page\" VALUE=\"versionForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself <<
        "\"><INPUT TYPE=HIDDEN NAME=\"software\" VALUE=\"" << software << 
        "\"><SELECT NAME=\"version\" SIZE=1>";   
    initialVersion = products->pVersions;
    if (versions.select(qAllVersions) != 0) { 
        print(con, versions);                                           
        con << TAG << "</SELECT><INPUT TYPE=submit VALUE=\"Select\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</FORM></TD></TR></TABLE>";
    mainMenuReference(con);
    return true;
}


bool updateSoftware(WWWconnection& con) 
{
    char* software = con.get("software");
    key = software;
    if (products.select(qSoftware) == 0) { 
        error(con, "No such software product");
        return true;
    }
    Version version;
    char* currentVersion = con.get("version");
    version.sLabel = con.get("label");
    version.sComment = con.get("comment");
    if (sscanf(currentVersion, "%d.%d", &majorVersion, &minorVersion) != 2) 
    { 
        error(con, "Bad version number (MAJOR.MINOR expected)");
        return true;
    }  
    products->sName = con.get("newname");
    version.majorVersionNumber = majorVersion;
    version.minorVersionNumber = minorVersion;
    version.released = dbDateTime::current();
    if (products->pVersions != null) { 
        initialVersion = products->pVersions;
        if (versions.select(qVersion) != 0) { 
            versions->sComment = version.sComment;
            versions->sLabel = version.sLabel;
        } else { 
            versions.at(products->pVersions);
            if (versions->majorVersionNumber > majorVersion ||
                (versions->majorVersionNumber == majorVersion && 
                 versions->minorVersionNumber > minorVersion))
            {
                error(con, "Version number less than of current version");
                return true;
            }
            version.pNext = products->pVersions;
            products->pVersions = insert(version);
        } 
    } else { 
        version.pNext = null;
        products->pVersions = insert(version);
    }
    products.update();
    con.addPair("name", con.get("myself"));
    return userForm(con);
}


bool versionForm(WWWconnection& con) 
{
    char* software = con.get("software");
    char* myself = con.get("myself");
    char  buf[64];
    key = software;
    if (products.select(qSoftware) == 0) { 
        error(con, "No such software product");
        return true;
    }
    char* versionString = con.get("version");
    if (sscanf(versionString, "%d.%d", &majorVersion, &minorVersion) != 2) { 
        error(con, "Bad version format");
        return true;
    }
    initialVersion = products->pVersions;
    if (versions.select(qVersion) == 0) { 
        error(con, "No such version");
        return true;
    }
    key = myself;
    if (persons.select(qPerson) == 0) { 
        error(con, "No such person");
        return true;
    }
    con << TAG << 
        HTML_HEAD "<TITLE>" << software << " v. " << versionString << 
        "</TITLE></HEAD>"
        BODY 
        "<H2>"  << software << " v. " << versionString << "</H2>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden "
        "NAME=\"page\" VALUE=\"updateVersion\">"
        "<INPUT TYPE=hidden NAME=\"software\" VALUE=\"" << software << "\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
        "<INPUT TYPE=hidden NAME=\"version\" VALUE=\"" << versionString <<"\">"
        "<TABLE><TR><TH ALIGN=LEFT>Released:</TH>"
        "<TD>" << versions->released.asString(buf, sizeof buf) << "</TD></TR>"
        "<TR><TH ALIGN=LEFT>Label:</TH>"
        "<TD><INPUT TYPE=text NAME=\"label\" SIZE=20 VALUE=\""
        << versions->sLabel << "\"></TD></TR>"
        "<TR><TH ALIGN=LEFT>Comment:</TH>"
        "<TD><INPUT TYPE=text NAME=\"comment\" SIZE=40 VALUE=\"" 
        << versions->sComment << "\"></TD></TR></TABLE>";
    if (persons->status != Person::isUser) { 
        con << TAG << 
            "<P><INPUT TYPE=submit NAME=\"action\" VALUE=\"Update\">";
        if (persons->status == Person::isAdministrator) { 
            con << TAG << 
                "&nbsp;<INPUT TYPE=submit NAME=\"action\" VALUE=\"Remove\">";
        }
        con << TAG << "&nbsp;<INPUT TYPE=reset>";
    }
    con << TAG << "<P></FORM>"
        "<B>Bugs:</B><BR>"      
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" 
        << con.getAddress() << "\"><INPUT TYPE=hidden NAME=\"page\" "
        "VALUE=\"bugForm\">"
        "<INPUT TYPE=hidden NAME=\"myself\" VALUE=\"" << myself << "\">"
        "<SELECT NAME=\"bug\" SIZE=5>";
    if (versions->setBugs.length() != 0) {
        print(con, versions->setBugs);
        con << TAG << 
           "</SELECT><BR><INPUT TYPE=submit NAME=\"action\" VALUE=\"Select\">";
    } else { 
        con << TAG << EMPTY_LIST;
    }   
    con << TAG << "</FORM>";
    mainMenuReference(con);
    return true;
}

bool updateVersion(WWWconnection& con) 
{
    char* software = con.get("software");
    key = software;
    if (products.select(qSoftware) == 0) { 
        error(con, "No such software product");
        return true;
    }
    if (sscanf(con.get("version"), "%d.%d", &majorVersion, &minorVersion) != 2)
    { 
        error(con, "Bad version format");
        return true;
    }
    if (strcmp(con.get("action"), "Remove") == 0) { 
        dbReference<Version> prev, curr = null, next = products->pVersions;
        do  { 
            prev = curr;
            if (next == null) { 
                error(con, "No such version");
                return true;
            } 
            versions.at(next);
            curr = next;
            next = versions->pNext;
        } while (versions->majorVersionNumber != majorVersion || 
                 versions->minorVersionNumber != minorVersion);
        if (versions->setBugs.length() != 0) { 
            error(con, "Can not remove version with non-empty bugs list");
            return true;
        }
        versions.remove();
        if (prev == null) { 
            products->pVersions = next;
            products.update();
        } else { 
            versions.at(prev);
            versions->pNext = next;
            versions.update();
        }
        con.addPair("action", "Select");
        return softwareForm(con);
    }    
    initialVersion = products->pVersions;
    if (versions.select(qVersion) == 0) { 
        error(con, "No such version");
        return true;
    }
    versions->sComment = con.get("comment");
    versions->sLabel = con.get("label");
    versions.update();
    return versionForm(con);
}

WWWapi::dispatcher dispatchTable[] = { 
    {"addUserForm", addUserForm},
    {"addEngineerForm", addEngineerForm},
    {"addSoftwareForm", addSoftwareForm},
    {"selectSoftwareForm", selectSoftwareForm},
    {"removeSoftwareForm", removeSoftwareForm},
    {"selectPersonForm", selectPersonForm},
    {"removePersonForm", removePersonForm},
    {"selectBugForm", selectBugForm},
    {"removeBugForm", removeBugForm},
    {"changePasswordForm", changePasswordForm},
    {"shutdown", shutdown},
    {"userForm", userForm},
    {"softwareForm", softwareForm},
    {"addUser", addUser},
    {"addEngineer", addEngineer},
    {"removePerson", removePerson},
    {"addSoftware", addSoftware},
    {"removeSoftware", removeSoftware},
    {"removeBug", removeBug},
    {"changePassword", changePassword},
    {"updatePerson", updatePerson},
    {"login", login},
    {"bugQueryForm", bugQueryForm},
    {"bugQuery", bugQuery},
    {"userForm", userForm},
    {"createBugReportForm", createBugReportForm},
    {"bugForm", bugForm},
    {"createBugReport", createBugReport},
    {"bugForm", bugForm},
    {"updateBug", updateBug},
    {"updateReportForm", updateReportForm},
    {"updateWorkAroundForm", updateWorkAroundForm},
    {"addReportForm", addReportForm},
    {"addReport", addReport},
    {"addWorkAroundForm", addWorkAroundForm},
    {"addWorkAround", addWorkAround},
    {"updateReport", updateReport},
    {"updateWorkAround", updateWorkAround},
    {"attachToProject", attachToProject},
    {"registerSoftware", registerSoftware},
    {"softwareForm", softwareForm},
    {"updateSoftware", updateSoftware},
    {"versionForm", versionForm},
    {"updateVersion", updateVersion}
}; 

#ifdef USE_EXTERNAL_HTTP_SERVER    
CGIapi wwwServer(db, itemsof(dispatchTable), dispatchTable);
char* defaultAddress = "localhost:6101";
socket_t::socket_domain domain = socket_t::sock_local_domain;
#else
HTTPapi wwwServer(db, itemsof(dispatchTable), dispatchTable);
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
    if (!db.open(_T("bugdb"))) { 
        fprintf(stderr, "Failed to open database\n");
        return EXIT_FAILURE;
    }
    qBug = "bugId=",bugId; 
    qReport = "index=",reportId,"start from",firstReport,"follow by pNext";
    qAllReports = "start from",firstReport,"follow by pNext";
    qVersion = "majorVersionNumber=",majorVersion,"and minorVersionNumber=",
        minorVersion,"start from",initialVersion,"follow by pNext";
    qAllVersions = "start from",initialVersion,"follow by pNext";
    qPerson = "sName=",&key;
    qSoftware = "sName=",&key;

    if (sequencer.select() == 0) { 
        BugSequencer seq;
        seq.nBugs = 0;
        insert(seq);
    }
    if (persons.select() == 0) {
        Person administrator;
        administrator.sName = "administrator";
        administrator.sEmailAddress = "root";
        administrator.sPassword = "";
        administrator.status = Person::isAdministrator;
        administrator.nReports = 0;
        insert(administrator);
    }
    WWWconnection con;
    while (wwwServer.connect(con) && wwwServer.serve(con)); 
    db.close(); 
    printf("End of session\n");
    return EXIT_SUCCESS;
}

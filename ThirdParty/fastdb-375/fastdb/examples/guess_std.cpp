//-< GUESS_STD.CPP >-------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     15-Sep-2001  K.A. Knizhnik  * / [] \ *
//                          Last update: 15-Sep-2001  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Sample of database application using std::string type: game "Guess an animal"
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <iostream>

#ifndef NO_NAMESPACES
using namespace std;
#endif

USE_FASTDB_NAMESPACE

class Guess {
  public:
    dbReference<Guess> yes;
    dbReference<Guess> no;
    string             question;

    TYPE_DESCRIPTOR((FIELD(yes), FIELD(question), FIELD(no)));
};

REGISTER(Guess);

void input(string const& prompt, string& answer)
{
    do {
        cout << prompt;
#if 0
// doesn't work with Visual C++ 7.0
        getline(cin, answer);
#else
        char buf[256];
        buf[0] = '\0';
        fgets(buf, (int)sizeof(buf), stdin);
        int len = (int)strlen(buf);
        if (len > 0) { 
            buf[len-1] = '\0';
        }
        answer = buf;
#endif
    } while (answer.length() == 0);
}

bool askQuestion(string const& question) {
    string answer;
    while (true) { 
        answer = "";
        input(question, answer);
        if (answer == "y") { 
            return true;
        }
        if (answer == "n") { 
            return false;
        }
        cout << "Please answer y/n\n";
    }
}


dbReference<Guess> whoIsIt(dbReference<Guess> const& parent) {
    string animal, difference;
    input("What is it ? ", animal);
    input("What is a difference from other ? ", difference);
    Guess node;
    node.question = animal;
    dbReference<Guess> child = insert(node);
    node.question = difference;
    node.yes = child;
    node.no = parent;
    return insert(node);
}


dbReference<Guess> dialog(dbCursor<Guess>& cur) {
    string question;
    dbCursor<Guess> c(dbCursorForUpdate);
    question = "May be ";
    question += cur->question;
    question += " (y/n) ? ";
    if (askQuestion(question)) {
        if (cur->yes == null) {
            cout << "It was very simple question for me...\n";
        } else {
            c.at(cur->yes);
            dbReference<Guess> clarify = dialog(c);
            if (clarify != null) {
                cur->yes = clarify;
                cur.update();
            }
        }
    } else {
        if (cur->no == null) {
            if (cur->yes == null) {
                return whoIsIt(cur.currentId());
            } else {
                cur->no = whoIsIt(null);
                cur.update();
            }
        } else {
            c.at(cur->no);
            dbReference<Guess> clarify = dialog(c);
            if (clarify != null) {
                cur->no = clarify;
                cur.update();
            }
        }
    }
    return null;
}




int main()
{
    dbDatabase db;
    if (db.open(_T("guess.dbs"))) {
        dbCursor<Guess> cur(dbCursorForUpdate);
        while (askQuestion("Think of an animal. Ready (y/n) ? ")) {
            if (cur.select() != 0) {
                cur.next(); // first question is in record number 2
                dialog(cur);
            } else {
                whoIsIt(null);
            }
            db.commit();
        }
        db.close();
        cout << "End of the game\n";
        return EXIT_SUCCESS;
    } else {
        cerr << "Failed to open database\n";
        return EXIT_FAILURE;
    }
}



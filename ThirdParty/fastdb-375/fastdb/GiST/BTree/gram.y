%{
#include <iostream.h>
#ifndef UNIX
#include <malloc.h>
#endif
#include "BT.h"
#include "command.h"

void CommandPrompt();
int yyerror(char *msg);
int yylex();
%}

%union {
    BTkey *key;
    char *string;
    BToper oper;
    GiSTpredicate *pred;
    int number;
}

%token <string> ID
%token <key> STRCONST
%token <number> INTCONST
%token CREATE INSERT DROP OPEN CLOSE QUIT SELECT DELETE WHERE FROM NL ERROR
%token AND OR DEBUG HELP DUMP
%token '(' ')' ',' '>' '=' '<' opLE opGE opNE

%left OR
%left AND
%right NOT

%type <number> dump_page
%type <pred> predicate where_clause
%type <key> key
%type <oper> operator

%start commands

%%

commands
	:
	| commands command
	{ CommandPrompt(); }
	;

command
	: CREATE ID ID NL
		{ CommandCreate($2, $3);
                  if ($2) delete $2;
                  if ($3) delete $3; }
	| OPEN ID ID NL
		{ CommandOpen($2, $3);
                  if ($2) delete $2; 
				  if ($3) delete $3; }
	| DROP ID NL
		{ CommandDrop($2);
                  if ($2) delete $2; }
	| CLOSE ID NL
		{ CommandClose($2);
                  if ($2) delete $2; }
	| SELECT FROM ID where_clause NL
		{ CommandSelect($3, *$4);
                  if ($3) delete $3;
                  if ($4) delete $4; }
	| INSERT ID '(' key ',' INTCONST ')' NL
		{ CommandInsert($2, *$4, $6);
                  if ($2) delete $2;
                  if ($4) delete $4; }
	| DELETE FROM ID where_clause NL
		{ CommandDelete($3, *$4);
                  if ($3) delete $3;
                  if ($4) delete $4; }
	| QUIT NL
		{ CommandQuit(); }
        | DEBUG NL
		{ CommandDebug(); }
        | HELP NL
                { CommandHelp(); }
        | DUMP ID dump_page NL
		{ CommandDump($2, $3);
                  if ($2) delete $2; }
	| error NL
		{ yyclearin; yyerrok; }
	| NL
	;

dump_page
        :
	       { $$ = 1; }
        | INTCONST
	       { $$ = $1; }
        ;

where_clause
	:
	  { $$ = new TruePredicate; }
	| WHERE predicate
	  { $$ = $2; }
	;

key
	: STRCONST 
	;

predicate
	: ID operator key
		{ $$ = new BTpredicate($2, *$3); }
	| predicate OR predicate
		{ $$ = new OrPredicate($1, $3); }
	| predicate AND predicate
		{ $$ = new AndPredicate($1, $3); }
	| NOT predicate
		{ $$ = new NotPredicate($2); }
	| '(' predicate ')'
		{ $$ = $2; }
	;

operator
	: '<'  { $$ = BTLessThan;     }
	| '>'  { $$ = BTGreaterThan;  }
	| opLE { $$ = BTLessEqual;    }
	| opGE { $$ = BTGreaterEqual; }
	| '='  { $$ = BTEqual;        }
        | opNE { $$ = BTNotEqual;     }
	;

%%

int yyerror(char *errmsg)
{
    extern char *yytext;

    cerr << "Parser error!\n";

    return 0;
}

extern "C" int yywrap()
{
	return 0;
}


%{
#include <iostream.h>
#ifndef UNIX
#include <malloc.h>
#endif
#include "RT.h"
#include "command.h"
#define YYDEBUG 1

void CommandPrompt();
int yyerror(char *msg);
int yylex();
%}

%union {
    RTkey *key;
    char *string;
    RToper oper;
    GiSTpredicate *pred;
    int number;
    double dbl;
}

%token <string> ID
/* %token <key> STRCONST */
%token <number> INTCONST 
%token <dbl> DBLCONST
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
%type <dbl> dub

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
	:  '(' dub ',' dub ',' dub ',' dub ')'
                { $$ = new RTkey($2, $4, $6, $8); }
	;

dub
        : INTCONST { $$ = (double)$1; }
        | DBLCONST  
        ;

predicate
	: ID operator key
		{ $$ = new RTpredicate($2, *$3); }
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
	: '&'  { $$ = RToverlap;     }
	| '<'  { $$ = RTcontained;  }
	| '>'  { $$ = RTcontains;    }
	| '='  { $$ = RTEqual;        }
	;

%%

int yyerror(char *errmsg)
{
    extern char *yytext;

    cerr << "Parser error!\n";

    return 1;
}

extern "C" int yywrap()
{
	return 0;
}


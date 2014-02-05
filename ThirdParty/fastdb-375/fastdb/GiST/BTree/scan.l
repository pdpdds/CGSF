%{
#include <iostream.h>
#include "BT.h"
#include "gram.tab.h"
%}

ws		[ \t]+
comment	#[^\n]*
qstring	\"[^\"\n]*[\"\n]
id		[a-zA-Z][a-zA-Z0-9]*
nl		[\r\n]
int		"-"?[0-9]+

%%

{ws}	;
{comment} ;
{qstring} { yylval.key = new BTkey((char *)(yytext+1), strlen(yytext+1));
            if(yylval.key->key[yyleng-2] != '\"') {
		cerr << "Scanner - Unterminated character string.\n";
		return ERROR;
	    } else {
yylval.key->len--;
		    return STRCONST;
	    }
	  }
{int}	  { yylval.number = atoi((const char *)yytext);
	    return INTCONST;
	  }
"(" |
")" |
"," |
"<" |
">" |
"="  { return yytext[0]; }
"<>" { return opNE; }
"<=" { return opLE; }
">=" { return opGE; }

create		{ return CREATE; }
open            { return OPEN; }
close           { return CLOSE; }
drop		{ return DROP; }
insert		{ return INSERT; }
delete          { return DELETE; }
select          { return SELECT; }
debug           { return DEBUG; }
dump            { return DUMP; }
help            { return HELP; }
from            { return FROM; }
and             { return AND; }
or              { return OR; }
not             { return NOT; }
where		{ return WHERE; }
quit		{ return QUIT; }
{id}	        { yylval.string = strdup((const char *)yytext); return ID; }
{nl}		{ return NL; }
<<EOF>>         { yyterminate(); }
.		{ cerr << "Lexical error in input string: invalid character\n";
		  return ERROR; }

%%

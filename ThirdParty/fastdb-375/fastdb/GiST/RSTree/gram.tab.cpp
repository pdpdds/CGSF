
/*  A Bison parser, made from gram.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	ID	258
#define	INTCONST	259
#define	DBLCONST	260
#define	CREATE	261
#define	INSERT	262
#define	DROP	263
#define	OPEN	264
#define	CLOSE	265
#define	QUIT	266
#define	SELECT	267
#define	DELETE	268
#define	WHERE	269
#define	FROM	270
#define	NL	271
#define	ERROR	272
#define	AND	273
#define	OR	274
#define	DEBUG	275
#define	HELP	276
#define	DUMP	277
#define	opLE	278
#define	opGE	279
#define	opNE	280
#define	NOT	281

#line 1 "gram.y"

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

#line 15 "gram.y"
typedef union {
    RTkey *key;
    char *string;
    RToper oper;
    GiSTpredicate *pred;
    int number;
    double dbl;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		79
#define	YYFLAG		-32768
#define	YYNTBASE	34

#define YYTRANSLATE(x) ((unsigned)(x) <= 281 ? yytranslate[x] : 42)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    33,     2,    23,
    24,     2,     2,    25,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    28,
    27,    26,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    29,    30,    31,
    32
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     9,    14,    18,    22,    28,    37,    43,
    46,    49,    52,    57,    60,    62,    63,    65,    66,    69,
    79,    81,    83,    87,    91,    95,    98,   102,   104,   106,
   108
};

static const short yyrhs[] = {    -1,
    34,    35,     0,     6,     3,     3,    16,     0,     9,     3,
     3,    16,     0,     8,     3,    16,     0,    10,     3,    16,
     0,    12,    15,     3,    37,    16,     0,     7,     3,    23,
    38,    25,     4,    24,    16,     0,    13,    15,     3,    37,
    16,     0,    11,    16,     0,    20,    16,     0,    21,    16,
     0,    22,     3,    36,    16,     0,     1,    16,     0,    16,
     0,     0,     4,     0,     0,    14,    40,     0,    23,    39,
    25,    39,    25,    39,    25,    39,    24,     0,     4,     0,
     5,     0,     3,    41,    38,     0,    40,    19,    40,     0,
    40,    18,    40,     0,    32,    40,     0,    23,    40,    24,
     0,    33,     0,    28,     0,    26,     0,    27,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    47,    48,    53,    57,    61,    64,    67,    71,    75,    79,
    81,    83,    85,    88,    90,    94,    96,   101,   103,   108,
   113,   114,   118,   120,   122,   124,   126,   131,   132,   133,
   134
};

static const char * const yytname[] = {   "$","error","$undefined.","ID","INTCONST",
"DBLCONST","CREATE","INSERT","DROP","OPEN","CLOSE","QUIT","SELECT","DELETE",
"WHERE","FROM","NL","ERROR","AND","OR","DEBUG","HELP","DUMP","'('","')'","','",
"'>'","'='","'<'","opLE","opGE","opNE","NOT","'&'","commands","command","dump_page",
"where_clause","key","dub","predicate","operator",""
};
#endif

static const short yyr1[] = {     0,
    34,    34,    35,    35,    35,    35,    35,    35,    35,    35,
    35,    35,    35,    35,    35,    36,    36,    37,    37,    38,
    39,    39,    40,    40,    40,    40,    40,    41,    41,    41,
    41
};

static const short yyr2[] = {     0,
     0,     2,     4,     4,     3,     3,     5,     8,     5,     2,
     2,     2,     4,     2,     1,     0,     1,     0,     2,     9,
     1,     1,     3,     3,     3,     2,     3,     1,     1,     1,
     1
};

static const short yydefact[] = {     1,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    15,     0,     0,     0,     2,    14,     0,     0,     0,     0,
     0,    10,     0,     0,    11,    12,    16,     0,     0,     5,
     0,     6,    18,    18,    17,     0,     3,     0,     0,     4,
     0,     0,     0,    13,    21,    22,     0,     0,     0,     0,
     0,    19,     7,     9,     0,     0,    30,    31,    29,    28,
     0,     0,    26,     0,     0,     0,     0,    23,    27,    25,
    24,     0,     8,     0,     0,     0,    20,     0,     0
};

static const short yydefgoto[] = {     1,
    15,    36,    42,    39,    47,    52,    61
};

static const short yypact[] = {-32768,
     1,   -13,     3,    13,    27,    31,    37,    25,    28,    29,
-32768,    26,    30,    42,-32768,-32768,    44,    33,    34,    45,
    35,-32768,    46,    49,-32768,-32768,    50,    39,    36,-32768,
    41,-32768,    47,    47,-32768,    48,-32768,    32,    38,-32768,
    -3,    51,    52,-32768,-32768,-32768,    40,    54,    -2,    -3,
    -3,    20,-32768,-32768,    32,    53,-32768,-32768,-32768,-32768,
    36,     9,-32768,    -3,    -3,    55,    56,-32768,-32768,-32768,
    57,    32,-32768,    58,    32,    60,-32768,    62,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,    19,    -1,   -40,   -46,-32768
};


#define	YYLAST		84


static const short yytable[] = {    49,
    78,     2,    16,    62,    63,    17,     3,     4,     5,     6,
     7,     8,     9,    10,    66,    18,    11,    70,    71,    50,
    12,    13,    14,    57,    58,    59,    64,    65,    51,    19,
    60,    74,    69,    20,    76,    45,    46,    64,    65,    21,
    22,    25,    23,    24,    27,    26,    28,    31,    33,    30,
    32,    34,    43,    35,    37,    29,    40,    56,    38,    68,
    41,    79,    48,    44,    55,     0,    53,    54,     0,     0,
     0,    73,     0,     0,    64,     0,    67,     0,     0,    72,
     0,     0,    75,    77
};

static const short yycheck[] = {     3,
     0,     1,    16,    50,    51,     3,     6,     7,     8,     9,
    10,    11,    12,    13,    55,     3,    16,    64,    65,    23,
    20,    21,    22,    26,    27,    28,    18,    19,    32,     3,
    33,    72,    24,     3,    75,     4,     5,    18,    19,     3,
    16,    16,    15,    15,     3,    16,     3,     3,     3,    16,
    16,     3,    34,     4,    16,    23,    16,     4,    23,    61,
    14,     0,    25,    16,    25,    -1,    16,    16,    -1,    -1,
    -1,    16,    -1,    -1,    18,    -1,    24,    -1,    -1,    25,
    -1,    -1,    25,    24
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not __GNUC__ */
#if HAVE_ALLOCA_H
#include <alloca.h>
#else /* not HAVE_ALLOCA_H */
#ifdef _AIX
 #pragma alloca
#else /* not _AIX */
char *alloca ();
#endif /* not _AIX */
#endif /* not HAVE_ALLOCA_H */
#endif /* not __GNUC__ */

extern int yylex();
extern void yyerror();

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 192 "bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 2:
#line 49 "gram.y"
{ CommandPrompt(); ;
    break;}
case 3:
#line 54 "gram.y"
{ CommandCreate(yyvsp[-2].string, yyvsp[-1].string);
                  if (yyvsp[-2].string) delete yyvsp[-2].string; 
	          if (yyvsp[-1].string) delete yyvsp[-1].string; ;
    break;}
case 4:
#line 58 "gram.y"
{ CommandOpen(yyvsp[-2].string, yyvsp[-1].string);
                  if (yyvsp[-2].string) delete yyvsp[-2].string;
		  if (yyvsp[-1].string) delete yyvsp[-1].string; ;
    break;}
case 5:
#line 62 "gram.y"
{ CommandDrop(yyvsp[-1].string);
                  if (yyvsp[-1].string) delete yyvsp[-1].string; ;
    break;}
case 6:
#line 65 "gram.y"
{ CommandClose(yyvsp[-1].string);
                  if (yyvsp[-1].string) delete yyvsp[-1].string; ;
    break;}
case 7:
#line 68 "gram.y"
{ CommandSelect(yyvsp[-2].string, *yyvsp[-1].pred);
                  if (yyvsp[-2].string) delete yyvsp[-2].string;
                  if (yyvsp[-1].pred) delete yyvsp[-1].pred; ;
    break;}
case 8:
#line 72 "gram.y"
{ CommandInsert(yyvsp[-6].string, *yyvsp[-4].key, yyvsp[-2].number);
                  if (yyvsp[-6].string) delete yyvsp[-6].string;
                  if (yyvsp[-4].key) delete yyvsp[-4].key; ;
    break;}
case 9:
#line 76 "gram.y"
{ CommandDelete(yyvsp[-2].string, *yyvsp[-1].pred);
                  if (yyvsp[-2].string) delete yyvsp[-2].string;
                  if (yyvsp[-1].pred) delete yyvsp[-1].pred; ;
    break;}
case 10:
#line 80 "gram.y"
{ CommandQuit(); ;
    break;}
case 11:
#line 82 "gram.y"
{ CommandDebug(); ;
    break;}
case 12:
#line 84 "gram.y"
{ CommandHelp(); ;
    break;}
case 13:
#line 86 "gram.y"
{ CommandDump(yyvsp[-2].string, yyvsp[-1].number);
                  if (yyvsp[-2].string) delete yyvsp[-2].string; ;
    break;}
case 14:
#line 89 "gram.y"
{ yyclearin; yyerrok; ;
    break;}
case 16:
#line 95 "gram.y"
{ yyval.number = 1; ;
    break;}
case 17:
#line 97 "gram.y"
{ yyval.number = yyvsp[0].number; ;
    break;}
case 18:
#line 102 "gram.y"
{ yyval.pred = new TruePredicate; ;
    break;}
case 19:
#line 104 "gram.y"
{ yyval.pred = yyvsp[0].pred; ;
    break;}
case 20:
#line 109 "gram.y"
{ yyval.key = new RTkey(yyvsp[-7].dbl, yyvsp[-5].dbl, yyvsp[-3].dbl, yyvsp[-1].dbl); ;
    break;}
case 21:
#line 113 "gram.y"
{ yyval.dbl = (double)yyvsp[0].number; ;
    break;}
case 23:
#line 119 "gram.y"
{ yyval.pred = new RTpredicate(yyvsp[-1].oper, *yyvsp[0].key); ;
    break;}
case 24:
#line 121 "gram.y"
{ yyval.pred = new OrPredicate(yyvsp[-2].pred, yyvsp[0].pred); ;
    break;}
case 25:
#line 123 "gram.y"
{ yyval.pred = new AndPredicate(yyvsp[-2].pred, yyvsp[0].pred); ;
    break;}
case 26:
#line 125 "gram.y"
{ yyval.pred = new NotPredicate(yyvsp[0].pred); ;
    break;}
case 27:
#line 127 "gram.y"
{ yyval.pred = yyvsp[-1].pred; ;
    break;}
case 28:
#line 131 "gram.y"
{ yyval.oper = RToverlap;     ;
    break;}
case 29:
#line 132 "gram.y"
{ yyval.oper = RTcontained;  ;
    break;}
case 30:
#line 133 "gram.y"
{ yyval.oper = RTcontains;    ;
    break;}
case 31:
#line 134 "gram.y"
{ yyval.oper = RTEqual;        ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 487 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 137 "gram.y"


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


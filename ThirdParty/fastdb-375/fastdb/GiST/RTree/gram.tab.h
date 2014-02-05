typedef union {
    RTkey *key;
    char *string;
    RToper oper;
    GiSTpredicate *pred;
    int number;
    double dbl;
} YYSTYPE;
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


extern YYSTYPE yylval;

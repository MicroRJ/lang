/*
** See Copyright Notice In lang.h
** llexer.h
** Lexical Analyzer
*/


typedef struct ltoken {
	enum ltokentype type;
	char *line;
	union {
		char const *s;
		llong   i;
		lnumber    n;
	};
} ltoken;

#include <src/ltoken.h>

#if 0
#define KEYWORD_XLIST \
KEYWORD_XITEM(FUN, "fun")\
KEYWORD_XITEM(NIL, "nil")\
KEYWORD_XITEM(LOCAL, "local")\
KEYWORD_XITEM(LET, "let")\
KEYWORD_XITEM(FINALY, "finally")\
KEYWORD_XITEM(IN, "in")\
KEYWORD_XITEM(FOR, "for")\
KEYWORD_XITEM(WHILE, "while")\
KEYWORD_XITEM(DO, "do")\
KEYWORD_XITEM(BREAK, "break")\
KEYWORD_XITEM(CONTINUE, "continue")\
KEYWORD_XITEM(LEAVE, "leave")\
KEYWORD_XITEM(IF, "if")\
KEYWORD_XITEM(IFF, "iff")\
KEYWORD_XITEM(ELIF, "elif")\
KEYWORD_XITEM(THEN, "then")\
KEYWORD_XITEM(ELSE, "else")\
KEYWORD_XITEM(LOAD, "load")\
KEYWORD_XITEM(STKGET, "stkget")\
KEYWORD_XITEM(STKLEN, "stklen")\
KEYWORD_XITEM(TRUE, "true")\
KEYWORD_XITEM(FALSE, "false")


typedef enum TokenRank {
	TK_RANK_NONE = -1,
	/* lowest to highest */
	TK_RANK_RANGE = 1,
	TK_RANK_OR,
	TK_RANK_AND,
	TK_RANK_BXOR,
	TK_RANK_BOR,
	TK_RANK_BAND,
	TK_RANK_EQL,
	TK_RANK_REL,
	TK_RANK_SHIFT,
	TK_RANK_ADD,
	TK_RANK_MUL,
} TokenRank;


#define TK_XLIST \
TK_XITEM(INTEGER,             TK_RANK_NONE,   "literal-integer")\
TK_XITEM(NUMBER,              TK_RANK_NONE,   "literal-number")\
TK_XITEM(STRING,              TK_RANK_NONE,   "literal-string")\
TK_XITEM(LETTER,              TK_RANK_NONE,   "literal-letter")\
TK_XITEM(WORD,                TK_RANK_NONE,   "identifier")\
TK_XITEM(QMARK,               TK_RANK_NONE,   "?")\
TK_XITEM(NEGATE,              TK_RANK_NONE,   "!")\
TK_XITEM(ASSIGN_QUESTION,     TK_RANK_NONE,   "?=")\
TK_XITEM(ASSIGN,              TK_RANK_NONE,   "=")\
TK_XITEM(COLON,               TK_RANK_NONE,   ":")\
TK_XITEM(SEMI_COLON,          TK_RANK_NONE,   ";")\
TK_XITEM(DOT,                 TK_RANK_NONE,   ".")\
TK_XITEM(COMMA,               TK_RANK_NONE,   ",")\
TK_XITEM(SQUARE_LEFT,         TK_RANK_NONE,   "[")\
TK_XITEM(SQUARE_RIGHT,        TK_RANK_NONE,   "]")\
TK_XITEM(CURLY_LEFT,          TK_RANK_NONE,   "{")\
TK_XITEM(CURLY_RIGHT,         TK_RANK_NONE,   "}")\
TK_XITEM(PAREN_LEFT,          TK_RANK_NONE,   "(")\
TK_XITEM(PAREN_RIGHT,         TK_RANK_NONE,   ")")\
TK_XITEM(MUL,                 TK_RANK_MUL,    "*")\
TK_XITEM(DIV,                 TK_RANK_MUL,    "/")\
TK_XITEM(MODULUS,             TK_RANK_MUL,    "%")\
TK_XITEM(ADD,                 TK_RANK_ADD,    "+")\
TK_XITEM(SUB,                 TK_RANK_ADD,    "-")\
TK_XITEM(RIGHT_SHIFT,    		TK_RANK_SHIFT,  ">>")\
TK_XITEM(LEFT_SHIFT,          TK_RANK_SHIFT,  "<<")\
TK_XITEM(LESS_THAN,           TK_RANK_REL,    "<")\
TK_XITEM(LESS_THAN_EQUAL,     TK_RANK_REL,    "<=")\
TK_XITEM(GREATER_THAN,        TK_RANK_REL,    ">")\
TK_XITEM(GREATER_THAN_EQUAL,  TK_RANK_REL,    ">=")\
TK_XITEM(EQUALS,              TK_RANK_EQL,    "==")\
TK_XITEM(NOT_EQUALS,          TK_RANK_EQL,    "!=")\
TK_XITEM(BIT_AND,             TK_RANK_BAND,   "&")\
TK_XITEM(BIT_XOR,             TK_RANK_BXOR,   "^")\
TK_XITEM(BIT_OR,              TK_RANK_OR,     "|")\
TK_XITEM(LOG_AND,             TK_RANK_AND,    "&&")\
TK_XITEM(LOG_OR,              TK_RANK_OR,     "||")\
TK_XITEM(DOT_DOT,             TK_RANK_RANGE,  "..")


typedef enum ltokentype {
	TK_NONE = 0,
#define TK_XITEM(NAME,PREC,RENDERING) XFUSE(TK_,NAME),
	TK_XLIST
#undef TK_XITEM
#define KEYWORD_XITEM(NAME,RENDERING) XFUSE(TK_,NAME),
	KEYWORD_XLIST
#undef KEYWORD_XITEM
} ltokentype;


char const *langX_tokenname(ltokentype k) {
	switch (k) {
#define TK_XITEM(NAME,PREC,RENDERING) \
		case XFUSE(TK_,NAME): {         \
			return RENDERING;            \
		}
		TK_XLIST
#undef TK_XITEM

#define KEYWORD_XITEM(NAME,RENDERING) \
		case XFUSE(TK_,NAME): {      \
			return RENDERING;            \
		}
		KEYWORD_XLIST
#undef KEYWORD_XITEM
	}
	return 0;
}


int langX_tokenprec(ltokentype k) {
	switch (k) {
#define TK_XITEM(NAME,PREC,RENDERING) \
		case XFUSE(TK_,NAME): {         \
			return PREC;                 \
		}
		TK_XLIST
#undef TK_XITEM
	}
	return TK_RANK_NONE;
}
#endif




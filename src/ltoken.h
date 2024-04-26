/*
** See Copyright Notice In elf.h
** (X) ltoken.h (Autogenerated)
** This file was autogenerated by token.lang in tools
*/


#define FIRST_KEYWORD 1
#define  LAST_KEYWORD 26


typedef struct ltoken {
	enum ltokentype type;
	llineid line;
	unsigned int eol: 1;
	union {
		char *s;
		elf_int i;
		elf_num n;
	};
} ltoken;


typedef enum ltokentype {
	TK_NONE                 = 0,
	TK_NEW                  = 1,
	TK_THIS                 = 2,
	TK_ENUM                 = 3,
	TK_FUN                  = 4,
	TK_NIL                  = 5,
	TK_LOCAL                = 6,
	TK_LET                  = 7,
	TK_FINALLY              = 8,
	TK_IN                   = 9,
	TK_FOR                  = 10,
	TK_FOREACH              = 11,
	TK_WHILE                = 12,
	TK_DO                   = 13,
	TK_BREAK                = 14,
	TK_CONTINUE             = 15,
	TK_LEAVE                = 16,
	TK_IF                   = 17,
	TK_IFF                  = 18,
	TK_ELIF                 = 19,
	TK_THEN                 = 20,
	TK_ELSE                 = 21,
	TK_LOAD                 = 22,
	TK_TRUE                 = 23,
	TK_FALSE                = 24,
	TK_STKGET               = 25,
	TK_STKLEN               = 26,
	TK_INTEGER              = 27,
	TK_NUMBER               = 28,
	TK_STRING               = 29,
	TK_LETTER               = 30,
	TK_WORD                 = 31,
	TK_QMARK                = 32,
	TK_NEGATE               = 33,
	TK_ASSIGN_QUESTION      = 34,
	TK_ASSIGN               = 35,
	TK_COLON                = 36,
	TK_SEMI_COLON           = 37,
	TK_DOT                  = 38,
	TK_COMMA                = 39,
	TK_SQUARE_LEFT          = 40,
	TK_SQUARE_RIGHT         = 41,
	TK_CURLY_LEFT           = 42,
	TK_CURLY_RIGHT          = 43,
	TK_PAREN_LEFT           = 44,
	TK_PAREN_RIGHT          = 45,
	TK_MUL                  = 46,
	TK_DIV                  = 47,
	TK_MODULUS              = 48,
	TK_ADD                  = 49,
	TK_SUB                  = 50,
	TK_RIGHT_SHIFT          = 51,
	TK_LEFT_SHIFT           = 52,
	TK_LESS_THAN            = 53,
	TK_LESS_THAN_EQUAL      = 54,
	TK_GREATER_THAN         = 55,
	TK_GREATER_THAN_EQUAL   = 56,
	TK_EQUALS               = 57,
	TK_NOT_EQUALS           = 58,
	TK_BIT_AND              = 59,
	TK_BIT_OR               = 60,
	TK_BIT_XOR              = 61,
	TK_LOG_AND              = 62,
	TK_LOG_OR               = 63,
	TK_DOT_DOT              = 64,
} ltokentype;


typedef struct ltokenintel {
	char *name;
	char prec;
	elf_hashint hash;
} ltokenintel;


elf_globaldecl ltokenintel elfX_tokenintel[] = {
	{"none",                 -2, 0},
	{"new",                  -1, 681154065},
	{"this",                 -1, 3660305025},
	{"enum",                 -1, 2171383808},
	{"fun",                  -1, 2832955486},
	{"nil",                  -1, 228849900},
	{"local",                -1, 2621662984},
	{"let",                  -1, 1349190650},
	{"finally",              -1, 2618047400},
	{"in",                   -1, 1094220446},
	{"for",                  -1, 2901640080},
	{"foreach",              -1, 3076491097},
	{"while",                -1, 231090382},
	{"do",                   -1, 1646057492},
	{"break",                -1, 3378807160},
	{"continue",             -1, 2977070660},
	{"leave",                -1, 1416872128},
	{"if",                   -1, 959999494},
	{"iff",                  -1, 1943388448},
	{"elif",                 -1, 3232090307},
	{"then",                 -1, 3844270454},
	{"else",                 -1, 3183434736},
	{"load",                 -1, 3859241449},
	{"true",                 -1, 1303515621},
	{"false",                -1, 184981848},
	{"stkget",               -1, 2654528197},
	{"stklen",               -1, 1897109354},
	{"literal-integer",      -2, 0},
	{"literal-number",       -2, 0},
	{"literal-string",       -2, 0},
	{"literal-letter",       -2, 0},
	{"identifier",           -2, 0},
	{"?",                    -2, 0},
	{"!",                    -2, 0},
	{"?=",                   -2, 0},
	{"=",                    -2, 0},
	{":",                    -2, 0},
	{";",                    -2, 0},
	{".",                    -2, 0},
	{",",                    -2, 0},
	{"[",                    -2, 0},
	{"]",                    -2, 0},
	{"{",                    -2, 0},
	{"}",                    -2, 0},
	{"(",                    -2, 0},
	{")",                    -2, 0},
	{"*",                    11, 0},
	{"/",                    11, 0},
	{"%",                    11, 0},
	{"+",                    10, 0},
	{"-",                    10, 0},
	{">>",                   9, 0},
	{"<<",                   9, 0},
	{"<",                    8, 0},
	{"<=",                   8, 0},
	{">",                    8, 0},
	{">=",                   8, 0},
	{"==",                   7, 0},
	{"!=",                   7, 0},
	{"&",                    6, 0},
	{"|",                    5, 0},
	{"^",                    4, 0},
	{"&&",                   3, 0},
	{"||",                   2, 0},
	{"..",                   1, 0},
};



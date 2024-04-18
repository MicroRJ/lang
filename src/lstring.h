/*
** See Copyright Notice In lang.h
** lstring.h
** String lObject and String Tools
*/


typedef struct lString {
	lObject  obj;
	lhashid   hash;
	int     length;
	union {
		char   string[1];
		char   c[1];
	};
} lString;


lString *langS_new2(lRuntime *fs, llongint length);
lString *langS_new(lRuntime *fs, char const *contents);
lTable *langS_newclass(lRuntime *R);


int langS_length_(lRuntime *c);
int langS_match_(lRuntime *c);
int langS_hash_(lRuntime *c);


lbool S_match(char *p, char *s);
char *S_copy(Alloc *cator, char const *contents);
int S_length(char const *contents);
lbool S_eq(char const *x, char const *y);
unsigned int S_hashcontents (char const *contents, unsigned int length);


#define S_tpf(format,...) (LCHECKPRINTF(format,__VA_ARGS__),S_tpf_(format,__VA_ARGS__))

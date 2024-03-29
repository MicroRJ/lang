/*
** See Copyright Notice In lang.h
** lstring.h
** String Object and String Tools
*/


typedef struct String {
	Object  obj;
	lhash   hash;
	int     length;
	union {
		char   string[1];
		char   c[1];
	};
} String;


String *langS_new(Runtime *fs, char const *contents);


int langS_length_(Runtime *c);
int langS_match_(Runtime *c);


lbool S_match(char *p, char *s);
char *S_copy(Alloc *cator, char const *contents);
int S_length(char const *contents);
lbool S_eq(char const *x, char const *y);
unsigned int S_hashcontents (char const *contents, unsigned int length);


#define S_tpf(format,...) (LCHECKPRINTF(format,__VA_ARGS__),S_tpf_(format,__VA_ARGS__))

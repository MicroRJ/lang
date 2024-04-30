/*
** See Copyright Notice In elf.h
** elf-str.h
** String
*/


typedef struct elf_String {
	elf_Object  obj;
	elf_hashint   hash;
	int     length;
	union {
		char   string[1];
		char   c[1];
	};
} elf_String;


elf_String *elf_newstrlen(elf_Runtime *R, elf_int length);
elf_String *elf_newstr(elf_Runtime *R, char *contents);
elf_Table *elf_newstrmetatab(elf_Runtime *R);


int langS_length_(elf_Runtime *c);
int langS_match_(elf_Runtime *c);
int langS_append_(elf_Runtime *R);
int langS_hash_(elf_Runtime *c);


elf_bool S_match(char *p, char *s);
char *S_copy(Alloc *cator, char const *contents);
int S_length(char const *contents);
elf_bool S_eq(char const *x, char const *y);
unsigned int S_hashcontents (char const *contents, unsigned int length);


#define elf_tpf(format,...) (LCHECKPRINTF(format,__VA_ARGS__),S_tpf_(format,__VA_ARGS__))

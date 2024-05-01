/*
** See Copyright Notice In elf.h
** elf-obj.h
** Objects And Values
*/


typedef enum elf_gccolor {
	GC_BLACK = 0, GC_WHITE, GC_PINK, GC_RED,
} elf_gccolor;



/* first object tag must be OBJECT, all other
objects come after it */
#define TAGLIST(_) \
_(NIL) _(GCD) _(SYS) \
_(INT) _(NUM) _(BID) \
_(OBJ) _(CLS) _(STR) _(TAB) /* end */



typedef enum elf_objty {
	OBJ_NONE = 0,
	OBJ_CLOSURE,
	OBJ_STRING,
	OBJ_ARRAY,
	OBJ_TAB,
	OBJ_CUSTOM,
} elf_objty;


typedef struct elf_Object {
#if defined(_DEBUG)
	int headtrap;
#endif
	elf_objty type;
	elf_gccolor gccolor;
	elf_int tell;
	elf_Table *metatable;
#if defined(_DEBUG)
	int tailtrap;
#endif
} elf_Object;



#define TAGENUM(NAME) XFUSE(TAG_,NAME),
typedef enum elf_valtag {
	TAGLIST(TAGENUM)
} elf_valtag;
#undef TAGENUM


#define TAGENUM(NAME) XSTRINGIFY(NAME),
elf_globaldecl char const *tag2s[] = {
	TAGLIST(TAGENUM)
};
#undef TAGENUM


typedef struct elf_val {
	elf_valtag tag;
	union {
		Ptr           p;
		elf_Handle    h;
		lBinding      c;
		elf_int   	  i,x_int;
		elf_num   	  n,x_num;
		elf_Closure  *f,*x_cls;
		elf_Object   *j,*x_obj;
		elf_Table    *t,*x_tab;
		elf_String   *s,*x_str;
	};
} elf_val;


typedef struct elf_Closure {
	elf_Object obj;
   /* I guess one of the things we could do
   if we ever get to having multi-byte encoding,
   is encode the entire prototype in the
   instruction stream since most closures are
   anonymous, given how the language works.
   If not, then there's no need to store the
   whole prototype here, we can instead store an
   index into the proto table. */
	elf_Proto   fn;
   /* current instruction index (coroutines) */
	int       j;
   /* allocated past this point */
	elf_val caches[1];
} elf_Closure;


elf_api elf_val elf_valtab(elf_Table *);
elf_api elf_val elf_valbid(lBinding);
elf_api elf_val elf_valstr(elf_String *);
elf_api elf_val elf_valcls(elf_Closure *);
elf_api elf_val elf_valint(elf_int i);
elf_api elf_val elf_valnum(elf_num n);




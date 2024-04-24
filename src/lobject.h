/*
** See Copyright Notice In lang.h
** lobject.h
** Objects And Values
*/


typedef struct elf_str elf_str;
typedef struct elf_obj elf_obj;
typedef struct elf_Closure elf_Closure;
typedef struct elf_val elf_val;


typedef enum GCColor {
	GC_WHITE = 0,
	GC_BLACK = 1,
	GC_PINK  = 2,
	GC_RED   = 3,
} GCColor;


typedef enum ObjectType {
	OBJ_NONE = 0,
	OBJ_CLOSURE,
	OBJ_STRING,
	OBJ_ARRAY,
	OBJ_TAB,
	OBJ_CUSTOM,
} ObjectType;


typedef struct MetaFunc {
	char    *name;
	lBinding    c;
} MetaFunc;


typedef struct elf_obj {
	ObjectType type;
	GCColor gccolor;
	elf_tab *metatable;
} elf_obj;


/* first object tag must be OBJECT, all other
objects come after it */
#define TAGLIST(_) \
_(NIL) _(SYS) \
_(INT) _(NUM) _(BID) \
_(OBJ) _(CLS) _(STR) _(TAB) /* end */


#define TAGENUM(NAME) XFUSE(TAG_,NAME),
typedef enum lvaluetag {
	TAGLIST(TAGENUM)
} lvaluetag;
#undef TAGENUM


#define TAGENUM(NAME) XSTRINGIFY(NAME),
lglobaldecl char const *tag2s[] = {
	TAGLIST(TAGENUM)
};
#undef TAGENUM



typedef struct elf_val {
	lvaluetag tag;
	union {
		Ptr       p;
		elf_Handle    h;
		elf_int     i;
		elf_num   n;
		lBinding   c;
		elf_obj   *j,*x_obj;
		elf_tab    *t,*x_tab;
		elf_str   *s;
		elf_Closure  *f;
	};
} elf_val;


typedef struct elf_Closure {
	elf_obj obj;
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


lapi elf_val lang_T(elf_tab *t);
lapi elf_val lang_C(lBinding c);
lapi elf_val lang_S(elf_str *s);
lapi elf_val lang_F(elf_Closure *f);
lapi elf_val lang_I(elf_int i);
lapi elf_val lang_N(elf_num n);



elf_bool ttisnumeric(lvaluetag tag) {
	return tag == TAG_NUM || tag == TAG_INT;
}


elf_bool elf_tagisobj(lvaluetag tag) {
	switch (tag) {
		case TAG_STR: case TAG_TAB:
		case TAG_OBJ: case TAG_CLS: {
			return ltrue;
		}
	}
	return lfalse;
}


lvaluetag elf_tttotag(ObjectType type) {
	switch(type) {
		case OBJ_CLOSURE: return TAG_CLS;
		case OBJ_TAB: return TAG_TAB;
		case OBJ_STRING: return TAG_STR;
	}
	LNOBRANCH;
	return -1;
}


int tisnil(elf_val x) {
	return (x.tag == TAG_NIL) || (!ttisnumeric(x.tag) && (x.p == lnil));
}



lapi elf_val lang_T(elf_tab *t) {
	elf_val v = (elf_val){TAG_TAB};
	v.t = t;
	return v;
}


lapi elf_val lang_C(lBinding c) {
	elf_val v = (elf_val){TAG_BID};
	v.c = c;
	return v;
}


lapi elf_val lang_H(elf_Handle h) {
	elf_val v = (elf_val){TAG_SYS};
	v.h = h;
	return v;
}


lapi elf_val lang_S(elf_str *s) {
	elf_val v = (elf_val){TAG_STR};
	v.s = s;
	return v;
}


lapi elf_val lang_F(elf_Closure *f) {
	elf_val v = (elf_val){TAG_CLS};
	v.f = f;
	return v;
}


lapi elf_val lang_I(elf_int i) {
	elf_val v = (elf_val){TAG_INT};
	v.i = i;
	return v;
}


lapi elf_val lang_N(elf_num n) {
	elf_val v = (elf_val){TAG_NUM};
	v.n = n;
	return v;
}




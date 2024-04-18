/*
** See Copyright Notice In lang.h
** lobject.h
** Objects And Values
*/


typedef struct lString lString;
typedef struct lObject lObject;
typedef struct lClosure lClosure;
typedef struct lValue lValue;


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
	OBJ_TABLE,
	OBJ_CUSTOM,
} ObjectType;


typedef struct MetaFunc {
	char    *name;
	lBinding    c;
} MetaFunc;


typedef struct lObject {
	ObjectType type;
	GCColor gccolor;
	lTable *metaclass;
} lObject;


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



typedef struct lValue {
	lvaluetag tag;
	union {
		Ptr       p;
		lsysobj    h;
		llongint     i;
		lnumber   n;
		lBinding   c;
		lObject   *j;
		lTable     *t;
		lString   *s;
		lClosure  *f;
	};
} lValue;


typedef struct lClosure {
	lObject obj;
   /* I guess one of the things we could do
   if we ever get to having multi-byte encoding,
   is encode the entire prototype in the
   instruction stream since most closures are
   anonymous, given how the language works.
   If not, then there's no need to store the
   whole prototype here, we can instead store an
   index into the proto table. */
	lProto   fn;
   /* current instruction index (coroutines) */
	int       j;
   /* allocated past this point */
	lValue caches[1];
} lClosure;


lapi lValue lang_T(lTable *t);
lapi lValue lang_C(lBinding c);
lapi lValue lang_S(lString *s);
lapi lValue lang_F(lClosure *f);
lapi lValue lang_I(llongint i);
lapi lValue lang_N(lnumber n);



lbool ttisnumeric(lvaluetag tag) {
	return tag == TAG_NUM || tag == TAG_INT;
}


lbool ttisobj(lvaluetag tag) {
	switch (tag) {
		case TAG_STR: case TAG_TAB:
		case TAG_OBJ: case TAG_CLS: {
			return ltrue;
		}
	}
	return lfalse;
}


lvaluetag ttobj2val(ObjectType type) {
	switch(type) {
		case OBJ_CLOSURE: return TAG_CLS;
		case OBJ_TABLE: return TAG_TAB;
		case OBJ_STRING: return TAG_STR;
	}
	LNOBRANCH;
	return -1;
}


int tisnil(lValue x) {
	return (x.tag == TAG_NIL) || (!ttisnumeric(x.tag) && (x.p == lnil));
}



lapi lValue lang_T(lTable *t) {
	lValue v = (lValue){TAG_TAB};
	v.t = t;
	return v;
}


lapi lValue lang_C(lBinding c) {
	lValue v = (lValue){TAG_BID};
	v.c = c;
	return v;
}


lapi lValue lang_H(lsysobj h) {
	lValue v = (lValue){TAG_SYS};
	v.h = h;
	return v;
}


lapi lValue lang_S(lString *s) {
	lValue v = (lValue){TAG_STR};
	v.s = s;
	return v;
}


lapi lValue lang_F(lClosure *f) {
	lValue v = (lValue){TAG_CLS};
	v.f = f;
	return v;
}


lapi lValue lang_I(llongint i) {
	lValue v = (lValue){TAG_INT};
	v.i = i;
	return v;
}


lapi lValue lang_N(lnumber n) {
	lValue v = (lValue){TAG_NUM};
	v.n = n;
	return v;
}




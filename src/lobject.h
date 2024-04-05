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
	/* todo: this should only be for custom objects */
	int       _n;
	MetaFunc *_m;
} lObject;


typedef enum lvaluetag {
	VALUE_NONE,
	TAG_INTEGER,
	TAG_NUMBER,
	VALUE_BINDING,
	VALUE_CUSTOM,
	VALUE_HANDLE,
	TAG_CLOSURE,
	// VALUE_JITFUNC,
	VALUE_STRING,
	VALUE_ARRAY,
	TAG_TABLE,
} lvaluetag;


typedef struct lValue {
	lvaluetag tag;
	union {
		Ptr       p;
		Handle    h;
		llongint     i;
		lnumber   n;
		lBinding   c;
		lObject   *j;
		Table     *t;
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


lapi lValue lang_T(Table *t);
lapi lValue lang_C(lBinding c);
lapi lValue lang_S(lString *s);
lapi lValue lang_F(lClosure *f);
lapi lValue lang_I(llongint i);
lapi lValue lang_N(lnumber n);




lapi lValue lang_T(Table *t) {
	lValue v = (lValue){TAG_TABLE};
	v.t = t;
	return v;
}


lapi lValue lang_C(lBinding c) {
	lValue v = (lValue){VALUE_BINDING};
	v.c = c;
	return v;
}


lapi lValue lang_H(Handle h) {
	lValue v = (lValue){VALUE_HANDLE};
	v.h = h;
	return v;
}


lapi lValue lang_S(lString *s) {
	lValue v = (lValue){VALUE_STRING};
	v.s = s;
	return v;
}


lapi lValue lang_F(lClosure *f) {
	lValue v = (lValue){TAG_CLOSURE};
	v.f = f;
	return v;
}


lapi lValue lang_I(llongint i) {
	lValue v = (lValue){TAG_INTEGER};
	v.i = i;
	return v;
}


lapi lValue lang_N(lnumber n) {
	lValue v = (lValue){TAG_NUMBER};
	v.n = n;
	return v;
}




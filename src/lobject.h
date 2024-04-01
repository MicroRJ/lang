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


typedef enum ValueName {
	VALUE_NONE,
	VALUE_LONG,
	VALUE_REAL,
	VALUE_BINDING,
	VALUE_CUSTOM,
	VALUE_HANDLE,
	VALUE_FUNC,
	VALUE_STRING,
	VALUE_ARRAY,
	VALUE_TABLE,
} ValueName;


typedef struct lValue {
	ValueName tag;
	union {
		Ptr       p;
		Handle    h;
		llong     i;
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
	/* todo: we don't need to store the whole thing here */
	Proto   fn;
	/* allocated past this point */
	lValue caches[1];
} lClosure;


lapi lValue lang_T(Table *t);
lapi lValue lang_C(lBinding c);
lapi lValue lang_S(lString *s);
lapi lValue lang_F(lClosure *f);
lapi lValue lang_I(llong i);
lapi lValue lang_N(lnumber n);




lapi lValue lang_T(Table *t) {
	lValue v = (lValue){VALUE_TABLE};
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
	lValue v = (lValue){VALUE_FUNC};
	v.f = f;
	return v;
}


lapi lValue lang_I(llong i) {
	lValue v = (lValue){VALUE_LONG};
	v.i = i;
	return v;
}


lapi lValue lang_N(lnumber n) {
	lValue v = (lValue){VALUE_REAL};
	v.n = n;
	return v;
}




/*
** See Copyright Notice In lang.h
** lobject.h
** Objects And Values
*/


typedef struct String String;
typedef struct Object Object;
typedef struct Value Value;
typedef struct Closure Closure;


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


typedef int (* Binding)(Runtime *);


typedef struct MetaFunc {
	char *name;
	Binding    c;
} MetaFunc;


typedef struct Object {
	ObjectType type;
	GCColor gccolor;
	/* todo: this should only be for custom objects */
	int       _n;
	MetaFunc *_m;
} Object;


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


typedef struct Value {
	ValueName tag;
	union {
		Ptr      p;
		Handle   h;
		llong  i;
		lnumber   n;
		Binding    c;
		Object  *j;
		Table   *t;
		String  *s;
		Closure *f;
	};
} Value;


typedef struct Closure {
	Object obj;
	/* todo: we don't need to store the whole
	thing here */
	Proto   fn;
	/* allocated past this point */
	Value caches[1];
} Closure;


lapi Value lang_T(Table *t);
lapi Value lang_C(Binding c);
lapi Value lang_S(String *s);
lapi Value lang_F(Closure *f);
lapi Value lang_I(llong i);
lapi Value lang_N(lnumber n);




lapi Value lang_T(Table *t) {
	Value v = (Value){VALUE_TABLE};
	v.t = t;
	return v;
}


lapi Value lang_C(Binding c) {
	Value v = (Value){VALUE_BINDING};
	v.c = c;
	return v;
}


lapi Value lang_H(Handle h) {
	Value v = (Value){VALUE_HANDLE};
	v.h = h;
	return v;
}


lapi Value lang_S(String *s) {
	Value v = (Value){VALUE_STRING};
	v.s = s;
	return v;
}


lapi Value lang_F(Closure *f) {
	Value v = (Value){VALUE_FUNC};
	v.f = f;
	return v;
}


lapi Value lang_I(llong i) {
	Value v = (Value){VALUE_LONG};
	v.i = i;
	return v;
}


lapi Value lang_N(lnumber n) {
	Value v = (Value){VALUE_REAL};
	v.n = n;
	return v;
}




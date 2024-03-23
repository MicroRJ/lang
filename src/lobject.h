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


typedef int (* CFunc)(Runtime *);


typedef struct MetaFunc {
	char *name;
	CFunc    c;
} MetaFunc;


typedef struct Object {
	ObjectType type;
	GCColor gccolor;
	char *debugname;
	char *debugline;
	int       _n;
	MetaFunc *_m;
} Object;


typedef enum ValueName {
	VALUE_NONE,
	VALUE_INTEGER,
	VALUE_NUMBER,
	VALUE_CFUN,
	VALUE_CUSTOM,
	VALUE_HANDLE,
	VALUE_FUNC,
	VALUE_STRING,
	VALUE_ARRAY,
	VALUE_TABLE,
} ValueName;
/*
ValueName\NONE:
	Can be interpreted as nil

ValueName\NUMBER:
	Floating point number

ValueName\CUSTOM:
	Custom Object

ValueName\TABLE:
	Builtin Table object

ValueName\FUNC:
	Builtin Closure object

ValueName\STRING:
	Builtin String object
*/


typedef struct Value {
	ValueName tag;
	union {
		Ptr      p;
		Integer  i;
		Number   n;
		CFunc    c;
		Object  *j;
		Table   *t;
		String  *s;
		Closure *f;
	};
} Value;


typedef struct Closure {
	Object obj;
	Proto   fn;
	/* allocated past this point */
	Value caches[1];
} Closure;


LAPI Value lang_T(Table *t);
LAPI Value lang_C(CFunc c);
LAPI Value lang_S(String *s);
LAPI Value lang_F(Closure *f);
LAPI Value lang_I(Integer i);
LAPI Value lang_N(Number n);


/*
** See Copyright Notice In lang.h
** lbyte.h
** Bytecodes
*/


typedef int lbyteid;


typedef enum ByteName {

	BYTE_J,
	BYTE_JZ,
	BYTE_JNZ,
	/* todo: implement? */
	BYTE_JNIL,

	BYTE_YIELD,
	BYTE_LEAVE,

	BYTE_DROP,
	BYTE_DUPL,
	BYTE_NUM,
	BYTE_INT,
	BYTE_NIL,

	BYTE_LOADFILE,
	BYTE_LOADCLIB,

	BYTE_CALL,
	BYTE_METACALL,

	BYTE_GLOBAL,
	BYTE_LOCAL,
	BYTE_CACHE,
	BYTE_INDEX,
	BYTE_FIELD,

	BYTE_SETGLOBAL,
	BYTE_SETLOCAL,
	BYTE_SETINDEX,
	BYTE_SETFIELD,

	BYTE_CLOSURE,
	BYTE_TABLE,

	BYTE_NEQ,
	BYTE_EQ,
	BYTE_LT,
	BYTE_LTEQ,
	BYTE_ISNIL,

	BYTE_ADD,
	BYTE_SUB,
	BYTE_DIV,
	BYTE_MUL,
	BYTE_MOD,

	BYTE_SHL,
	BYTE_SHR,
	BYTE_XOR,
} ByteName;


/* todo: eventually this will be made more
compact, since this language is meant be
simple, for teaching, and fast prototyping,
I'm not worrying too much about it...
though it would help performance quite a bit... */
typedef struct Bytecode {
	ByteName k;
	union {
		llong  i;
		struct {
			int x,y;
		};
	};
} Bytecode;



char const *lang_bytename(ByteName k) {
	switch (k) {
		case BYTE_LEAVE: return "leave";
		case BYTE_YIELD: return "yield";
		case BYTE_J: return "j";
		case BYTE_JZ: return "jz";
		case BYTE_JNZ: return "jnz";
		case BYTE_ISNIL: return "z";
		case BYTE_DROP: return "pop";
		case BYTE_DUPL: return "dup";
		case BYTE_INT: return "i";
		case BYTE_NIL: return "nil";
		case BYTE_CALL: return "()";
		case BYTE_METACALL: return "M()";
		case BYTE_CLOSURE: return "f+";
		case BYTE_CACHE: return "u.";
		case BYTE_GLOBAL: return "g.";
		case BYTE_SETGLOBAL: return "g=";
		case BYTE_LOCAL: return "l.";
		case BYTE_SETLOCAL: return "l=";
		case BYTE_TABLE: return "t+";
		case BYTE_INDEX: return "[";
		case BYTE_FIELD: return ".";
		case BYTE_SETFIELD: return ".=";
		case BYTE_SETINDEX: return "[=";
		case BYTE_ADD: return "+";
		case BYTE_SUB: return "-";
		case BYTE_DIV: return "/";
		case BYTE_MUL: return "*";
		case BYTE_MOD: return "%";
		case BYTE_LT: return "<";
		case BYTE_NEQ: return "!=";
		case BYTE_EQ: return "==";
		case BYTE_LOADFILE: return "loadfile";
		case BYTE_LOADCLIB: return "loadclib";
	}
	LNOCHANCE;
	return 0;
}

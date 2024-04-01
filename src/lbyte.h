/*
** See Copyright Notice In lang.h
** lbyte.h
** Bytecodes
*/




typedef enum ByteName {

	BYTE_J,
	BYTE_JZ,
	BYTE_JNZ,

	/* delays the execution n instructions until
	procedure exits, jumps to the specified byte
	address. */
	BYTE_DELAY,
	/* checks delay list, pops the last delay from it
	if any and jumps to it, otherwise returns control
	flow to the calling procedure */
	BYTE_LEAVE,
	/* copies specified values to corresponding return
	registers */
	BYTE_YIELD,

	BYTE_DROP,
	BYTE_DUPL,
	BYTE_NUM,
	BYTE_INT,
	BYTE_NIL,

	BYTE_LOADFILE,
	BYTE_LOADCLIB,

	BYTE_CALL,
	BYTE_METACALL,

	BYTE_STKGET,
	BYTE_STKLEN,

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
		case BYTE_STKGET: return "stkget";
		case BYTE_STKLEN: return "stklen";
		case BYTE_LEAVE: return "leave";
		case BYTE_YIELD: return "yield";
		case BYTE_J: return "j";
		case BYTE_JZ: return "jz";
		case BYTE_JNZ: return "jnz";
		case BYTE_ISNIL: return "isnil";
		case BYTE_DROP: return "drop";
		case BYTE_DUPL: return "dupl";
		case BYTE_INT: return "int";
		case BYTE_NIL: return "nil";
		case BYTE_CALL: return "call";
		case BYTE_METACALL: return "metacall";
		case BYTE_CACHE: return "getcache";
		case BYTE_GLOBAL: return "getglobal";
		case BYTE_LOCAL: return "getlocal";
		case BYTE_SETGLOBAL: return "setglobal";
		case BYTE_SETLOCAL: return "setlocal";
		case BYTE_CLOSURE: return "newclosure";
		case BYTE_TABLE: return "newtable";
		case BYTE_INDEX: return "getindex";
		case BYTE_FIELD: return "getfield";
		case BYTE_SETFIELD: return "setfield";
		case BYTE_SETINDEX: return "setindex";
		case BYTE_ADD: return "add";
		case BYTE_SUB: return "sub";
		case BYTE_DIV: return "dib";
		case BYTE_MUL: return "mul";
		case BYTE_MOD: return "mod";
		case BYTE_LT: return "lt";
		case BYTE_NEQ: return "neq";
		case BYTE_EQ: return "eq";
		case BYTE_DELAY: return "delay";
		case BYTE_LOADFILE: return "loadfile";
		case BYTE_LOADCLIB: return "loadclib";
	}
	LNOCHANCE;
	return 0;
}

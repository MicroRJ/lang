/*
** See Copyright Notice In lang.h
** lbyte.h
** Bytecodes
*/


typedef enum lbyteclass {
	BC_CLASS_I,
	BC_CLASS_XY,
	BC_CLASS_XYZ,
} lbyteclass;


typedef enum lbyteop {
	BC_HALT = 0,
	BC_J,
	/* counter pairs, must start at even value, ^1 to get opposite */
	BC_JZ, BC_JNZ,
	BC_JE, BC_JNE,
	/* delays the execution of \i instructions until
	procedure exits by jumping to the specified byte.
	the byte address of the following instruction
	gets saved into the delay list. */
	BC_DELAY/* (i) */,
	BC_LOOPER/* (x,y,z) */,
	/* checks delay list, pops the last delay from it
	if any and jumps to it, otherwise returns control
	flow to the calling procedure */
	BC_LEAVE,
	/* copies z values starting at y to corresponding return
	registers, jumps to x */
	BC_YIELD/*(x,y,z)*/,

	BC_TYPEGUARD,

	BC_LOADGLOBAL,
	BC_LOADNUM, BC_LOADINT, BC_LOADNIL,

	BC_LOADFILE, BC_LOADCLIB,
	/* BC_CALL(io x,y,z):
	b.x = function address and first return value
	b.y = number of inputs
	b.z = number of outputs
	-- function is located at b.x and arguments are located below
	b.x, result is to be placed at b.x through b.y-1, so the user
	should have allocated sufficient space for both the arguments
	and the returns. */
	BC_CALL,
	BC_METANAME,
	BC_METACALL,
	BC_TABLECALL,
	BC_STKGET, BC_STKLEN,
	BC_RELOAD,
	BC_LOADCACHED,
	BC_INDEX,
	BC_FIELD,
	BC_SETGLOBAL,
	BC_SETINDEX,
	BC_SETFIELD,
	BC_TABLE, BC_CLOSURE,
	BC_ISNIL,
	BC_EQ, BC_NEQ,
	BC_LT, BC_LTEQ,
	BC_MUL, BC_DIV, BC_MOD,
	BC_ADD, BC_SUB,
	BC_SHL, BC_SHR, BC_XOR,
} lbyteop;


/* -- todo: eventually this will be made more
compact, since this language is meant be
simple, for teaching, and fast prototyping,
I'm not worrying too much about it...
though it would help performance quite a bit... */
typedef struct lBytecode {
	lbyteop k;
	union {
		llongint  i;
		struct {
			int x,y,z;
		};
	};
} lBytecode;


lbyteclass lang_byteclass(lbyteop k) {
	switch (k) {
		case BC_JZ:
		case BC_JNZ:
		case BC_TYPEGUARD:
		case BC_SETGLOBAL:
		case BC_STKGET:
		case BC_LOADNUM:
		case BC_LOADINT:
		case BC_LOADNIL:
		case BC_LOADGLOBAL:
		case BC_LOADCACHED:
		case BC_RELOAD: {
			return BC_CLASS_XY;
		}
		case BC_CLOSURE:
		case BC_FIELD:
		case BC_SETFIELD: case BC_SETINDEX:
		case BC_INDEX:
		case BC_ISNIL:
		case BC_NEQ: case BC_EQ:
		case BC_LT: case BC_LTEQ:
		case BC_ADD: case BC_SUB:
		case BC_DIV: case BC_MUL: case BC_MOD:
		case BC_SHL: case BC_SHR:
		case BC_XOR:
		case BC_CALL:
		case BC_YIELD:
		case BC_METANAME:
		case BC_METACALL:
		case BC_LOADFILE: {
			return BC_CLASS_XYZ;
		}
	}
	return BC_CLASS_I;
}


char const *lang_bytename(lbyteop k) {
	switch (k) {
		case BC_LOADNUM: return "loadnum";
		case BC_LOADINT: return "loadint";
		case BC_LOADNIL: return "loadnil";
		case BC_STKGET: return "stkget";
		case BC_STKLEN: return "stklen";
		case BC_LEAVE: return "leave";
		case BC_YIELD: return "yield";
		case BC_J: return "j";
		case BC_JZ: return "jz";
		case BC_JNZ: return "jnz";
		case BC_ISNIL: return "isnil";
		case BC_CALL: return "call";
		case BC_METANAME: return "metaname";
		case BC_METACALL: return "metacall";
		case BC_LOADCACHED: return "loadcached";
		case BC_LOADGLOBAL: return "loadglobal";
		case BC_RELOAD: return "reload";
		case BC_SETGLOBAL: return "setglobal";
		case BC_CLOSURE: return "newclosure";
		case BC_TABLE: return "newtable";
		case BC_INDEX: return "getindex";
		case BC_FIELD: return "getfield";
		case BC_SETFIELD: return "setfield";
		case BC_SETINDEX: return "setindex";
		case BC_TYPEGUARD: return "typeguard";
		case BC_ADD: return "add";
		case BC_SUB: return "sub";
		case BC_DIV: return "dib";
		case BC_MUL: return "mul";
		case BC_MOD: return "mod";
		case BC_LT: return "lt";
		case BC_LTEQ: return "lteq";
		case BC_NEQ: return "neq";
		case BC_EQ: return "eq";
		case BC_DELAY: return "delay";
		case BC_LOADFILE: return "loadfile";
		case BC_LOADCLIB: return "loadclib";

		case BC_SHL: return "shl";
		case BC_SHR: return "shr";
		case BC_XOR: return "xor";
	}
	return "???";
}


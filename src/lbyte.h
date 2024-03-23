/*
** See Copyright Notice In lang.h
** lbyte.h
** Bytecodes
*/


typedef int ByteId;


typedef enum ByteName {
	BYTE_J,
	BYTE_JZ,
	BYTE_JNZ,
	BYTE_POP,
	BYTE_DUP,
	BYTE_RET,

	BYTE_LOADFILE,

	BYTE_INT,

	BYTE_CALL,
	BYTE_MCALL,

	BYTE_FNEW,

	BYTE_GGET,
	BYTE_GSET,

	BYTE_LGET,
	BYTE_LSET,

	BYTE_UGET,

	BYTE_TNEW,
	BYTE_TGET,
	BYTE_TSET,

	BYTE_ADD  = TK_ADD,
	BYTE_SUB  = TK_SUB,
	BYTE_DIV  = TK_DIV,
	BYTE_MUL  = TK_MUL,
	BYTE_MOD  = TK_MODULUS,
	BYTE_NEQ  = TK_NOT_EQUALS,
	BYTE_EQ   = TK_EQUALS,
	BYTE_LT   = TK_LESS_THAN,
} ByteName;


typedef struct Bytecode {
	ByteName k;
	/* todo: */
	Integer  i;
} Bytecode;


char const *lang_bytename(ByteName k) {
	switch (k) {
		case BYTE_RET: return "ret";
		case BYTE_J: return "j";
		case BYTE_JZ: return "jz";
		case BYTE_JNZ: return "jnz";
		case BYTE_POP: return "pop";
		case BYTE_DUP: return "dup";
		case BYTE_INT: return "i";
		case BYTE_CALL: return "()";
		case BYTE_MCALL: return "M()";
		case BYTE_FNEW: return "f+";
		case BYTE_UGET: return "u.";
		case BYTE_GGET: return "g.";
		case BYTE_GSET: return "g=";
		case BYTE_LGET: return "l.";
		case BYTE_LSET: return "l=";
		case BYTE_TNEW: return "t+";
		case BYTE_TGET: return "t.";
		case BYTE_TSET: return "t=";
		case BYTE_ADD: return "+";
		case BYTE_SUB: return "-";
		case BYTE_DIV: return "/";
		case BYTE_MUL: return "*";
		case BYTE_MOD: return "%";
		case BYTE_LT: return "<";
		case BYTE_NEQ: return "!=";
		case BYTE_EQ: return "==";
		case BYTE_LOADFILE: return "loadfile";
	}
	LNOCHANCE;
	return 0;
}

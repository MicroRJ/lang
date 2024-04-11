/*
** See Copyright Notice In lang.h
** (Y) lnode.h
** IR?...
*/


#define NO_NODE (-1)


typedef int lnodeid;


typedef enum lnodety {
	NT_ANY = 0,
	NT_NIL, NT_BOL, NT_INT, NT_NUM,
	NT_TAB, NT_FUN, NT_STR
} lnodety;

typedef enum lnodeop {
	Y_NONE = 0,
	Y_NOP  = 1,
	/* -- 4.9.24 ------------------------------------
	the following are set up in counterpart pairs,
	this enables us to easily derive their opposite
	operation by simply applying the (^1) operation.
	must be even odd pairs and must start on an even
	value - learned this trick from Mike Pall @ LuaJIT */
	Y_LOG_AND, Y_LOG_OR,
	Y_EQ, Y_NEQ,
	Y_BSHL, Y_BSHR,
	Y_ADD, Y_SUB,
	Y_MUL, Y_DIV,
	Y_LT, Y_GT,
	Y_LTEQ, Y_GTEQ,

	Y_BXOR, Y_MOD,

	NODE_LOAD,

	Y_LOADFILE,
	/* constant values, closure points to prototype
	in proto table in module, all other constants
	are embedded in the node and the generator decides
	how to emit the instruction. */
	N_FUN, Y_STRING, N_TABLE,
	Y_INTEGER, Y_NUMBER, N_NIL,
	/* l-values, represent some address,
	source is x */
	Y_GLOBAL, N_LOCAL, N_CACHE,

	N_INDEX,// {x}[{x}]
	N_FIELD,// {x}.{x}
	Y_CALL,// {x}({x})
	Y_MCALL,// {x}:{x}({x})
	/* some builtin instruction node, x is the
	builtin id, (the token type) and z the
	inputs */
	Y_BUILTIN,// ID({x})

	Y_MCALL_CLONE,
	Y_MCALL_SPLIT,
	Y_MCALL_MATCH,
	Y_MCALL_REPLACE,
	Y_MCALL_INDEXOF,
	Y_MCALL_LENGTH,
	Y_MCALL_INSERT,
	Y_MCALL_LOOKUP,

	/* psuedo nodes */
	Y_RANGE_INDEX,// [{x}..{x}]
	Y_RANGE,// {x}..{x}
	Y_GROUP,// ({x})
} lnodeop;


typedef struct lNode {
	lnodeop k;
	lnodety t;
	llineid line;
	/* todo: eventually remove this */
	int level;

	/* x,y,z represent node inputs, if more than 2
	are required, use z, this could change in the
	future, we could use complementary nodes to
	append more inputs to a node... */
	struct {
		lnodeid x,y,*z;
	};
	/* todo?: don't quite union these two for debugging? */
	union {
		char const *s;
		llongint    i;
		lnumber     n;
	} lit;
} lNode;


lnodeid langN_xyz(FileState *fs, llineid, lnodeop k, lnodety t, lnodeid x, lnodeid y, lnodeid *z);
lnodeid langN_xy(FileState *fs, llineid, lnodeop k, lnodety t, lnodeid x, lnodeid y);
lnodeid langN_x(FileState *fs, llineid, lnodeop k, lnodety t, lnodeid x);
lnodeid lanN_node(FileState *fs, llineid, lnodeop k, lnodety t);

lnodeid langN_group(FileState *fs, llineid, lnodeid x);

lnodeid langN_longint(FileState *fs, llineid, llongint i);
lnodeid langN_number(FileState *fs, llineid, lnumber n);
lnodeid langN_string(FileState *fs, llineid, char *);
lnodeid langN_table(FileState *fs, llineid, lnodeid *z);
lnodeid langN_closure(FileState *fs, llineid, lnodeid x, lnodeid *z);
lnodeid langN_nil(FileState *fs, llineid);

lnodeid langN_load(FileState *fs, llineid line, lnodeid x, lnodeid y);

lnodeid langN_local(FileState *fs, llineid line, lnodeid i);
lnodeid langN_cache(FileState *fs, llineid line, lnodeid i);
lnodeid langN_global(FileState *fs, llineid line, lnodeid i);

lnodeid langN_field(FileState *fs, llineid line, lnodeid x, lnodeid y);
lnodeid langN_index(FileState *fs, llineid line, lnodeid x, lnodeid y);

lnodeid langN_loadfile(FileState *fs, llineid line, lnodeid x);

lnodeid langN_rangedindex(FileState *fs, llineid line, lnodeid x, lnodeid y);

lnodeid langN_builtincall(FileState *fs, llineid line, ltokentype k, lnodeid *z);
lnodeid langN_metacall(FileState *fs, llineid line, lnodeid x, lnodeid y, lnodeid *z);
lnodeid langN_call(FileState *fs, llineid line, lnodeid x, lnodeid *z);


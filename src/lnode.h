/*
** See Copyright Notice In lang.h
** (Y) lnode.h
** Simple AST/IR
*/


typedef int lnodeid;


#define NO_NODE (-1)

/*
** Nodes are mainly for storing expression like
** instructions temporarily and then generating
** bytecode from that, effectively a very
** simple intermediate representation.
** This step isn't necessary but it makes
** things easier to understand and tends to be
** a little bit more flexible when targetting
** other instruction sets.
** Nodes are stored in a linear buffer and
** deallocated naturally as they become
** inaccessible.
*/
typedef enum NodeType {
	NODE_NONE = 0,

	NODE_GROUP,

	NODE_FUNCTION,
	NODE_LOADFILE,
	NODE_STRING,
	NODE_TABLE,
	NODE_INTEGER,
	NODE_NUMBER,
	NODE_NIL,

	NODE_GLOBAL,
	NODE_LOCAL,
	NODE_CACHE,

	// [{x}..{x}]
	NODE_RANGE_INDEX,
	NODE_INDEX,
	// {x}.{x}
	NODE_FIELD,

	NODE_DESIG,

	// {x}({x})
	NODE_CALL,
	// {x}:({x})
	NODE_MCALL,
	/* Some builtin instruction node, x is the
	builtin id, (the token type) and z the
	inputs */
	NODE_BUILTIN,
	/* Common meta functions */
	NODE_MCALL_CLONE,
	NODE_MCALL_SPLIT,
	NODE_MCALL_MATCH,
	NODE_MCALL_REPLACE,
	NODE_MCALL_INDEXOF,
	NODE_MCALL_LENGTH,
	NODE_MCALL_INSERT,
	NODE_MCALL_LOOKUP,

	/* binary nodes use x,y */
	NODE_RANGE,
	NODE_LOG_AND,
	NODE_LOG_OR,
	NODE_ADD,
	NODE_SUB,
	NODE_DIV,
	NODE_MUL,
	NODE_MOD,
	NODE_NEQ,
	NODE_EQ,
	NODE_LT,
	NODE_LTEQ,
	NODE_GT,
	NODE_GTEQ,
	NODE_BSHL,
	NODE_BSHR,
	NODE_BXOR,
} NodeType;


typedef struct Node {
	NodeType k;
	char *line;
	int level;

	/* x,y,z represent node inputs, if more
	than 2 are required, use z */
	struct {
		lnodeid x,y,*z;
	};
	/* todo?: don't quite union these two for debugging? */
	union {
		char const *s;
		llong       i;
		lnumber     n;
	} lit;
} Node;


lnodeid langY_nodexyz(FileState *fs, char *, NodeType k, lnodeid x, lnodeid y, lnodeid *z);
lnodeid langY_nodexy(FileState *fs, char *, NodeType k, lnodeid x, lnodeid y);
lnodeid langY_nodex(FileState *fs, char *, NodeType k, lnodeid x);
lnodeid langY_node(FileState *fs, char *, NodeType k);

lnodeid langY_groupnode(FileState *fs, char *, lnodeid x);

/* The following nodes represent different literal
or constant values, I for int, N for number and
so on respectively. node-0 means nil. */
lnodeid langY_nodeI(FileState *fs, char *, llong i);
lnodeid langY_nodeN(FileState *fs, char *, lnumber n);
lnodeid langY_nodeS(FileState *fs, char *, char *);
lnodeid langY_nodeH(FileState *fs, char *, lnodeid *z);
lnodeid langY_nodeF(FileState *fs, char *, lnodeid x, lnodeid *z);
lnodeid langY_nodeZ(FileState *fs, char *);

lnodeid langY_localnode(FileState *fs, char *line, lnodeid i);
lnodeid langY_cachenode(FileState *fs, char *line, lnodeid i);
lnodeid langY_globalnode(FileState *fs, char *line, lnodeid i);

lnodeid langY_fieldnode(FileState *fs, char *line, lnodeid x, lnodeid y);
lnodeid langY_indexnode(FileState *fs, char *line, lnodeid x, lnodeid y);
lnodeid langY_designode(FileState *fs, char *line, lnodeid x, lnodeid y);

lnodeid langY_loadfilenode(FileState *fs, char *line, lnodeid x);

lnodeid langY_rangeindexnode(FileState *fs, char *line, lnodeid x, lnodeid y);

lnodeid langY_builtinnode(FileState *fs, char *line, ltokentype k, lnodeid *z);
lnodeid langY_metacallnode(FileState *fs, char *line, lnodeid x, lnodeid y, lnodeid *z);
lnodeid langY_callnode(FileState *fs, char *line, lnodeid x, lnodeid *z);


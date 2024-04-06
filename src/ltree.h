/*
** See Copyright Notice In lang.h
** (Y) ltree.h
** IR?...
*/


#define NO_TREE (-1)


typedef int ltreeid;


/* - Trees are mainly for storing expressions
- temporarily and then generating bytecode
- from that, effectively a very simple AST.
- This largely unnecessary for how simple
- this language is, period...
*/
typedef enum ltreetype {
	Y_NONE = -1,
	/* -- the following are set up in counterpart pairs.
	- this enables us to easily derive their opposite
	- operation by simply applying the (^1) operation.
	- must be even odd pairs and must start on an even value.
	- learned this trick from Mike Pall @ LuaJIT */
	Y_LOG_AND, Y_LOG_OR,
	Y_NEQ, Y_EQ,
	Y_BSHL, Y_BSHR,
	Y_ADD, Y_SUB,
	Y_MUL, Y_DIV,
	Y_LT, Y_GT,
	Y_LTEQ, Y_GTEQ,
	/* end */
	Y_BXOR, Y_MOD,
	Y_GROUP,

	Y_LOADFILE,
	Y_FUNCTION,
	Y_STRING,
	Y_TABLE,
	Y_INTEGER, Y_NUMBER,
	Y_NIL,

	Y_GLOBAL, Y_LOCAL, Y_CACHED,

	// [{x}..{x}]
	Y_RANGE_INDEX,
	Y_INDEX,
	// {x}.{x}
	Y_FIELD,

	Y_DESIG,

	// {x}({x})
	Y_CALL,
	// {x}:({x})
	Y_MCALL,
	/* Some builtin instruction node, x is the
	builtin id, (the token type) and z the
	inputs */
	Y_BUILTIN,

	Y_MCALL_CLONE,
	Y_MCALL_SPLIT,
	Y_MCALL_MATCH,
	Y_MCALL_REPLACE,
	Y_MCALL_INDEXOF,
	Y_MCALL_LENGTH,
	Y_MCALL_INSERT,
	Y_MCALL_LOOKUP,

	/* binary nodes use x,y, reason why did not use
	token here was because this will prob either go
	away or be reused, in either case, I want this to
	be as modular as possible, also I did think about
	assigning these to tokens, but value clashing
	is annoying. */
	Y_RANGE,


} ltreetype;


typedef struct Tree {
	ltreetype k;
	char *line;
	/* todo: eventually remove this when
	blocks are added */
	int level;

	/* x,y,z represent node inputs, if more
	than 2 are required, use z */
	struct {
		ltreeid x,y,*z;
	};
	/* todo?: don't quite union these two for debugging? */
	union {
		char const *s;
		llongint    i;
		lnumber     n;
	} lit;
} Tree;


ltreeid langY_treexyz(FileState *fs, llineid , ltreetype k, ltreeid x, ltreeid y, ltreeid *z);
ltreeid langY_treexy(FileState *fs, llineid , ltreetype k, ltreeid x, ltreeid y);
ltreeid langY_treex(FileState *fs, llineid , ltreetype k, ltreeid x);
ltreeid langY_tree(FileState *fs, llineid , ltreetype k);

ltreeid langY_treegroup(FileState *fs, llineid , ltreeid x);

/* The following nodes represent different literal
or constant values, I for int, N for number and
so on respectively. node-0 means nil. */
ltreeid langY_treelongint(FileState *fs, llineid , llongint i);
ltreeid langY_treenumber(FileState *fs, llineid , lnumber n);
ltreeid langY_treeString(FileState *fs, llineid , char *);
ltreeid langY_treetable(FileState *fs, llineid , ltreeid *z);
ltreeid langY_treeclosure(FileState *fs, llineid , ltreeid x, ltreeid *z);
ltreeid langY_treenil(FileState *fs, llineid );

ltreeid langY_treelocal(FileState *fs, llineid line, ltreeid i);
ltreeid langY_treecache(FileState *fs, llineid line, ltreeid i);
ltreeid langY_treeglobal(FileState *fs, llineid line, ltreeid i);

ltreeid langY_treefield(FileState *fs, llineid line, ltreeid x, ltreeid y);
ltreeid langY_treeindex(FileState *fs, llineid line, ltreeid x, ltreeid y);
ltreeid langY_treedesig(FileState *fs, llineid line, ltreeid x, ltreeid y);

ltreeid langY_treeloadfile(FileState *fs, llineid line, ltreeid x);

ltreeid langY_treerangedindex(FileState *fs, llineid line, ltreeid x, ltreeid y);

ltreeid langY_treebuiltincall(FileState *fs, llineid line, ltokentype k, ltreeid *z);
ltreeid langY_treemetacall(FileState *fs, llineid line, ltreeid x, ltreeid y, ltreeid *z);
ltreeid langY_treecall(FileState *fs, llineid line, ltreeid x, ltreeid *z);


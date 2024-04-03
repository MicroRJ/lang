/*
** See Copyright Notice In lang.h
** (Y) ltree.h
** Simple AST
*/


#define NO_TREE (-1)


typedef int ltreeid;


/* - Trees are mainly for storing expressions
- temporarily and then generating bytecode
- from that, effectively a very simple AST.
- This step isn't necessary but it makes
- things easier to understand and flexible.
- Trees are stored in a linear buffer and
- deallocated naturally as they become
- inaccessible.
*/
typedef enum ltreetype {
	TREE_NONE = 0,

	TREE_GROUP,

	TREE_FUNCTION,
	TREE_LOADFILE,
	TREE_STRING,
	TREE_TABLE,
	TREE_INTEGER,
	TREE_NUMBER,
	TREE_NIL,

	TREE_GLOBAL,
	TREE_LOCAL,
	TREE_CACHE,

	// [{x}..{x}]
	TREE_RANGE_INDEX,
	TREE_INDEX,
	// {x}.{x}
	TREE_FIELD,

	TREE_DESIG,

	// {x}({x})
	TREE_CALL,
	// {x}:({x})
	TREE_MCALL,
	/* Some builtin instruction node, x is the
	builtin id, (the token type) and z the
	inputs */
	TREE_BUILTIN,
	/* Common meta functions */
	TREE_MCALL_CLONE,
	TREE_MCALL_SPLIT,
	TREE_MCALL_MATCH,
	TREE_MCALL_REPLACE,
	TREE_MCALL_INDEXOF,
	TREE_MCALL_LENGTH,
	TREE_MCALL_INSERT,
	TREE_MCALL_LOOKUP,

	/* binary nodes use x,y, reason why did not use
	token here was because this will prob either go
	away or be reused, in either case, I want this to
	be as modular as possible, also I did think about
	assigning these to tokens, but value clashing
	is annoying. */
	TREE_RANGE,
	TREE_LOG_AND,
	TREE_LOG_OR,
	TREE_ADD,
	TREE_SUB,
	TREE_DIV,
	TREE_MUL,
	TREE_MOD,
	TREE_NEQ,
	TREE_EQ,
	TREE_LT,
	TREE_LTEQ,
	TREE_GT,
	TREE_GTEQ,
	TREE_BSHL,
	TREE_BSHR,
	TREE_BXOR,
} ltreetype;


typedef struct Tree {
	ltreetype k;
	char *line;
	int level;

	/* x,y,z represent node inputs, if more
	than 2 are required, use z */
	struct {
		ltreeid x,y,*z;
	};
	/* todo?: don't quite union these two for debugging? */
	union {
		char const *s;
		llongint       i;
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


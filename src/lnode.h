/*
** See Copyright Notice In lang.h
** lnode.h
** (Y) Expressing Parsing Stuff
*/


typedef int NodeId;

/* Nodes are mainly for storing expressions
temporarily and then generating bytecode from
them, effectively an AST but limited to expressions,
this step isn't necessary but it makes things
easier to understand and tends to be a little
bit more flexible. Nodes are stored in a
node pool and recycled. */
typedef enum NodeName {
	NODE_NONE = 0,

	/* literals */
	NODE_INTEGER,
	NODE_NUMBER,
	NODE_STRING,

	NODE_FUNCTION,
	NODE_TABLE,
	NODE_LOADFILE,

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

	/* binary nodes, use x,y */
	NODE_RANGE   = TK_DOT_DOT,
	NODE_LOG_AND = TK_LOG_AND,
	NODE_LOG_OR  = TK_LOG_OR,
	NODE_ADD     = TK_ADD,
	NODE_SUB     = TK_SUB,
	NODE_DIV     = TK_DIV,
	NODE_MUL     = TK_MUL,
	NODE_MOD     = TK_MODULUS,
	NODE_NEQ     = TK_NOT_EQUALS,
	NODE_EQ      = TK_EQUALS,
	NODE_LT      = TK_LESS_THAN,
} NodeName;


typedef struct Node {
	NodeName k;
	char *line;

	union {
		/* Node inputs, if variable number of inputs
		are required, use z. */
		struct {
			NodeId x,y,*z;
		};
		union {
			char const *s;
			Integer   i;
			Number    n;
		} lit;
	};
} Node;


NodeId langY_node3(FileState *fs, char * loc, NodeId k, NodeId x, NodeId y, NodeId *z);
NodeId langY_node2(FileState *fs, char * loc, NodeId k, NodeId x, NodeId y);

/* The following nodes represent different literal
or constant values, I for int, N for number and
so on respectively */
NodeId langY_nodeI(FileState *fs, char *, Integer i);
NodeId langY_nodeN(FileState *fs, char *, Integer i);
NodeId langY_nodeS(FileState *fs, char *, char const *);
NodeId langY_nodeH(FileState *fs, char *, NodeId *z);
NodeId langY_nodeF(FileState *fs, char *, NodeId x, NodeId *z);

/* essentially an upvalue, but I wanted to come
up with a different name ... */
NodeId langY_cachenode(FileState *fs, char * loc, NodeId i);
NodeId langY_localnode(FileState *fs, char * loc, NodeId i);
NodeId langY_globalnode(FileState *fs, char * loc, NodeId i);

NodeId langY_fieldnode(FileState *fs, char * loc, NodeId x, NodeId y);
NodeId langY_designode(FileState *fs, char * loc, NodeId x, NodeId y);

NodeId langY_loadfilenode(FileState *fs, char * loc, NodeId x);

NodeId langY_rangeindexnode(FileState *fs, char * loc, NodeId x, NodeId y);

NodeId langY_metacallnode(FileState *fs, char * loc, NodeId x, NodeId y, NodeId *z);
NodeId langY_callnode(FileState *fs, char * loc, NodeId x, NodeId *z);

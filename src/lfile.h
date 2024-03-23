/*
** See Copyright Notice In lang.h
** lfile.h
** (Y) Parsing Structures
*/


typedef int LocalId;


#define NO_LOC 0


/* maps a name to a node.  */
typedef struct FuncLocal {
	char *name;
	/* todo: remove these two */
	int  level;
	char  *loc;
	/* the node itself */
	NodeId   n;
} FuncLocal;


typedef struct FuncState FuncState;
typedef struct FuncState {
	FuncState *enclosing;
	char *loc;
	/* the level in which this function
	was created in. */
	int level;
	/* index to first local within locals
	in file. */
	int locals;

	/* todo: not quite yet,
	array of cache nodes that are to be
	retreived from enclosing function.
	each cached local level and index
	should be higher than that of this
	function's level and starting local index. */

	LocalId *caches;
	/* this is needed to emit instructions
	relative to the current function we're parsing */
	int bytes;
} FuncState;


typedef struct FileState {
	Module *md;
	/* this is so that we can allocate
	objects during compilation time,
	runtime is present everywhere anyways. */
	Runtime *rt;
	char *filename;
	char *linechar;
	char *contents;
	char *thischar;
	int linenumber;
	Token tk;
	Token tkthen;
	/* buffer for nodes */
	Node *nodes;
	int nnodes;
	/* the current level, 0 is for file. */
	int level;
	/* Stack of locals for current function stack,
	each function points to a local defined here,
	when a function is finished parsing, deallocates
	its locals by restoring nlocals to what it
	was when it entered. Remaining locals are file
	locals level, when calling this file, use
	nlocals to create a prototype. */
	FuncLocal *locals;
	int nlocals;
	int bytes;
	int nbytes;
	/* stack of currently being parsed functions,
	allocated in C stack by caller function. */
	FuncState *fn;
	char *lhsname;
} FileState;


NodeId langY_loadexpr(FileState *fs);
NodeId langY_parsestat(FileState *fs);

/*
** See Copyright Notice In lang.h
** (Y) lfile.h
** Parsing Structures
*/


typedef int LocalId;


#define NO_LOC 0


/* maps a name to a local node.  */
typedef struct FileLocal {
	char *name;
	/* todo: remove these two */
	int  level;
	char  *loc;
	/* the node itself */
	NodeId   n;
} FileLocal;


typedef struct FileFunc FileFunc;
typedef struct FileFunc {
	FileFunc *enclosing;
	char *line;
	/* Index to first local within locals
	in file. We use this to also determine
	whether a local should be cached or not. */
	NodeId locals;

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
} FileFunc;


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
	Token lasttk,tk,thentk;
	/* buffer for nodes */
	Node *nodes;
	int nnodes;
	/* the current level, 0 is for file. */
	int level;
	/* Stack of locals for current function stack,
	each function points to a local defined here,
	when a function is finished parsing, it deallocates
	its locals by restoring nlocals to what it
	was when it entered. Remaining locals are file
	locals, at file level, when calling this file, use
	nlocals to create a prototype. */
	FileLocal *locals;
	int nlocals;
	int bytes;
	int nbytes;
	/* stack of currently being parsed functions,
	allocated in C stack by caller function. */
	FileFunc *fn;
	char *lhsname;
} FileState;


NodeId langY_loadexpr(FileState *fs);
NodeId langY_loadstat(FileState *fs);
NodeId langY_loadunary(FileState *fs);

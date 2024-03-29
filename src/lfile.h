/*
** See Copyright Notice In lang.h
** (Y) lfile.h
** Parsing Structures
*/


/* maps a name to a local  */
typedef struct FileLocal {
	char   *name;
	char   *line;
	lnodeid node;
	int    level;
} FileLocal;


typedef struct FileFunc FileFunc;
typedef struct FileFunc {
	FileFunc *enclosing;
	char *line;
	/* index to first local within locals in file,
	we use this to also determine whether a local
	should be cached (captured) or not. locals
	refer to specifically named values. */
	llocalid locals;
	llocalid nlocals;
	/* array of locals from enclosing function
	that are to be cached */
	llocalid *caches;
	/* this is needed to emit instructions
	relative to the current function we're parsing */
	lbyteid bytes;

	int nyield;
	/* list of yield jumps to be patched */
	lbyteid *yj;
	/* list of all defer targets */
	lbyteid *lt;
	/* last defer jump */
	lbyteid lj;
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
	ltoken lasttk,tk,thentk;
	/* buffer for nodes */
	Node *nodes;
	lnodeid nnodes;
	/* the current level, level is incremented
	per level, block or statement or whenever
	it makes sense, represents a visibility
	layer, 0 is for file. */
	int level;
	/* hierarchical list of locals for current
	for all loading functions, each function points
	to a local defined here by index. remaining
	locals are file locals, at file level,
	when calling this file, use nlocals to create
	a prototype. */
	FileLocal *locals;
	llocalid nlocals;
	/* hierarchical list of loading functions,
	each allocated in C stack by caller function */
	FileFunc *fn;
} FileState;


lnodeid langY_loadexpr(FileState *fs);
lnodeid langY_loadunary(FileState *fs);
void langY_loadstat(FileState *fs);

/*
** See Copyright Notice In lang.h
** (Y) lfile.h
** Parsing Structures
*/


/* Used for scoping, will possibly remove in the
future if performance matters than much, maps a
name to either an enum or local, enums are
jit time constants and do not occupy
stack space */
typedef struct FileName {
	char   *name;
	llineid line;
	/* the level in which this name was
	declared, for scoping */
	int    level;
	/* the node that contains the value
	if constant, or local node if local. */
	ltreeid node;
	lbool   enm;
} FileName;


typedef struct FileFunc FileFunc;
typedef struct FileFunc {
	FileFunc *enclosing;
	llineid line;
	/* the total stack length, for explicit
	addressing, when jitting this is what's
	allocated */
	llocalid stklen;
	llocalid stktop;
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
	relative to the current function we're
	parsing, there's always an active function,
	even at file level */
	lbyteid bytes;

	int nyield;

	/* todo: deprecated */
	/* list of yield jumps to be patched */
	lbyteid *yj;
} FileFunc;


typedef struct FileState {
	lModule *md;
	/* this is so that we can allocate
	objects during compilation time,
	runtime is present everywhere anyways. */
	lRuntime *rt;
	char *filename;
	char *linechar;
	char *contents;
	char *thischar;
	int linenumber;
	ltoken lasttk,tk,thentk;
	/* buffer for nodes */
	Tree *nodes;
	ltreeid nnodes;
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
	FileName *locals;
	llocalid nlocals;
	/* hierarchical list of loading functions,
	each allocated in C stack by caller function */
	FileFunc *fn;

	lbyteid bytes;
} FileState;


ltreeid langY_loadexpr(FileState *fs);
ltreeid langY_loadunary(FileState *fs);
void langY_loadstat(FileState *fs);

/*
** See Copyright Notice In lang.h
** (Y) lfile.h
** Parsing Structures
*/


#define NO_ENTITY (-1)


typedef int lentityid;


/* -- for scoping, binds a name to some local
- value such as a label, enum or local variable. */
typedef struct lentity {
	char    *name;
	llineid  line;
	llocalid slot;
	lbool     enm;
	/* the level in which this name was
	declared, for scoping */
	int     level;
} lentity;


typedef struct FileFunc FileFunc;
typedef struct FileFunc {
	FileFunc *enclosing;
	llineid line;
	/* number of locals for this function,
	allocated once at runtime, each bytecode
	instruction explicity addresses one of
	these slots. */
	llocalid nlocals;
	/* current number of locals, can be less
	than nlocals but never less, in other words,
	nlocals grows to accommodate xlocals */
	llocalid xlocals;
	/* index to first entity within entities
	in enclosing scope. */
	lentityid entitys;
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
	/* -- hierarchical list of local tags for the current
	- function stack, each function points to a base local
	- tag defined here by index.
	-- remaining locals are file locals, at file level. */
	lentity *entitys;
	int nentitys;

	/* hierarchical list of loading functions,
	each allocated in C stack by caller function */
	FileFunc *fn;

	lbyteid bytes;
} FileState;


ltreeid langY_loadexpr(FileState *fs);
ltreeid langY_loadunary(FileState *fs);
void langY_loadstat(FileState *fs);

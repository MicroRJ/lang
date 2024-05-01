/*
** See Copyright Notice In elf.h
** (Y) lfile.h
** Parsing Structures
*/


#define NO_ENTITY (lentityid){-1}

#define NOTANENTITY 0x01

typedef struct { int x; } lentityid;


/* -- for scoping, binds a name to some local
- value such as a label, enum or local variable. */
typedef struct lentity {
	char    *name;
	llineid  line;
	llocalid slot;
	elf_bool     enm;
	/* the level in which this name was
	declared, for scoping */
	int     level;
} lentity;


typedef struct elf_FileFunc elf_FileFunc;
typedef struct elf_FileFunc {
	elf_FileFunc *enclosing;
	llineid line;
	/* for the basic register allocation system, where we have
	an infinite number of register, but we still want to keep
	the number of registers at a minimum, we resort to using
	two counters, nlocals and xmemory */
	/* maximum number of local register used concurrently
	at any point for this function */
	llocalid nlocals;
	/* the memory state, in other words, the current number
	of local registers that are being used at this point. */
	llocalid xmemory;
	/* index to first entity within entity list in file. */
	int entities;
	/* array of entities from enclosing function
	that are to be cached */
	lentityid *captures;
	/* this is needed to emit instructions
	relative to the current function we're
	parsing, there's always an active function,
	even at file level */
	lbyteid bytes;

	int nyield;

	/* todo: deprecated */
	/* list of yield jumps to be patched */
	lbyteid *yj;
} elf_FileFunc;


typedef struct elf_FileState {

	union { elf_Module  *M,*md; };
	union { elf_Runtime *R,*rt; };

	char *filename;
	char *linechar;
	char *contents;
	char *thischar;
	int linenumber;
	ltoken lasttk,tk,thentk;
	/* buffer for nodes */
	lNode *nodes;
	lnodeid nnodes;
	/* the current level, level is incremented
	per level, block or statement or whenever
	it makes sense, represents a visibility
	layer, 0 is for file. */
	int level;
	/* -- hierarchical list of local tags for the current
	- function stack, each function points to a base local
	- tag defined here by index.
	-- remaining locals are file locals, at file level. */
	lentity *entities;
	int nentities;

	/* hierarchical list of loading functions,
	each allocated in C stack by caller function */
	elf_FileFunc *fn;

	lbyteid bytes;

	int flags;

	unsigned statbreak: 1;
} elf_FileState;


lnodeid elf_fsloadexpr(elf_FileState *fs);
lnodeid elf_fsloadunary(elf_FileState *fs);
void elf_fsloadstat(elf_FileState *fs);

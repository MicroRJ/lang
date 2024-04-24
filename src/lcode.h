/*
** See Copyright Notice In lang.h
** (L) lcode.h
** Bytecode Generator (node -> bytecode)
*/


#define NO_SLOT (-1)
#define NO_BYTE (-1)
#define NO_JUMP (-0)
#define NO_LINE (-0)


typedef struct FileBlock {
	lbyteid j,e;
} FileBlock;


/* Contains list of false and true jumps
generated by some boolean expression. */
typedef struct ljlist {
	lbyteid *t,*f;
} ljlist;


/* Contains list of jumps generated
by a select statement, such as if else */
typedef struct Select {
	/* Conditional false jump instructions
	to be patched so that they jump to
	the next block or instruction */
	lbyteid *jz;

	/* list of exit jump instructions from
	each block to be patched */
	lbyteid *j;
} Select;


/* memory state should be handled by the user, the loop
can allocate a register if none given, so always save
and restore the memory state. */
typedef struct Loop {
	/* the expression to increment, should be a local */
	lnodeid  x;
	/* the register to increment, should be same as x.r,
	this is kept here to ensure x wasn't deallocated or
	modified. */
	llocalid r;

	/* the entry byte */
	lbyteid  e;
	/* list of false jumps to patch */
	lbyteid *f;
} Loop;


void langL_moveto(elf_FileState *fs, llineid line, lnodeid x, lnodeid y);
void langL_localload(elf_FileState *fs, llineid line, elf_bool reload, llocalid x, llocalid y, lnodeid id);
llocalid langL_localize(elf_FileState *fs, llineid line, lnodeid id);


void langL_begindelayedblock(elf_FileState *fs, llineid line, FileBlock *bl);
void langL_closedelayedblock(elf_FileState *fs, llineid line, FileBlock *bl);


lbyteid langL_branchiffalse(elf_FileState *fs, ljlist *js, llocalid x, lnodeid id);
lbyteid langL_branchiftrue(elf_FileState *fs, ljlist *js, llocalid x, lnodeid id);
lbyteid *langL_jumpiftrue(elf_FileState *fs, ljlist *js, llocalid x, lnodeid id);
lbyteid *langL_jumpiffalse(elf_FileState *fs, ljlist *js, llocalid x, lnodeid id);


enum {
	L_IF  = 0, // JZ
	L_IFF = 1, // JNZ
};


void langL_beginif(elf_FileState *fs, char *line, Select *s, lnodeid x, int z);
void langL_addelif(elf_FileState *fs, char *line, Select *s, lnodeid x);
void langL_addelse(elf_FileState *fs, char *line, Select *s);
void langL_addthen(elf_FileState *fs, char *line, Select *s);
void langL_closeif(elf_FileState *fs, char *line, Select *s);


void langL_beginrangedloop(elf_FileState *fs, char *line, Loop *loop, lnodeid x, lnodeid lo, lnodeid hi);
void langL_closerangedloop(elf_FileState *fs, char *line, Loop *loop);

void langL_begindowhile(elf_FileState *fs, char *line, Loop *loop);
void langL_closedowhile(elf_FileState *fs, char *line, Loop *loop, lnodeid x);

void langL_beginwhile(elf_FileState *fs, char *line, Loop *loop, lnodeid x);
void langL_closewhile(elf_FileState *fs, char *line, Loop *loop);

/*
** See Copyright Notice In lang.h
** (L) lcode.h
** (L)ang Code Generation
*/


#define NO_SLOT (-1)
#define NO_BYTE (-1)
#define NO_JUMP (-0)
#define NO_LINE (-0)


typedef struct CodeBlock {
	lbyteid j,e;
} CodeBlock;


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


typedef struct Loop {
	/* local register for this loop */
	llocalid r,y;
	/* associated loop expression */
	lnodeid  x;
	lbyteid  e;
	/* list of false jumps to patch */
	lbyteid *f;
} Loop;


void langL_moveto(FileState *fs, llineid line, lnodeid x, lnodeid y);
void langL_localload(FileState *fs, llineid line, lbool reload, llocalid x, llocalid y, lnodeid id);
llocalid langL_localize(FileState *fs, llineid line, lnodeid id);


void langL_begindelayedblock(FileState *fs, char *line, CodeBlock *bl);
void langL_closedelayedblock(FileState *fs, char *line, CodeBlock *bl);


lbyteid langL_branchiffalse(FileState *fs, ljlist *js, llocalid x, lnodeid id);
lbyteid langL_branchiftrue(FileState *fs, ljlist *js, llocalid x, lnodeid id);
lbyteid *langL_jumpiftrue(FileState *fs, ljlist *js, llocalid x, lnodeid id);
lbyteid *langL_jumpiffalse(FileState *fs, ljlist *js, llocalid x, lnodeid id);


enum {
	L_IF  = 0, // JZ
	L_IFF = 1, // JNZ
};


void langL_beginif(FileState *fs, char *line, Select *s, lnodeid x, int z);
void langL_addelif(FileState *fs, char *line, Select *s, lnodeid x);
void langL_addelse(FileState *fs, char *line, Select *s);
void langL_addthen(FileState *fs, char *line, Select *s);
void langL_closeif(FileState *fs, char *line, Select *s);


void langL_beginrangedloop(FileState *fs, char *line, Loop *loop, lnodeid x, lnodeid lo, lnodeid hi);
void langL_closerangedloop(FileState *fs, char *line, Loop *loop);

void langL_begindowhile(FileState *fs, char *line, Loop *loop);
void langL_closedowhile(FileState *fs, char *line, Loop *loop, lnodeid x);

void langL_beginwhile(FileState *fs, char *line, Loop *loop, lnodeid x);
void langL_closewhile(FileState *fs, char *line, Loop *loop);

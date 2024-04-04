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
	lbyteid x,e;
	/* list of false jumps to patch */
	lbyteid *f;
} Loop;


void langL_begindelayedblock(FileState *fs, char *line, CodeBlock *bl);
void langL_closedelayedblock(FileState *fs, char *line, CodeBlock *bl);


void langL_loadinto(FileState *fs, llineid line, ltreeid x, ltreeid y);
void langL_reload(FileState *fs, llineid line, lbool reload, llocalid x, llocalid y, ltreeid id);
llocalid langL_load2any(FileState *fs, llineid line, llocalid n, ltreeid t);


enum {
	L_IF  = 0, // JZ
	L_IFF = 1, // JNZ
};
void langL_beginif(FileState *fs, char *line, Select *s, ltreeid x, int z);
void langL_addelif(FileState *fs, char *line, Select *s, ltreeid x);
void langL_addelse(FileState *fs, char *line, Select *s);
void langL_addthen(FileState *fs, char *line, Select *s);
void langL_closeif(FileState *fs, char *line, Select *s);


void langL_beginloop(FileState *fs, char *line, Loop *loop, ltreeid x, ltreeid r);
void langL_closeloop(FileState *fs, char *line, Loop *loop);

void langL_begindowhile(FileState *fs, char *line, Loop *loop);
void langL_closedowhile(FileState *fs, char *line, Loop *loop, ltreeid x);

void langL_beginwhile(FileState *fs, char *line, Loop *loop, ltreeid x);
void langL_closewhile(FileState *fs, char *line, Loop *loop);

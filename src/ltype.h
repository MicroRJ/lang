/*
** See Copyright Notice In lang.h
** ltype.h
** Type Definitions
*/



typedef long long int llongint;
typedef signed int lbool;
typedef double lnumber;



typedef unsigned int lhashid;
typedef int llocalid;
typedef int lbyteid;
typedef int lglobalid;
/* todo: eventually convert this to an offset */
typedef char *llineid;


typedef char lchar;
typedef void *Ptr;
typedef void *Handle;


typedef int (* lBinding)(lRuntime *);
// typedef int (* lJITFunc)(int);

/*
** Function Prototype
*/
typedef struct lProto {
	/* xin, yield */
	short x,y;
	/* number of cached values */
	short ncaches;
	/* stack size required allocated by runtime
	at call time. */
	short nlocals;
	short nbytes;
	short bytes;
} lProto;




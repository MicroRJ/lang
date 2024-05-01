/*
** See Copyright Notice In elf.h
** elf-tys.h
** Type Definitions
*/



typedef long long int elf_int;
typedef signed int elf_bool;
typedef double elf_num;



typedef unsigned int elf_hashint;
typedef int llocalid;
typedef int lbyteid;
typedef int lglobalid;
/* todo: eventually convert this to an offset */
typedef char *llineid;


typedef char lchar;
typedef void *Ptr;
typedef void *elf_Handle;


typedef int (* lBinding)(elf_ThreadState *);
// typedef int (* lJITFunc)(int);

/*
** Function Prototype
*/
typedef struct elf_Proto {
	/* xin, yield */
	short x,y;
	/* number of cached values */
	short ncaches;
	/* stack size required allocated by runtime
	at call time. */
	short nlocals;
	int nbytes;
	int bytes;
} elf_Proto;




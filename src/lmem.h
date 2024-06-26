/*
** See Copyright Notice In lang.h
** lmem.h
** Memory Tools
*/


typedef struct Alloc Alloc;


#define FLYTRAP 0x55555555

#define CHUNKSIZE 1024
#define CHUNKCATE(x,y) ((x+y-1)/y*y)


#define GIGABYTES(x) ((x)*1024llu*1024llu*1024llu)


#define ALLOCFN(NAME) Error NAME (Alloc *allocator, int flags, llong oldSize, llong newSize, void **oldAndNewMemory, ldebugloc loca)


typedef Error (* AllocFn)(Alloc *allocator, int flags, llong oldSize, llong newSize, void **oldAndNewMemory, ldebugloc loca);


typedef struct Alloc {
	char const *label;
	AllocFn fn;
} Alloc;


#if 0
typedef struct MemBlock MemBlock;
typedef struct MemBlock {
	unsigned int headtrap;
	MemBlock *then;
	ldebugloc loca;
	ldebugloc freeloca;
	llong contentssize;
	unsigned int foottrap;
} MemBlock;
#endif


void langM_debugdealloc(void *mem, ldebugloc loca);
void *langM_debugrealloc(void *mem, llong contentssize, ldebugloc loca);
void *langM_debugalloc(llong contentssize, ldebugloc loca);


lapi void langM_dealloc_(Alloc *allocator, void const *memory, ldebugloc loca);
lapi void *langM_realloc_(Alloc *allocator, llong size, void *memory, ldebugloc loca);
lapi void *langM_alloc_(Alloc *allocator, llong size, ldebugloc loca);
lapi void *langM_clearalloc_(Alloc *allocator, llong size, ldebugloc loca);


#define langM_dealloc(cator,mem) langM_dealloc_(cator,mem,LHERE)
#define langM_realloc(cator,sze,mem) langM_realloc_(cator,sze,mem,LHERE)
#define langM_alloc(cator,sze) langM_alloc_(cator,sze,LHERE)
#define langM_clearalloc(cator,sze) langM_clearalloc_(cator,sze,LHERE)


lapi ALLOCFN(langM_deftlocalallocfn);
lapi ALLOCFN(langM_defglobalallocfn);


/* todo: better names */
#define lTLOC (&langM_tlocalalloc)
#define lHEAP (&langM_globalalloc)

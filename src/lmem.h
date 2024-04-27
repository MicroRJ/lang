/*
** See Copyright Notice In elf.h
** lmem.h
** Memory Tools
*/


typedef struct Alloc Alloc;


#define FLYTRAP 0x55555555

#define CHUNKSIZE 1024
#define CHUNKCATE(x,y) ((x+y-1)/y*y)




#define ALLOCFN(NAME) Error NAME (Alloc *allocator, int flags, elf_int oldSize, elf_int newSize, void **oldAndNewMemory, ldebugloc loca)


typedef Error (* AllocFn)(Alloc *allocator, int flags, elf_int oldSize, elf_int newSize, void **oldAndNewMemory, ldebugloc loca);


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
	elf_int contentssize;
	unsigned int foottrap;
} MemBlock;
#endif


void langM_debugdealloc(void *mem, ldebugloc loca);
void *langM_debugrealloc(void *mem, elf_int contentssize, ldebugloc loca);
void *langM_debugalloc(elf_int contentssize, ldebugloc loca);


elf_api void langM_dealloc_(Alloc *allocator, void const *memory, ldebugloc loca);
elf_api void *langM_realloc_(Alloc *allocator, elf_int size, void *memory, ldebugloc loca);
elf_api void *langM_alloc_(Alloc *allocator, elf_int size, ldebugloc loca);
elf_api void *langM_clearalloc_(Alloc *allocator, elf_int size, ldebugloc loca);


#define elf_delmem(cator,mem) langM_dealloc_(cator,mem,LHERE)
#define langM_realloc(cator,sze,mem) langM_realloc_(cator,sze,mem,LHERE)
#define elf_alloc(cator,sze) langM_alloc_(cator,sze,LHERE)
#define elf_clearalloc(cator,sze) langM_clearalloc_(cator,sze,LHERE)


elf_api ALLOCFN(langM_deftlocalallocfn);
elf_api ALLOCFN(langM_defglobalallocfn);


/* todo: better names */
#define lTLOC (&langM_tlocalalloc)
#define lHEAP (&langM_globalalloc)

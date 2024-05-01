/*
** See Copyright Notice In elf.h
** elf-tab.h
** Table
*/


typedef struct elf_tabslot {
	elf_val k;
	elf_int i;
} elf_tabslot;


typedef struct elf_Table {
	elf_Object obj;
	elf_tabslot *slots;
	elf_int ntotal;
	elf_int nslots;
	elf_int ncollisions;
	/* array object */
	union {elf_val *v,*array;};
} elf_Table;


elf_Table *elf_newtabmetatab(elf_ThreadState *);

void elf_deltab(elf_Table *);
elf_Table *elf_newtablen(elf_ThreadState *, elf_int);
elf_Table *elf_newtab(elf_ThreadState *);

elf_int elf_tabtake(elf_Table *table, elf_val k);
void elf_tabset(elf_Table *table, elf_val k, elf_val v);
elf_int elf_tabhashval(elf_val v);
elf_hashint elf_tabrehash(elf_hashint hash);
elf_hashint elf_tabhashstr(char *junk);
elf_hashint elf_tabhashptr(Ptr *ptr);
elf_bool elf_tabvaleq(elf_val *x, elf_val *y);


/* metatable */
int elf_tabadd_(elf_ThreadState *);
int elf_tabidx_(elf_ThreadState *);
int elf_tabxrem_(elf_ThreadState *);
int elf_tabtally_(elf_ThreadState *);
int elf_tablength_(elf_ThreadState *);
int elf_tabunload_(elf_ThreadState *);
int elf_tabhaskey_(elf_ThreadState *);
int elf_tabput_(elf_ThreadState *);
int elf_tablookup_(elf_ThreadState *);
int elf_tabforeach_(elf_ThreadState *);
int elf_tabsort_(elf_ThreadState *);
int elf_tabcollisions_(elf_ThreadState *);

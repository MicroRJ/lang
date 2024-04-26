/*
** See Copyright Notice In elf.h
** (H) ltable.h
** Table
*/


typedef struct elf_tabslot {
	elf_val k;
	elf_int i;
} elf_tabslot;


typedef struct elf_tab {
	elf_Object obj;
	elf_tabslot *slots;
	elf_int ntotal;
	elf_int nslots;
	elf_int ncollisions;
	/* array object */
	union {elf_val *v,*array;};
} elf_tab;


elf_tab *elf_newtabmetatab(lRuntime *);

void elf_deltab(elf_tab *);
elf_tab *elf_newtablen(lRuntime *, elf_int);
elf_tab *elf_newtab(lRuntime *);

elf_int elf_tabtake(elf_tab *table, elf_val k);
void elf_tabput(elf_tab *table, elf_val k, elf_val v);
elf_int elf_tabhashval(elf_val v);
elf_hashint elf_tabrehash(elf_hashint hash);
elf_hashint elf_tabhashstr(char *junk);
elf_hashint elf_tabhashptr(Ptr *ptr);
elf_bool elf_tabvaleq(elf_val *x, elf_val *y);


/* metatable */
int elf_tabadd_(lRuntime *);
int elf_tabidx_(lRuntime *);
int elf_tabxrem_(lRuntime *);
int elf_tabtally_(lRuntime *);
int elf_tablength_(lRuntime *);
int elf_tabunload_(lRuntime *);
int elf_tabhaskey_(lRuntime *);
int elf_tabput_(lRuntime *);
int elf_tablookup_(lRuntime *);
int elf_tabforeach_(lRuntime *);
int elf_tabsort_(lRuntime *);
int elf_tabcollisions_(lRuntime *);

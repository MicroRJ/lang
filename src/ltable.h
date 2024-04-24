/*
** See Copyright Notice In lang.h
** (H) ltable.h
** Table
*/


typedef struct elf_tabslot {
	elf_val k;
	elf_int i;
} elf_tabslot;


typedef struct elf_tab {
	elf_obj obj;
	elf_tabslot *slots;
	elf_int ntotal;
	elf_int nslots;
	elf_int ncollisions;
	/* array object */
	elf_val    *v;
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
int langH_add_(lRuntime *);
int langH_idx_(lRuntime *);
int langH_unload_(lRuntime *);
int langH_length_(lRuntime *);
int langH_haskey_(lRuntime *);
int elf_tabput_(lRuntime *);
int langH_lookup_(lRuntime *);
int langH_foreach_(lRuntime *);
int langH_sort_(lRuntime *);
int langH_collisions_(lRuntime *);

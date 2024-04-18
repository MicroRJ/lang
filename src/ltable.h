/*
** See Copyright Notice In lang.h
** (H) ltable.h
** Hashing Tools && lTable lObject
*/




typedef struct HashSlot {
	lValue k;
	llongint i;
} HashSlot;


typedef struct lTable {
	lObject      obj;
	HashSlot *slots;
	llongint    ntotal;
	llongint    nslots;
	int ncollisions;
	/* array object */
	lValue        *v;
} lTable;


lTable *langH_newclass(lRuntime *R);
lTable *langH_new2(lRuntime *fs, llongint);
lTable *langH_new(lRuntime *fs);

llongint langH_take(lTable *table, lValue k);
void langH_free(lTable *t);
void langH_insert(lTable *table, lValue k, lValue v);
lhashid langH_rehash(lhashid hash);
lhashid langH_hashS(char *junk);
lhashid langH_hashPtr(Ptr *ptr);
lbool langH_valueeq(lValue *x, lValue *y);
llongint langH_hashvalue(lValue v);


/* metaclass */
int langH_add_(lRuntime *);
int langH_idx_(lRuntime *);
int langH_unload_(lRuntime *);
int langH_length_(lRuntime *);
int langH_haskey_(lRuntime *);
int langH_insert_(lRuntime *);
int langH_lookup_(lRuntime *);
int langH_foreach_(lRuntime *);
int langH_sort_(lRuntime *);
int langH_collisions_(lRuntime *);

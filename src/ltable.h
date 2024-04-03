/*
** See Copyright Notice In lang.h
** (H) ltable.h
** Hashing Tools && Table lObject
*/




typedef struct HashSlot {
	lValue k;
	llongint i;
} HashSlot;


typedef struct Table {
	lObject      obj;
	HashSlot *slots;
	llongint    ntotal;
	llongint    nslots;
	int ncollisions;
	/* array object */
	lValue        *v;
} Table;


Table *langH_new2(lRuntime *fs, llongint);
Table *langH_new(lRuntime *fs);


int langH_length_(lRuntime *c);
int langH_haskey_(lRuntime *c);
int langH_insert_(lRuntime *c);
int langH_lookup_(lRuntime *c);
int langH_foreach_(lRuntime *c);
int langH_sort_(lRuntime *c);
int langH_collisions_(lRuntime *c);


llongint langH_take(Table *table, lValue k);
void langH_free(Table *t);


lhashid langH_rehash(lhashid hash);
lhashid langH_hashS(char *bytes);
lhashid langH_hashPtr(Ptr *p);
lbool langH_valueeq(lValue *x, lValue *y);
llongint langH_hashvalue(lValue v);

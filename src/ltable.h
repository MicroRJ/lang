/*
** See Copyright Notice In lang.h
** (H) ltable.h
** Hashing Tools && Table Object
*/




typedef struct HashSlot {
	Value k;
	llong i;
} HashSlot;


typedef struct Table {
	Object      obj;
	HashSlot *slots;
	llong    ntotal;
	llong    nslots;
	int ncollisions;
	/* array object */
	Value        *v;
} Table;


Table *langH_new2(Runtime *fs, llong);
Table *langH_new(Runtime *fs);


int langH_length_(Runtime *c);
int langH_haskey_(Runtime *c);
int langH_insert_(Runtime *c);
int langH_lookup_(Runtime *c);
int langH_foreach_(Runtime *c);
int langH_sort_(Runtime *c);
int langH_collisions_(Runtime *c);


llong langH_take(Table *table, Value k);
void langH_free(Table *t);


lhash langH_rehash(lhash hash);
lhash langH_hashS(char *bytes);
lhash langH_hashPtr(Ptr *p);
lbool langH_valueeq(Value *x, Value *y);
llong langH_hashvalue(Value v);

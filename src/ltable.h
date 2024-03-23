/*
** See Copyright Notice In lang.h
** (H) ltable.h
** Hashing Tools && Table Object
*/


typedef unsigned int HashSize;


typedef struct HashSlot {
	Value   k;
	Integer i;
} HashSlot;


#if 0
/* this is is a simpler more generic construct
that can be used in more permissive contexts */
typedef struct Hash {
	HashSlot *slots;
	Integer  ntotal;
	Integer  ntally;
	int ncollisions;
} Hash;
#endif

/* todo: split this into a hash and table */
typedef struct Table {
	Object      obj;
	HashSlot *slots;
	Integer  ntotal;
	Integer  nslots;
	int ncollisions;
	/* array object */
	Value        *v;
} Table;


Table *langH_new2(Runtime *fs, Integer);
Table *langH_new(Runtime *fs);


int langH_length_(Runtime *c);
int langH_haskey_(Runtime *c);
int langH_insert_(Runtime *c);
int langH_lookup_(Runtime *c);
int langH_foreach_(Runtime *c);
int langH_sort_(Runtime *c);
int langH_collisions_(Runtime *c);


Integer langH_take(Table *table, Value k);
void langH_free(Table *t);


HashSize langH_rehash(HashSize hash);
HashSize langH_hashS(char *bytes);
HashSize langH_hashPtr(Ptr *p);
Bool langH_valueeq(Value *x, Value *y);
Integer langH_hashvalue(Value v);

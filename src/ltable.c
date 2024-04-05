/*
** See Copyright Notice In lang.h
** (H) ltable.c
** Table lObject && Hash Tools
*/


Table *langH_new2(lRuntime *fs, llongint ntotal) {
	/* todo: we could make this system a a bit more
	efficient by storing all methods in a single
	hash table, then we would have to assign each
	class a unique name id, which we would use to hash
	into the method table... */
	static MetaFunc _m[] = {
		{"length",langH_length_},
		{"haskey",langH_haskey_},
		{"hashin",langH_insert_},
		{"lookup",langH_lookup_},
		{"foreach",langH_foreach_},
		{"collisions",langH_collisions_},
	};
	Table *table = langGC_allocobj(fs,OBJ_TABLE,sizeof(Table));
	table->obj._m = _m;
	table->obj._n = _countof(_m);
	// lang_loginfo("'%llx': created new table",(llongint)table);

	table->ntotal = ntotal;
	table->nslots = 0;
	table->slots = langM_clearalloc(lHEAP,ntotal * sizeof(HashSlot));
	return table;
}


Table *langH_new(lRuntime *fs) {
	return langH_new2(fs,4);
}


void langH_free(Table *t) {
	langM_dealloc(lHEAP,t->slots);
	langM_dealloc(lHEAP,t->v);
}


/*
** 	Traverses the table until it finds a match
** or a nil slot for this particular key.
**
** 	If the result is -1 it means no nil slot
** nor match found, this is most likely an
** error as it means the table has reached
** peak capacity.
**
** 	Then you have to check whether the slot
** is nil, which means no match, use the
** result to modify the slot and value as desired.
**
*/
llongint langH_hashin(Table *table, lValue k) {
	/* This particular function uses double hashing,
	which should allow us to get more resolution out
	of the hash value, the first hash computes the
	starting index, and the secondary hash computes
	the step by which we increment.
	Since the increment depends on the data, it
	should reduce clustering, and in practice it
	has proven to be drastically more efficient
	than linear probing. Of course, this is already
	well known... */
	HashSlot *slots = table->slots;
	llongint ntotal = table->ntotal;
	llongint hash = langH_hashvalue(k);
	llongint head = hash % ntotal;
	llongint tail = head;
	lhashid walk = langH_rehash(hash)|1;
	do {
		lValue x = slots[tail].k;
		if (x.tag == VALUE_NONE) return tail;
		if (langH_valueeq(&x,&k)) return tail;
		tail = (tail+walk) % ntotal;
		LDODEBUG( table->ncollisions ++ );
	} while(head != tail);
	return -1;
}


llongint langH_slot2index(Table *table, llongint slot) {
	return table->slots[slot].i;
}


lValue langH_slot2value(Table *table, llongint slot) {
	return table->v[table->slots[slot].i];
}


lbool langH_slotiskey(Table *table, llongint slot) {
	return slot >= 0 && table->slots[slot].k.tag != VALUE_NONE;
}


void langH_slotsetkeyval(Table *table, llongint slot, lValue k, llongint i) {
	table->slots[slot].k = k;
	table->slots[slot].i = i;
}


void langH_checkthreshold(Table *table) {
	if (table->ntotal * 3 < table->nslots * 4) {
		// LDODEBUG( table->ncollisions = 0 );

		/* todo:
		Find a better strategy for incrementing
		the table size >> 1 << 2 */
		Table newtable = * table;
		newtable.ntotal = table->ntotal << 2;
		if (newtable.ntotal < table->ntotal) LNOBRANCH;
		newtable.slots = langM_clearalloc(lHEAP,newtable.ntotal * sizeof(HashSlot));

		for (int i = 0; i < table->ntotal; ++ i) {
			HashSlot slot = table->slots[i];
			if (slot.k.tag == VALUE_NONE) continue;

			llongint newslot = langH_hashin(&newtable,slot.k);

			if (newslot == -1) LNOBRANCH;

			newtable.slots[newslot] = slot;
		}

		langM_dealloc(lHEAP,table->slots);

		table->ntotal = newtable.ntotal;
		table->slots = newtable.slots;
	}
}


void langH_insert(Table *table, lValue k, lValue v) {
	langH_checkthreshold(table);
	llongint slot = langH_hashin(table,k);
	if (slot == -1) LNOBRANCH;
	HashSlot *entry = table->slots + slot;
	if (!langH_slotiskey(table,slot)) {
		llongint i = langA_variadd(table->v,1);
		table->v[i] = v;

		table->slots[slot].k = k;
		table->slots[slot].i = i;
		table->nslots ++;
	} else {
		table->v[entry->i] = v;
	}
}


lValue langH_lookup(Table *table, lValue k) {
	llongint slot = langH_hashin(table,k);
	if (slot == -1) LNOBRANCH;
	if (langH_slotiskey(table,slot)) {
		return langH_slot2value(table,slot);
	}
	return (lValue){VALUE_NONE,0};
}


llongint langH_take(Table *table, lValue k) {
	LASSERT((k.tag == TAG_INTEGER || k.tag == TAG_NUMBER) || k.s != 0);

	langH_checkthreshold(table);
	llongint slot = langH_hashin(table,k);
	if (slot == -1) LNOBRANCH;
	if (!langH_slotiskey(table,slot)) {
		llongint i = langA_variadd(table->v,1);
		table->v[i] = (lValue){VALUE_NONE};

		table->slots[slot].k = k;
		table->slots[slot].i = i;
		table->nslots ++;
	}
	return langH_slot2index(table,slot);
}


llongint langH_iadd(Table *table, lValue v) {
	return langA_variadd(table->v,1);
}


void langH_add(Table *table, lValue v) {
	langA_varadd(table->v,v);
}


/*
** Meta Methods:
*/
int langH_length_(lRuntime *c) {
	lang_pushlong(c,langA_varlen(((Table*)c->f->obj)->v));
	return 1;
}


int langH_haskey_(lRuntime *c) {
	LASSERT(c->f->x == 1);
	Table *table = (Table*) c->f->obj;
	lValue k = lang_load(c,0);
	lang_pushlong(c,langH_slotiskey(table,langH_hashin(table,k)));
	return 1;
}


int langH_lookup_(lRuntime *c) {
	LASSERT(c->f->x == 1);
	lValue k = lang_load(c,0);
	Table *table = (Table*) c->f->obj;
	lang_pushvalue(c,langH_lookup(table,k));
	return 1;
}


int langH_collisions_(lRuntime *c) {
	Table *table = (Table*) c->f->obj;
	lang_pushlong(c,table->ncollisions);
	return 1;
}


int langH_insert_(lRuntime *c) {
	int n = c->f->x;

	LASSERT(n >= 1 && n <= 2);

	lValue k = lang_load(c,0);

	lValue j = {VALUE_NONE};
	if (n == 2) j = lang_load(c,1);

	Table *table = (Table*) c->f->obj;
	langH_insert(table,k,j);
	return 0;
}


int langH_foreach_(lRuntime *c) {
	LASSERT(c->f->x == 1);
	LASSERT(lang_load(c,0).tag == TAG_CLOSURE);

	Table *table = (Table *) c->f->obj;

	lClosure *cl = lang_load(c,0).f;

	for (int i = 0; i < table->ntotal; ++ i) {
		HashSlot slot = table->slots[i];
		if (slot.k.tag != VALUE_NONE) {
			/* call the iterator function with two arguments */
			lang_callargs(c,cl,2,0,slot.k,table->v[slot.i]);
		}
	}

	return 0;
}


/* Some of the hash functions and comments
were borrowed from the great Sean Barrett */
lhashid langH_rehash(lhashid hash) {
	return ((hash) + ((hash) >> 6) + ((hash) >> 19));
}


#if 1
// FNV-1a
lhashid langH_hashS (char *bytes) {
	lhashid hash = 2166136261u;
	while (*bytes) {
		hash ^= *bytes++;
		hash *= 16777619;
	}
	return hash;
}
#else
lhashid langH_hashS(char *bytes) {
	lhashid hash = 0;
	while (*bytes) {
		hash = (hash << 7) + (hash >> 25) + *bytes++;
	}
	return hash + (hash >> 16);
}
#endif


lhashid langH_hashPtr(Ptr *p) {
   // typically lacking in low bits and high bits
	lhashid hash = langH_rehash((lhashid)(llongint)p);
	hash += hash << 16;

   // pearson's shuffle
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 2;
	hash += hash >> 15;
	hash ^= hash << 10;
	return langH_rehash(hash);
}


lbool langH_valueeq(lValue *x, lValue *y) {
	if (x->tag != y->tag) {
		return lfalse;
	}

	switch (x->tag) {
		case VALUE_STRING: {
			return langS_eq(x->s,y->s);
		}
		case TAG_INTEGER:
		case TAG_NUMBER:
		case TAG_TABLE:
		case TAG_CLOSURE:
		case VALUE_BINDING: {
			return x->i == y->i;
		}
	}

	LNOBRANCH;
	return lfalse;
}


llongint langH_hashvalue(lValue v) {
	switch (v.tag) {
		case VALUE_STRING: {
			return v.s->hash;
		}
		case TAG_NUMBER:
		case TAG_TABLE:
		case TAG_CLOSURE:
		case VALUE_BINDING:
		case TAG_INTEGER: {
			return langH_hashPtr(v.p);
		}
	}
	LNOBRANCH;
	return lfalse;
}


#if 0
void langH_mergesort_(Table *h, int level, llongint x, llongint z, lProto *fn) {

	if (z-x <= 1) return;

	int y = x + (z-x)/2;
	langH_mergesort_(h,level+1,x,y,fn);
	langH_mergesort_(h,level+1,y,z,fn);

	lValue *v = table->v;
	HashSlot *s = table->slots;

	for (;;) {
		int i = 0;

		for (; i < z-y && s[i].k.tag == VALUE_NONE; ++i);
		if (s[i].k.tag == VALUE_NONE) break;

		HashSlot xs = table->slots[i];

		for (; i < z-y && s[i].k.tag == VALUE_NONE; ++i);
		if (s[i].k.tag == VALUE_NONE) break;

		HashSlot ys = table->slots[i];

		lValue a = v[xs.i];
		lValue b = v[ys.i];

		int r = lang_callargs(c,fn,2,a,b);
		lValue vr = pop();
		LASSERT(vr.tag == TAG_INTEGER);

		if (vr.i) {
			lValue t = h[x+i];
			h[x+i] = h[y+i];
			h[y+i] = t;
		}
	}
}
int langH_sort_(lRuntime *c) {

}




#if 0
int tosort[] = {1,2,3,4};

void printarr(int *h, int z) {
	printf("{");
	for (int i = 0; i < z; ++i) {
		if (i != 0) printf(", ");
		printf("%i",h[i]);
	}
	printf("}\n");
}

void sort(int *h, int level, int x, int z) {
	if (z-x > 1) {
		int y = x + (z-x)/2;
		sort(h,level+1,x,y);
		sort(h,level+1,y,z);
		for (int i=0; i<z-y; ++i) {
			if (h[x+i] < h[y+i]) {
				int t = h[x+i];
				h[x+i] = h[y+i];
				h[y+i] = t;
			}
		}
	}
}

// printarr(tosort,_countof(tosort));
sort(tosort,0,0,_countof(tosort));
printf("finished: \n");
printarr(tosort,_countof(tosort));
int a = 1;
if(a) return 1;
#endif

#endif


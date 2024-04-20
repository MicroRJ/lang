/*
** See Copyright Notice In lang.h
** (H) ltable.c
** lTable lObject && Hash Tools
*/


lTable *langH_newclass(lRuntime *R) {
	lTable *tab = lang_pushnewtable(R);
	langH_mf(R,tab,"length",langH_length_);
	langH_mf(R,tab,"haskey",langH_haskey_);
	langH_mf(R,tab,"hashin",langH_insert_);
	langH_mf(R,tab,"lookup",langH_lookup_);
	langH_mf(R,tab,"iter",langH_foreach_);
	langH_mf(R,tab,"collisions",langH_collisions_);
	langH_mf(R,tab,"unload",langH_unload_);
	langH_mf(R,tab,"add",langH_add_);
	langH_mf(R,tab,"idx",langH_idx_);
	return tab;
}


lTable *langH_new2(lRuntime *fs, llongint ntotal) {
	lTable *table = langGC_allocobj(fs,OBJ_TABLE,sizeof(lTable));
	table->obj.metaclass = fs->classofH;

	table->ntotal = ntotal;
	table->nslots = 0;
	table->slots = langM_clearalloc(lHEAP,ntotal * sizeof(HashSlot));
	return table;
}


lTable *langH_new(lRuntime *fs) {
	return langH_new2(fs,4);
}


void langH_free(lTable *t) {
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
llongint langH_hashin(lTable *table, lValue k) {
	/* this particular function uses double hashing,
	which should allow us to get more resolution out
	of the hash value, the first hash computes the
	starting index, and the secondary hash computes
	the step by which we increment.
	Since the increment depends on the data, it
	should reduce clustering, and in practice it
	has proven to be drastically more efficient
	than linear probing.
	Of course, this is already well known... */
	HashSlot *slots = table->slots;
	llongint ntotal = table->ntotal;
	llongint hash = langH_hashvalue(k);
	llongint head = hash % ntotal;
	llongint tail = head;
	lhashid walk = langH_rehash(hash)|1;
	do {
		lValue x = slots[tail].k;
		if (x.tag == TAG_NIL) return tail;
		if (langH_valueeq(&x,&k)) return tail;
		tail = (tail+walk) % ntotal;
		LDODEBUG( table->ncollisions ++ );
	} while(head != tail);
	return -1;
}


llongint langH_slot2index(lTable *table, llongint slot) {
	return table->slots[slot].i;
}


lValue langH_slot2value(lTable *table, llongint slot) {
	return table->v[table->slots[slot].i];
}


lbool langH_slotiskey(lTable *table, llongint slot) {
	return slot >= 0 && table->slots[slot].k.tag != TAG_NIL;
}


void langH_slotsetkeyval(lTable *table, llongint slot, lValue k, llongint i) {
	table->slots[slot].k = k;
	table->slots[slot].i = i;
}


void langH_checkthreshold(lTable *table) {
	if (table->ntotal * 3 < table->nslots * 4) {
		// LDODEBUG( table->ncollisions = 0 );

		/* todo:
		Find a better strategy for incrementing
		the table size >> 1 << 2 */
		lTable newtable = * table;
		newtable.ntotal = table->ntotal << 2;
		if (newtable.ntotal < table->ntotal) LNOBRANCH;
		newtable.slots = langM_clearalloc(lHEAP,newtable.ntotal * sizeof(HashSlot));

		for (int i = 0; i < table->ntotal; ++ i) {
			HashSlot slot = table->slots[i];
			if (slot.k.tag == TAG_NIL) continue;

			llongint newslot = langH_hashin(&newtable,slot.k);

			if (newslot == -1) LNOBRANCH;

			newtable.slots[newslot] = slot;
		}

		langM_dealloc(lHEAP,table->slots);

		table->ntotal = newtable.ntotal;
		table->slots = newtable.slots;
	}
}


void langH_insert(lTable *table, lValue k, lValue v) {
	langH_checkthreshold(table);
	llongint slot = langH_hashin(table,k);
	/* todo: instead return an error here */
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


lValue langH_lookup(lTable *table, lValue k) {
	llongint slot = langH_hashin(table,k);
	if (langH_slotiskey(table,slot)) {
		return langH_slot2value(table,slot);
	}
	return (lValue){TAG_NIL,0};
}


llongint langH_take(lTable *table, lValue k) {
	LASSERT((k.tag == TAG_INT || k.tag == TAG_NUM) || k.s != 0);

	langH_checkthreshold(table);
	llongint slot = langH_hashin(table,k);
	if (slot == -1) LNOBRANCH;
	if (!langH_slotiskey(table,slot)) {
		llongint i = langA_variadd(table->v,1);
		table->v[i] = (lValue){TAG_NIL};

		table->slots[slot].k = k;
		table->slots[slot].i = i;
		table->nslots ++;
	}
	return langH_slot2index(table,slot);
}


llongint langH_iadd(lTable *table, lValue v) {
	return langA_variadd(table->v,1);
}


void langH_add(lTable *table, lValue v) {
	langA_varadd(table->v,v);
}


/*
** Meta Methods:
*/
int langH_length_(lRuntime *c) {
	lang_pushlong(c,langA_varlen(((lTable*)c->f->obj)->v));
	return 1;
}


int langH_haskey_(lRuntime *c) {
	LASSERT(c->f->x == 1);
	lTable *table = (lTable*) c->f->obj;
	lValue k = lang_load(c,0);
	lang_pushlong(c,langH_slotiskey(table,langH_hashin(table,k)));
	return 1;
}


int langH_lookup_(lRuntime *c) {
	LASSERT(c->f->x == 1);
	lValue k = lang_load(c,0);
	lTable *table = (lTable*) c->f->obj;
	lang_pushvalue(c,langH_lookup(table,k));
	return 1;
}


int langH_collisions_(lRuntime *c) {
	lTable *table = (lTable*) c->f->obj;
	lang_pushlong(c,table->ncollisions);
	return 1;
}


int langH_add_(lRuntime *R) {
	LASSERT(R->call->x >= 1);
	lTable *tab = (lTable *) R->call->obj;
	langH_add(tab,lang_load(R,0));
	return 0;
}


int langH_idx_(lRuntime *R) {
	LASSERT(R->call->x >= 1);
	lTable *tab = (lTable *) R->call->obj;
	llongint idx = lang_loadlong(R,0);
	lang_pushvalue(R,tab->v[idx]);
	return 1;
}


int langH_insert_(lRuntime *c) {
	int n = c->f->x;

	LASSERT(n >= 1 && n <= 2);

	lValue k = lang_load(c,0);

	lValue j = {TAG_NIL};
	if (n == 2) j = lang_load(c,1);

	lTable *table = (lTable*) c->f->obj;
	langH_insert(table,k,j);
	return 0;
}


int langH_foreach_(lRuntime *R) {
	LASSERT(R->frame->x == 1);
	lTable *table = (lTable *) R->frame->obj;
	lang_checkcl(R,0);
	llocalid k = lang_stkalloc(R,1);
	llocalid v = lang_stkalloc(R,1);
	for (int i = 0; i < table->ntotal; ++ i) {
		HashSlot slot = table->slots[i];
		if (slot.k.tag != TAG_NIL) {
			R->stk[k] = slot.k;
			R->stk[v] = table->v[slot.i];
			/* call the iterator function with two arguments */
			/* todo: should yield boolean to signal whether to
			stop or not */
			lang_call(R,R->frame->obj,0,0,2,0);
		}
	}
	return 0;
}


void ftabs(FILE *io, int level) {
	while (level --) fprintf(io,"\t");
}
void langH_unload(FILE *io, lTable *tab, int level) {
	fprintf(io,"{");
	int nitems = 0;
	for (int i = 0; i < tab->ntotal; ++ i) {
		HashSlot slot = tab->slots[i];
		if (slot.k.tag != TAG_NIL) {
			if (nitems ++ != 0) fprintf(io,",");
			syslib_fpfv_(io,slot.k,ltrue);
			fprintf(io," = ");
			lValue v = tab->v[slot.i];
			if (v.tag == TAG_TAB) {
				langH_unload(io,v.t,level+1);
			} else {
				syslib_fpfv_(io,v,ltrue);
			}
		}
	}
	fprintf(io,"}");
}


int langH_unload_(lRuntime *R) {
	lsysobj io = lang_getsysobj(R,0);
	langH_unload(io,(lTable*)(R->frame->obj),0);
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
		case TAG_STR: {
			return langS_eq(x->s,y->s);
		}
		case TAG_SYS: case TAG_INT: case TAG_NUM:
		case TAG_TAB: case TAG_CLS: case TAG_BID: {
			return x->i == y->i;
		}
	}

	LNOBRANCH;
	return lfalse;
}


llongint langH_hashvalue(lValue v) {
	switch (v.tag) {
		case TAG_STR: {
			return v.s->hash;
		}
		case TAG_TAB: case TAG_CLS: case TAG_SYS:
		case TAG_INT: case TAG_NUM: case TAG_BID: {
			return langH_hashPtr(v.p);
		}
	}
	LNOBRANCH;
	return lfalse;
}


#if 0
void langH_mergesort_(lTable *h, int level, llongint x, llongint z, lProto *fn) {

	if (z-x <= 1) return;

	int y = x + (z-x)/2;
	langH_mergesort_(h,level+1,x,y,fn);
	langH_mergesort_(h,level+1,y,z,fn);

	lValue *v = table->v;
	HashSlot *s = table->slots;

	for (;;) {
		int i = 0;

		for (; i < z-y && s[i].k.tag == TAG_NIL; ++i);
		if (s[i].k.tag == TAG_NIL) break;

		HashSlot xs = table->slots[i];

		for (; i < z-y && s[i].k.tag == TAG_NIL; ++i);
		if (s[i].k.tag == TAG_NIL) break;

		HashSlot ys = table->slots[i];

		lValue a = v[xs.i];
		lValue b = v[ys.i];

		int r = lang_callargs(c,fn,2,a,b);
		lValue vr = pop();
		LASSERT(vr.tag == TAG_INT);

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


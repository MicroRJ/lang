/*
** See Copyright Notice In lang.h
** (H) ltable.c
** elf_tab elf_obj && Hash Tools
*/


elf_tab *elf_newtabmetatab(lRuntime *R) {
	elf_tab *tab = elf_pushnewtab(R);
	elf_tabmfld(R,tab,"length",langH_length_);
	elf_tabmfld(R,tab,"haskey",langH_haskey_);
	elf_tabmfld(R,tab,"hashin",elf_tabput_);
	elf_tabmfld(R,tab,"lookup",langH_lookup_);
	elf_tabmfld(R,tab,"iter",langH_foreach_);
	elf_tabmfld(R,tab,"collisions",langH_collisions_);
	elf_tabmfld(R,tab,"unload",langH_unload_);
	elf_tabmfld(R,tab,"add",langH_add_);
	elf_tabmfld(R,tab,"idx",langH_idx_);
	return tab;
}


elf_tab *elf_newtablen(lRuntime *R, elf_int ntotal) {
	elf_tab *table = elf_newobj(R,OBJ_TAB,sizeof(elf_tab));
	if (R) table->obj.metatable = R->metatable_tab;

	table->ntotal = ntotal;
	table->nslots = 0;
	table->slots = elf_newmemzro(lHEAP,ntotal*sizeof(elf_tabslot));
	return table;
}


elf_tab *elf_newtab(lRuntime *R) {
	return elf_newtablen(R,4);
}


void elf_deltab(elf_tab *tab) {
	elf_delmem(lHEAP,tab->slots);
	elf_delvar(tab->v);
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
elf_int langH_hashin(elf_tab *table, elf_val k) {
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
	elf_tabslot *slots = table->slots;
	elf_int ntotal = table->ntotal;
	elf_int hash = elf_tabhashval(k);
	elf_int head = hash % ntotal;
	elf_int tail = head;
	elf_hashint walk = elf_tabrehash(hash)|1;
	do {
		elf_val x = slots[tail].k;
		if (x.tag == TAG_NIL) return tail;
		if (elf_tabvaleq(&x,&k)) return tail;
		tail = (tail+walk) % ntotal;
		LDODEBUG( table->ncollisions ++ );
	} while(head != tail);
	return -1;
}


elf_int langH_slot2index(elf_tab *table, elf_int slot) {
	return table->slots[slot].i;
}


elf_val langH_slot2value(elf_tab *table, elf_int slot) {
	return table->v[table->slots[slot].i];
}


elf_bool langH_slotiskey(elf_tab *table, elf_int slot) {
	return slot >= 0 && table->slots[slot].k.tag != TAG_NIL;
}


void langH_slotsetkeyval(elf_tab *table, elf_int slot, elf_val k, elf_int i) {
	table->slots[slot].k = k;
	table->slots[slot].i = i;
}


void langH_checkthreshold(elf_tab *table) {
	if (table->ntotal * 3 < table->nslots * 4) {
		// LDODEBUG( table->ncollisions = 0 );

		/* todo:
		Find a better strategy for incrementing
		the table size >> 1 << 2 */
		elf_tab newtable = * table;
		newtable.ntotal = table->ntotal << 2;
		if (newtable.ntotal < table->ntotal) LNOBRANCH;
		newtable.slots = elf_newmemzro(lHEAP,newtable.ntotal * sizeof(elf_tabslot));

		for (int i = 0; i < table->ntotal; ++ i) {
			elf_tabslot slot = table->slots[i];
			if (slot.k.tag == TAG_NIL) continue;

			elf_int newslot = langH_hashin(&newtable,slot.k);

			if (newslot == -1) LNOBRANCH;

			newtable.slots[newslot] = slot;
		}

		elf_delmem(lHEAP,table->slots);

		table->ntotal = newtable.ntotal;
		table->slots = newtable.slots;
	}
}


void elf_tabput(elf_tab *table, elf_val k, elf_val v) {
	langH_checkthreshold(table);
	elf_int slot = langH_hashin(table,k);
	/* todo: instead return an error here */
	if (slot == -1) LNOBRANCH;
	elf_tabslot *entry = table->slots + slot;
	if (!langH_slotiskey(table,slot)) {
		elf_int i = langA_variadd(table->v,1);
		table->v[i] = v;

		table->slots[slot].k = k;
		table->slots[slot].i = i;
		table->nslots ++;
	} else {
		table->v[entry->i] = v;
	}
}


elf_val langH_lookup(elf_tab *table, elf_val k) {
	elf_int slot = langH_hashin(table,k);
	if (langH_slotiskey(table,slot)) {
		return langH_slot2value(table,slot);
	}
	return (elf_val){TAG_NIL,0};
}


elf_int elf_tabtake(elf_tab *table, elf_val k) {
	LASSERT((k.tag == TAG_INT || k.tag == TAG_NUM) || k.s != 0);

	langH_checkthreshold(table);
	elf_int slot = langH_hashin(table,k);
	if (slot == -1) LNOBRANCH;
	if (!langH_slotiskey(table,slot)) {
		elf_int i = langA_variadd(table->v,1);
		table->v[i] = (elf_val){TAG_NIL};

		table->slots[slot].k = k;
		table->slots[slot].i = i;
		table->nslots ++;
	}
	return langH_slot2index(table,slot);
}


elf_int langH_iadd(elf_tab *table, elf_val v) {
	return langA_variadd(table->v,1);
}


void langH_add(elf_tab *table, elf_val v) {
	langA_varadd(table->v,v);
}


/*
** Meta Methods:
*/
int langH_length_(lRuntime *c) {
	elf_putint(c,elf_arrlen(((elf_tab*)c->f->obj)->v));
	return 1;
}


int langH_haskey_(lRuntime *c) {
	LASSERT(c->f->x == 1);
	elf_tab *table = (elf_tab*) c->f->obj;
	elf_val k = elf_getval(c,0);
	elf_putint(c,langH_slotiskey(table,langH_hashin(table,k)));
	return 1;
}


int langH_lookup_(lRuntime *c) {
	LASSERT(c->f->x == 1);
	elf_val k = elf_getval(c,0);
	elf_tab *table = (elf_tab*) c->f->obj;
	elf_putval(c,langH_lookup(table,k));
	return 1;
}


int langH_collisions_(lRuntime *c) {
	elf_tab *table = (elf_tab*) c->f->obj;
	elf_putint(c,table->ncollisions);
	return 1;
}


int langH_add_(lRuntime *R) {
	LASSERT(R->call->x >= 1);
	elf_tab *tab = (elf_tab *) R->call->obj;
	langH_add(tab,elf_getval(R,0));
	return 0;
}


int langH_idx_(lRuntime *R) {
	LASSERT(R->call->x >= 1);
	elf_tab *tab = (elf_tab *) R->call->obj;
	elf_int idx = elf_getint(R,0);
	elf_putval(R,tab->v[idx]);
	return 1;
}


int elf_tabput_(lRuntime *c) {
	int n = c->f->x;

	LASSERT(n >= 1 && n <= 2);

	elf_val k = elf_getval(c,0);

	elf_val j = {TAG_NIL};
	if (n == 2) j = elf_getval(c,1);

	elf_tab *table = (elf_tab*) c->f->obj;
	elf_tabput(table,k,j);
	return 0;
}


int langH_foreach_(lRuntime *R) {
	LASSERT(R->frame->x == 1);
	elf_tab *table = (elf_tab *) R->frame->obj;
	elf_checkcl(R,0);
	llocalid k = elf_stkput(R,1);
	llocalid v = elf_stkput(R,1);
	for (int i = 0; i < table->ntotal; ++ i) {
		elf_tabslot slot = table->slots[i];
		if (slot.k.tag != TAG_NIL) {
			R->stk[k] = slot.k;
			R->stk[v] = table->v[slot.i];
			/* call the iterator function with two arguments */
			/* todo: should yield boolean to signal whether to
			stop or not */
			elf_callfn(R,R->frame->obj,0,0,2,0);
		}
	}
	return 0;
}


void ftabs(FILE *io, int level) {
	while (level --) fprintf(io,"\t");
}
void elf_tabunload(FILE *io, elf_tab *tab, int level) {
	fprintf(io,"{");
	int nitems = 0;
	for (int i = 0; i < tab->ntotal; ++ i) {
		elf_tabslot slot = tab->slots[i];
		if (slot.k.tag != TAG_NIL) {
			if (nitems ++ != 0) fprintf(io,",");
			syslib_fpfv_(io,slot.k,ltrue);
			fprintf(io," = ");
			elf_val v = tab->v[slot.i];
			if (v.tag == TAG_TAB) {
				elf_tabunload(io,v.t,level+1);
			} else {
				syslib_fpfv_(io,v,ltrue);
			}
		}
	}
	fprintf(io,"}");
}


int langH_unload_(lRuntime *R) {
	elf_Handle io = elf_getsys(R,0);
	elf_tabunload(io,(elf_tab*)elf_getthis(R),0);
	return 0;
}


/* Some of the hash functions and comments
were borrowed from the great Sean Barrett */
elf_hashint elf_tabrehash(elf_hashint hash) {
	return ((hash) + ((hash) >> 6) + ((hash) >> 19));
}


#if 1
// FNV-1a
elf_hashint elf_tabhashstr (char *bytes) {
	elf_hashint hash = 2166136261u;
	while (*bytes) {
		hash ^= *bytes++;
		hash *= 16777619;
	}
	return hash;
}
#else
elf_hashint elf_tabhashstr(char *bytes) {
	elf_hashint hash = 0;
	while (*bytes) {
		hash = (hash << 7) + (hash >> 25) + *bytes++;
	}
	return hash + (hash >> 16);
}
#endif


elf_hashint elf_tabhashptr(Ptr *p) {
   // typically lacking in low bits and high bits
	elf_hashint hash = elf_tabrehash((elf_hashint)(elf_int)p);
	hash += hash << 16;

   // pearson's shuffle
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 2;
	hash += hash >> 15;
	hash ^= hash << 10;
	return elf_tabrehash(hash);
}


elf_bool elf_tabvaleq(elf_val *x, elf_val *y) {
	if (x->tag != y->tag) {
		return lfalse;
	}
	switch (x->tag) {
		case TAG_STR: {
			return elf_streq(x->s,y->s);
		}
		case TAG_SYS: case TAG_INT: case TAG_NUM:
		case TAG_TAB: case TAG_CLS: case TAG_BID: {
			return x->i == y->i;
		}
	}
	LNOBRANCH;
	return lfalse;
}


elf_int elf_tabhashval(elf_val v) {
	switch (v.tag) {
		case TAG_STR: {
			return v.s->hash;
		}
		case TAG_TAB: case TAG_CLS: case TAG_SYS:
		case TAG_INT: case TAG_NUM: case TAG_BID: {
			return elf_tabhashptr(v.p);
		}
	}
	LNOBRANCH;
	return lfalse;
}


#if 0
void langH_mergesort_(elf_tab *h, int level, elf_int x, elf_int z, elf_Proto *fn) {

	if (z-x <= 1) return;

	int y = x + (z-x)/2;
	langH_mergesort_(h,level+1,x,y,fn);
	langH_mergesort_(h,level+1,y,z,fn);

	elf_val *v = table->v;
	elf_tabslot *s = table->slots;

	for (;;) {
		int i = 0;

		for (; i < z-y && s[i].k.tag == TAG_NIL; ++i);
		if (s[i].k.tag == TAG_NIL) break;

		elf_tabslot xs = table->slots[i];

		for (; i < z-y && s[i].k.tag == TAG_NIL; ++i);
		if (s[i].k.tag == TAG_NIL) break;

		elf_tabslot ys = table->slots[i];

		elf_val a = v[xs.i];
		elf_val b = v[ys.i];

		int r = lang_callargs(c,fn,2,a,b);
		elf_val vr = pop();
		LASSERT(vr.tag == TAG_INT);

		if (vr.i) {
			elf_val t = h[x+i];
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


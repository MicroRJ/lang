/*
** See Copyright Notice In elf.h
** elf-tab.c
** Table
*/


elf_Table *elf_newtabmetatab(elf_Runtime *R) {
	elf_Table *tab = elf_locnewtab(R);
	elf_tabmfld(R,tab,"length",elf_tablength_);
	elf_tabmfld(R,tab,"tally",elf_tabtally_);
	elf_tabmfld(R,tab,"haskey",elf_tabhaskey_);
	elf_tabmfld(R,tab,"hashin",elf_tabput_);
	elf_tabmfld(R,tab,"lookup",elf_tablookup_);
	elf_tabmfld(R,tab,"iter",elf_tabforeach_);
	elf_tabmfld(R,tab,"collisions",elf_tabcollisions_);
	elf_tabmfld(R,tab,"unload",elf_tabunload_);
	elf_tabmfld(R,tab,"add",elf_tabadd_);
	elf_tabmfld(R,tab,"idx",elf_tabidx_);
	elf_tabmfld(R,tab,"xrem",elf_tabxrem_);
	return tab;
}


elf_Table *elf_newtablen(elf_Runtime *R, elf_int ntotal) {
	elf_Table *table = elf_newobj(R,OBJ_TAB,sizeof(elf_Table));
	if (R) table->obj.metatable = R->metatable_tab;

	table->ntotal = ntotal;
	table->nslots = 0;
	table->slots = elf_clearalloc(lHEAP,ntotal*sizeof(elf_tabslot));
	return table;
}


elf_Table *elf_newtab(elf_Runtime *R) {
	return elf_newtablen(R,4);
}


void elf_deltab(elf_Table *tab) {
	elf_delmem(lHEAP,tab->slots);
	elf_delvar(tab->array);
	tab->array = 0;
	tab->slots = 0;
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
elf_int elf_tabhashin(elf_Table *table, elf_val k) {
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


elf_int elf_tabslot2index(elf_Table *table, elf_int slot) {
	return table->slots[slot].i;
}


elf_val elf_tabslot2value(elf_Table *table, elf_int slot) {
	return table->v[table->slots[slot].i];
}


elf_bool elf_tabslotiskey(elf_Table *table, elf_int slot) {
	return slot >= 0 && table->slots[slot].k.tag != TAG_NIL;
}


void elf_tabslotsetkeyval(elf_Table *table, elf_int slot, elf_val k, elf_int i) {
	table->slots[slot].k = k;
	table->slots[slot].i = i;
}


void elf_tabcheckthreshold(elf_Table *table) {
	if (table->ntotal * 3 < table->nslots * 4) {
		// LDODEBUG( table->ncollisions = 0 );

		/* todo:
		Find a better strategy for incrementing
		the table size >> 1 << 2 */
		elf_Table newtable = * table;
		newtable.ntotal = table->ntotal << 2;
		if (newtable.ntotal < table->ntotal) LNOBRANCH;
		newtable.slots = elf_clearalloc(lHEAP,newtable.ntotal * sizeof(elf_tabslot));

		for (int i = 0; i < table->ntotal; ++ i) {
			elf_tabslot slot = table->slots[i];
			if (slot.k.tag == TAG_NIL) continue;

			elf_int newslot = elf_tabhashin(&newtable,slot.k);

			if (newslot == -1) LNOBRANCH;

			newtable.slots[newslot] = slot;
		}

		elf_delmem(lHEAP,table->slots);

		table->ntotal = newtable.ntotal;
		table->slots = newtable.slots;
	}
}


void elf_tabsetstrfld(elf_Table *tab, elf_String *key, elf_String *val) {
	elf_tabset(tab,elf_valstr(key),elf_valstr(val));
}


void elf_tabsetintfld(elf_Table *tab, elf_String *key, elf_int val) {
	elf_tabset(tab,elf_valstr(key),elf_valint(val));
}


void elf_tabsetnumfld(elf_Table *tab, elf_String *key, elf_num val) {
	elf_tabset(tab,elf_valstr(key),elf_valnum(val));
}


void elf_tabset(elf_Table *table, elf_val k, elf_val v) {
	elf_tabcheckthreshold(table);
	elf_int slot = elf_tabhashin(table,k);
	/* todo: instead return an error here */
	if (slot == -1) LNOBRANCH;
	elf_tabslot *entry = table->slots + slot;
	if (!elf_tabslotiskey(table,slot)) {
		elf_int i = elf_varaddi(table->v,1);
		table->v[i] = v;

		table->slots[slot].k = k;
		table->slots[slot].i = i;
		table->nslots ++;
	} else {
		table->v[entry->i] = v;
	}
}


elf_val elf_tablookup(elf_Table *table, elf_val k) {
	elf_int slot = elf_tabhashin(table,k);
	if (elf_tabslotiskey(table,slot)) {
		return elf_tabslot2value(table,slot);
	}
	return (elf_val){TAG_NIL,0};
}


elf_num elf_tabgetnum(elf_Table *tab, elf_String *key) {
	elf_val val = elf_tablookup(tab,elf_valstr(key));
	return elf_iton(val);
}


elf_int elf_tabgetint(elf_Table *tab, elf_String *key) {
	elf_val val = elf_tablookup(tab,elf_valstr(key));
	return elf_ntoi(val);
}


elf_String *elf_tabgetstr(elf_Table *tab, elf_String *key) {
	return elf_tablookup(tab,elf_valstr(key)).x_str;
}


elf_Table *elf_tabgettab(elf_Table *tab, elf_String *key) {
	return elf_tablookup(tab,elf_valstr(key)).x_tab;
}


elf_int elf_tabtake(elf_Table *table, elf_val k) {
	elf_ensure((k.tag == TAG_INT || k.tag == TAG_NUM) || k.s != 0);

	elf_tabcheckthreshold(table);
	elf_int slot = elf_tabhashin(table,k);
	if (slot == -1) LNOBRANCH;
	if (!elf_tabslotiskey(table,slot)) {
		elf_int i = elf_varaddi(table->v,1);
		table->v[i] = (elf_val){TAG_NIL};

		table->slots[slot].k = k;
		table->slots[slot].i = i;
		table->nslots ++;
	}
	return elf_tabslot2index(table,slot);
}


elf_int elf_tabiadd(elf_Table *table, elf_val v) {
	return elf_varaddi(table->v,1);
}


void elf_tabadd(elf_Table *table, elf_val v) {
	elf_varadd(table->v,v);
}


/* metatable */


int elf_tablength_(elf_Runtime *R) {
	elf_Table *tab = (elf_Table*) elf_getthis(R);
	elf_putint(R,elf_varlen(tab->v));
	return 1;
}


int elf_tabtally_(elf_Runtime *R) {
	elf_Table *tab = (elf_Table*) elf_getthis(R);
	elf_putint(R,elf_varlen(tab->v));
	return 1;
}


int elf_tabhaskey_(elf_Runtime *c) {
	elf_ensure(c->f->x == 1);
	elf_Table *table = (elf_Table*) elf_getthis(c);
	elf_val k = elf_getval(c,0);
	elf_putint(c,elf_tabslotiskey(table,elf_tabhashin(table,k)));
	return 1;
}


int elf_tablookup_(elf_Runtime *c) {
	elf_ensure(c->f->x == 1);
	elf_val k = elf_getval(c,0);
	elf_Table *table = (elf_Table*) c->f->obj;
	elf_putval(c,elf_tablookup(table,k));
	return 1;
}


int elf_tabcollisions_(elf_Runtime *c) {
	elf_Table *table = (elf_Table*) c->f->obj;
	elf_putint(c,table->ncollisions);
	return 1;
}


int elf_tabadd_(elf_Runtime *R) {
	elf_ensure(R->call->x >= 1);
	elf_Table *tab = (elf_Table *) R->call->obj;
	elf_tabadd(tab,elf_getval(R,0));
	return 0;
}


int elf_tabidx_(elf_Runtime *R) {
	elf_ensure(R->call->x >= 1);
	elf_Table *tab = (elf_Table *)elf_getthis(R);
	elf_int idx = elf_getint(R,0);
	elf_putval(R,tab->v[idx]);
	return 1;
}


int elf_tabxrem_(elf_Runtime *R) {
	elf_ensure(R->call->x >= 1);
	elf_Table *tab = (elf_Table *)elf_getthis(R);
	elf_int idx = elf_getint(R,0);
	elf_int min = elf_arrdecmin(tab->array);
	if (idx != min) {
		tab->array[idx] = tab->array[min];
	}
	return 1;
}


int elf_tabput_(elf_Runtime *c) {
	int n = c->f->x;

	elf_ensure(n >= 1 && n <= 2);

	elf_val k = elf_getval(c,0);

	elf_val j = {TAG_NIL};
	if (n == 2) j = elf_getval(c,1);

	elf_Table *table = (elf_Table*) c->f->obj;
	elf_tabset(table,k,j);
	return 0;
}


int elf_tabforeach_(elf_Runtime *R) {
	elf_ensure(R->frame->x == 1);
	elf_Table *table = (elf_Table *) R->frame->obj;
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
			elf_callex(R,R->frame->obj,0,0,2,0);
		}
	}
	return 0;
}


void ftabs(FILE *io, int level) {
	while (level --) fprintf(io,"\t");
}
void elf_tabunload(FILE *io, elf_Table *tab, int level) {
	fprintf(io,"{");
	int nitems = 0;
	for (int i = 0; i < tab->ntotal; ++ i) {
		elf_tabslot slot = tab->slots[i];
		if (slot.k.tag != TAG_NIL) {
			if (nitems ++ != 0) fprintf(io,",");
			elf_valfpf(io,slot.k,ltrue);
			fprintf(io," = ");
			elf_val v = tab->v[slot.i];
			if (v.tag == TAG_TAB) {
				elf_tabunload(io,v.t,level+1);
			} else {
				elf_valfpf(io,v,ltrue);
			}
		}
	}
	fprintf(io,"}");
}


int elf_tabunload_(elf_Runtime *R) {
	elf_Handle io = elf_getsys(R,0);
	elf_tabunload(io,(elf_Table*)elf_getthis(R),0);
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
		default: LNOBRANCH;
	}
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
		default: LNOBRANCH;
	}
	return lfalse;
}


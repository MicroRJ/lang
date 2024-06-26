/*
** See Copyright Notice In lang.h
** lgc.c
** Garbage Collector
*/

#define L_GC_THRESHOLD (llong) (8192)

void langGC_collect(Runtime *fs);


void langGC_pause(Runtime *fs) {
	fs->isgcpaused = ltrue;
}


void langGC_unpause(Runtime *fs) {
	fs->isgcpaused = lfalse;
}


void langGC_markpink(lObject *obj) {
	obj->gccolor = GC_PINK;
}


void langGC_markwhite(lObject *obj) {
	obj->gccolor = GC_WHITE;
}


void *langGC_allocobj(Runtime *rt, ObjectType type, llong length) {
	if (rt != 0) {
		if (!rt->isgcpaused) {
			if (rt->gcthreshold <= 0) {
				rt->gcthreshold = L_GC_THRESHOLD;
			}
			if (langA_varlen(rt->gc) >= rt->gcthreshold) {
				langGC_collect(rt);
				rt->gcthreshold *= 2;
			}
		}
	}


	lObject *obj = langM_clearalloc(lHEAP,length);
	obj->type = type;

	if (rt != 0) {
		langA_varadd(rt->gc,obj);
	}
	return obj;
}


void langGC_remobj(Runtime *fs, llong i) {
	lObject **gc = fs->gc;
	if (gc == 0) return;
	llong n = langA_varlen(gc);
	LASSERT(i >= 0 && i < n);
	gc[i] = gc[n-1];
	((Array*)(gc))[-1].min --;
}


lbool ttisobj(ValueName tag) {
	if (tag == VALUE_STRING) return ltrue;
	if (tag == VALUE_TABLE) return ltrue;
	if (tag == VALUE_FUNC) return ltrue;
	if (tag == VALUE_CUSTOM) return ltrue;
	return lfalse;
}


ValueName ttobj2val(ObjectType type) {
	switch(type) {
		case OBJ_CLOSURE: return VALUE_FUNC;
		case OBJ_TABLE: return VALUE_TABLE;
		case OBJ_STRING: return VALUE_STRING;
	}
	LNOCHANCE;
	return -1;
}


void langGC_deallocobj(lObject *j) {
	if (j->type == OBJ_TABLE) {
		langH_free((Table*)j);
	}
	langM_dealloc(lHEAP,j);
}


void langGC_deallocvalue(lValue v) {
	if (ttisobj(v.tag)) {
		langGC_deallocobj(v.j);
	}
}


lbool langGC_markvalue(lValue *v);
llong langGC_marktable(Table *table);


int langGC_markclosure(lClosure *cl) {
	int n = 0;
	int k;
	for (k=0; k<cl->fn.ncaches; ++k) {
		n += langGC_markvalue(&cl->caches[k]);
	}
	return n;
}


llong langGC_marktable(Table *table) {
	llong n = 0, k = 0;
	for (k=0; k<table->ntotal; ++k) {
		n += langGC_markvalue(&table->slots[k].k);
	}
	langA_varifor(table->v) {
		n += langGC_markvalue(&table->v[i]);
	}
	return n;
}


lbool langGC_markobj(lObject *obj) {
	/* Red and green objects are not collectable,
	and thus cannot be marked black or white,
	only white objects can be marked black,
	if not white, return whether it is already black */
	if (obj->gccolor != GC_WHITE) {
		return obj->gccolor == GC_BLACK;
	}

	obj->gccolor = GC_BLACK;

	if (obj->type == OBJ_CLOSURE) {
		return 1 + langGC_markclosure((lClosure*)obj);
	}
	if (obj->type == OBJ_TABLE) {
		return 1 + langGC_marktable((Table*)obj);
	}

	return 1;
}


lbool langGC_markvalue(lValue *v) {
	if (!ttisobj(v->tag)) return 0;
	return langGC_markobj(v->j);
}


llong langGC_mark(Runtime *fs) {
	/* mark global table first */
	llong n = langGC_marktable(fs->md->g);
	lValue *v;
	for (v = fs->s; v < fs->v; ++ v) {
		n += langGC_markvalue(v);
	}
	return n;
}


void langGC_collect(Runtime *fs) {
	llong n = langGC_mark(fs);
	(void) n;
	lang_loginfo("marked: %lli/%lli",langA_varlen(fs->gc),n);

	llong d = 0;
	langA_varifor(fs->gc) {
		lObject *it = fs->gc[i];
		if (it == 0) continue;
		if (it->gccolor == GC_BLACK) {
			it->gccolor = GC_WHITE;
		} else
		if (it->gccolor == GC_WHITE) {
			it->gccolor = GC_RED;
			d ++;
			// langGC_deallocobj(it);
			// langGC_remobj(fs,i);
		}
	}

	lang_loginfo("	=> collected: %lli",d);
}

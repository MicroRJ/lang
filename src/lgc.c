/*
** See Copyright Notice In lang.h
** lgc.c
** Garbage Collector
*/

#define L_GC_THRESHOLD (llongint) (8192)

void langGC_collect(lRuntime *fs);


void langGC_pause(lRuntime *fs) {
	fs->isgcpaused = ltrue;
}


void langGC_unpause(lRuntime *fs) {
	fs->isgcpaused = lfalse;
}


void langGC_markpink(lObject *obj) {
	obj->gccolor = GC_PINK;
}


void langGC_markwhite(lObject *obj) {
	obj->gccolor = GC_WHITE;
}


void *langGC_allocobj(lRuntime *rt, ObjectType type, llongint length) {
#if 0
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
#endif

	lObject *obj = langM_clearalloc(lHEAP,length);
	obj->type = type;

	if (rt != 0) {
		langA_varadd(rt->gc,obj);
	}
	return obj;
}


void langGC_remobj(lRuntime *fs, llongint i) {
	lObject **gc = fs->gc;
	if (gc == 0) return;
	llongint n = langA_varlen(gc);
	LASSERT(i >= 0 && i < n);
	gc[i] = gc[n-1];
	((Array*)(gc))[-1].min --;
}

void langGC_deallocobj(lObject *j) {
	if (j->type == OBJ_TABLE) {
		langH_free((lTable*)j);
	}
	langM_dealloc(lHEAP,j);
}


void langGC_deallocvalue(lValue v) {
	if (ttisobj(v.tag)) {
		langGC_deallocobj(v.j);
	}
}


lbool langGC_markvalue(lValue *v);
llongint langGC_marktable(lTable *table);


int langGC_markclosure(lClosure *cl) {
	int n = 0;
	int k;
	for (k=0; k<cl->fn.ncaches; ++k) {
		n += langGC_markvalue(&cl->caches[k]);
	}
	return n;
}


llongint langGC_marktable(lTable *table) {
	llongint n = 0, k = 0;
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
		return 1 + langGC_marktable((lTable*)obj);
	}

	return 1;
}


lbool langGC_markvalue(lValue *v) {
	if (!ttisobj(v->tag)) return 0;
	return langGC_markobj(v->j);
}


llongint langGC_mark(lRuntime *fs) {
	/* mark global table first */
	llongint n = langGC_marktable(fs->md->g);
	lValue *v;
	for (v = fs->s; v < fs->v; ++ v) {
		n += langGC_markvalue(v);
	}
	return n;
}


void langGC_collect(lRuntime *fs) {
	llongint n = langGC_mark(fs);
	(void) n;
	lang_loginfo("marked: %lli/%lli",langA_varlen(fs->gc),n);

	llongint d = 0;
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

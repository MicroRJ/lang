/*
** See Copyright Notice In lang.h
** lgc.c
** Garbage Collector
*/

#define L_GC_THRESHOLD (Integer) (8192)

void langGC_collect(Runtime *fs);


void langGC_pause(Runtime *fs) {
	fs->isgcpaused = True;
}


void langGC_unpause(Runtime *fs) {
	fs->isgcpaused = False;
}


void langGC_markpink(Object *obj) {
	obj->gccolor = GC_PINK;
}


void *langGC_allocobj(Runtime *rt, ObjectType type, Integer length) {
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


	Object *obj = langM_clearalloc(lHEAP,length);
	obj->type = type;

	if (rt != 0) {
		langA_varadd(rt->gc,obj);
	}
	return obj;
}


void langGC_remobj(Runtime *fs, Integer i) {
	LASSERT(i >= 0 && i < langA_varlen(fs->gc));
	fs->gc[i] = fs->gc[langA_varlen(fs->gc)-1];
	((Array*)(fs->gc))[-1].min --;
}


Bool ttisobj(ValueName tag) {
	if (tag == VALUE_STRING) return True;
	if (tag == VALUE_TABLE) return True;
	if (tag == VALUE_FUNC) return True;
	return False;
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


void langGC_deallocobj(Object *j) {
	if (j->type == OBJ_TABLE) {
		langH_free((Table*)j);
	}
	langM_dealloc(lHEAP,j);
}


void langGC_deallocvalue(Value v) {
	if (ttisobj(v.tag)) {
		langGC_deallocobj(v.j);
	}
}


Bool langGC_markvalue(Value *v);
Integer langGC_marktable(Table *table);


Integer langGC_marktable(Table *table) {
	Integer n = 0, k = 0;
	for (k=0; k<table->ntotal; ++k) {
		n += langGC_markvalue(&table->slots[k].k);
	}
	langA_varifor(table->v) {
		n += langGC_markvalue(&table->v[i]);
	}
	return n;
}


Bool langGC_markobj(Object *obj) {
	/* Red and green objects are not collectable,
	and thus cannot be marked black or white,
	only white objects can be marked black,
	if not white, return whether it is already black */
	if (obj->gccolor != GC_WHITE) {
		return obj->gccolor == GC_BLACK;
	}

	obj->gccolor = GC_BLACK;

	if (obj->type == OBJ_TABLE) {
		return 1 + langGC_marktable((Table*)obj);
	}

	return 1;
}


Bool langGC_markvalue(Value *v) {
	if (!ttisobj(v->tag)) return 0;
	return langGC_markobj(v->j);
}


Integer langGC_mark(Runtime *fs) {
	/* mark global table first */
	Integer n = langGC_marktable(fs->md->g);
	Value *v;
	for (v = fs->s; v < fs->v; ++ v) {
		n += langGC_markvalue(v);
	}
	return n;
}


void langGC_collect(Runtime *fs) {
	Integer n = langGC_mark(fs);
	(void) n;
	lang_loginfo("marked: %lli/%lli",langA_varlen(fs->gc),n);

	Integer d = 0;
	langA_varifor(fs->gc) {
		Object *it = fs->gc[i];
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

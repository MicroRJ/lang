/*
** See Copyright Notice In lang.h
** lgc.c
** Garbage Collector
*/

// #define L_GC_THRESHOLD (elf_int) (8192)
#define L_GC_THRESHOLD (elf_int) (512)

void elf_collect(lRuntime *fs);


void langGC_pause(lRuntime *fs) {
	fs->isgcpaused = ltrue;
}


void langGC_unpause(lRuntime *fs) {
	fs->isgcpaused = lfalse;
}


void langGC_markpink(elf_obj *obj) {
	obj->gccolor = GC_PINK;
}


void langGC_markwhite(elf_obj *obj) {
	obj->gccolor = GC_WHITE;
}


void *elf_newobj(lRuntime *rt, ObjectType type, elf_int length) {
#if 1
	if (rt != 0) {
		if (!rt->isgcpaused) {
			if (rt->gcthreshold <= 0) {
				rt->gcthreshold = L_GC_THRESHOLD;
			}
			if (elf_arrlen(rt->gc) >= rt->gcthreshold) {
				elf_collect(rt);
				rt->gcthreshold *= 2;
			}
		}
	}
#endif

	elf_obj *obj = elf_newmemzro(lHEAP,length);
	obj->type = type;

	if (rt != 0) {
		langA_varadd(rt->gc,obj);
	}
	return obj;
}


void elf_remobj(lRuntime *fs, elf_int i) {
	elf_obj **gc = fs->gc;
	if (gc == 0) return;
	elf_int n = elf_arrlen(gc);
	LASSERT(i >= 0 && i < n);
	gc[i] = gc[n-1];
	((Array*)(gc))[-1].min --;
}


void elf_delobj(elf_obj *obj) {
	if (obj->type == OBJ_TAB) {
		elf_deltab((elf_tab*)obj);
	} else elf_delmem(lHEAP,obj);
}


void elf_delval(elf_val v) {
	if (elf_tagisobj(v.tag)) elf_delobj(v.j);
}


elf_bool elf_markval(elf_val *v);
elf_int elf_marktab(elf_tab *table);


elf_int elf_markcl(elf_Closure *cl) {
	elf_int n = 0, k;
	for (k=0; k<cl->fn.ncaches; ++k) {
		n += elf_markval(&cl->caches[k]);
	}
	return n;
}


elf_int elf_marktab(elf_tab *table) {
	if (table->ntotal > 1024) {
		lang_logdebug("marked high count table: %lli/%lli",
		table->nslots,table->ntotal);
	}
	elf_int n = 0, k;
	for (k=0; k<table->ntotal; ++k) {
		n += elf_markval(&table->slots[k].k);
	}
	elf_forivar(table->v) {
		n += elf_markval(&table->v[i]);
	}
	return n;
}


elf_bool elf_markobj(elf_obj *obj) {
	if (obj == lnil) return 0;
	/* Red and green objects are not collectable,
	and thus cannot be marked black or white,
	only white objects can be marked black,
	if not white, return whether it is already black */
	if (obj->gccolor != GC_WHITE) {
		return obj->gccolor == GC_BLACK;
	}
	obj->gccolor = GC_BLACK;
	if (obj->type == OBJ_CLOSURE) {
		return 1 + elf_markcl((elf_Closure*)obj);
	}
	if (obj->type == OBJ_TAB) {
		return 1 + elf_marktab((elf_tab*)obj);
	}
	return 1;
}


elf_bool elf_markval(elf_val *v) {
	if (!elf_tagisobj(v->tag)) {
	 	return 0;
	} else {
		return elf_markobj(v->x_obj);
	}
}


elf_int elf_markall(lRuntime *R) {
	elf_int num;
	elf_val *val;
	num = elf_marktab(R->md->g);
	for (val = R->stk; val < R->top; ++ val) {
		num += elf_markval(val);
	}
	return num;
}


void elf_collect(lRuntime *R) {
	elf_int num = elf_markall(R);
	elf_int ngc = elf_arrlen(R->gc);
	elf_int tbf = ngc-num;
	lang_logdebug("marked: %lli/%lli -> %lli ~(%%%.1f)",num,ngc
	, num, (elf_num)(tbf)/(elf_num)(ngc) * 100.);
	elf_int nwo = 0;

	elf_forivar(R->gc) {
		elf_obj *it = R->gc[i];
		if (it == lnil) continue;
		if (it->gccolor == GC_RED) __debugbreak();
		if (it->gccolor == GC_BLACK) {
			num --;
			it->gccolor = GC_WHITE;
		} else
		if (it->gccolor == GC_WHITE) {
			it->gccolor = GC_RED;
			tbf --;
			elf_delobj(it);
			elf_remobj(R,i);
		} else nwo ++;
	}

	lang_logdebug("	=> leaked: %lli, %lli, %lli",tbf,nwo,num);
}

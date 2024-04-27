/*
** See Copyright Notice In elf.h
** lgc.c
** Garbage Collector
*/


/* we have to factor in additional heuristics
for this, sometimes memory usage isn't what
boggles the GC, instead is the sheer quantity
of objects, so that's something we have to
take into account, and most of the time you
make small allocations tightly, so that's where
most of the spikes occur */
#define L_GC_THRESHOLD_MIN (elf_int) MEGABYTES(2)
#define L_GC_THRESHOLD_MAX (elf_int) MEGABYTES(4)


void elf_collect(elf_Runtime *fs);


void elf_gcpause(elf_Runtime *fs) {
	fs->gcflags = ltrue;
}


void elf_gcresume(elf_Runtime *fs) {
	fs->gcflags = lfalse;
}


void *elf_newobj(elf_Runtime *R, ObjectType type, elf_int tell) {
	/* this is temporary! */
	if (R != 0) {
		R->gcmemory += tell;
		if (!R->gcflags) {
	if (R->gcthreshold <= 0) {
		R->gcthreshold = L_GC_THRESHOLD_MIN;
	}
	if (R->gcmemory >= R->gcthreshold) {
		R->gcthreshold *= 2;
		if (R->gcthreshold > L_GC_THRESHOLD_MAX) {
			R->gcthreshold = L_GC_THRESHOLD_MAX;
		}
		elf_collect(R);
	}
		}
	}

	elf_Object *obj = elf_newmemzro(lHEAP,tell);
	obj->type = type;
	obj->tell = tell;
	LDODEBUG(
		obj->headtrap = FLYTRAP;
		obj->tailtrap = FLYTRAP;
	);
	if (R != 0) {
		elf_varadd(R->gc,obj);
		// LDODEBUG(elf_arrfori(R->gc) {
		// 	if (R->gc[i]->headtrap != FLYTRAP) LNOBRANCH;
		// 	if (R->gc[i]->tailtrap != FLYTRAP) LNOBRANCH;
		// });
	}
	return obj;
}


void elf_remobj(elf_Runtime *fs, elf_int i) {
	elf_Object **gc = fs->gc;
	if (gc == 0) return;
	elf_int n = elf_varlen(gc);
	elf_assert(i >= 0 && i < n);
	gc[i] = gc[n-1];
	((Array*)(gc))[-1].min --;
}


void elf_delobj(elf_Runtime *R, elf_Object *obj) {
	if (obj != lnil) {
		R->gcmemory -= obj->tell;
	if (obj->type == OBJ_TAB) {
		elf_deltab((elf_Table*)obj);
	}
		elf_delmem(lHEAP,obj);
	}
}

elf_bool elf_markval(elf_val *v);
elf_int elf_marktab(elf_Table *table);


/* todo: remove this function */
elf_int elf_markcl(elf_Closure *cl) {
	elf_int n = 0, k;
	for (k=0; k<cl->fn.ncaches; ++k) {
		n += elf_markval(&cl->caches[k]);
	}
	return n;
}


/* todo: remove this function */
elf_int elf_marktab(elf_Table *table) {
	if (table->ntotal > 1024) {
		elf_logdebug("marked high count table: %lli/%lli"
		, table->nslots,table->ntotal);
	}
	elf_int n = 0, k;
	for (k=0; k<table->ntotal; ++k) {
		n += elf_markval(&table->slots[k].k);
	}
	elf_arrfori(table->v) {
		n += elf_markval(&table->v[i]);
	}
	return n;
}


elf_bool elf_markobj(elf_Object *obj) {
	if (obj == lnil || obj->gccolor != GC_WHITE) return 0;
	obj->gccolor = GC_BLACK;
	if (obj->type == OBJ_CLOSURE) {
		return 1 + elf_markcl((elf_Closure*)obj);
	}
	if (obj->type == OBJ_TAB) {
		return 1 + elf_marktab((elf_Table*)obj);
	}
	return 1;
}


elf_bool elf_markval(elf_val *v) {
	return elf_tagisobj(v->tag) ? elf_markobj(v->x_obj) : lfalse;
}


elf_int elf_markall(elf_Runtime *R) {
	elf_int num = elf_markobj((elf_Object*)R->M->g);
	for (elf_val *val = R->stk; val < R->top; ++ val) {
		num += elf_markval(val);
	}
	return num;
}


void elf_collect(elf_Runtime *R) {
	elf_int num = elf_markall(R);

#if defined(LLOGGING)
	elf_int time_ = elf_clocktime();
	elf_int ngc = elf_varlen(R->gc);
	elf_int tbf = ngc-num;
	elf_int nwo = 0;
	elf_logdebug("tbf: %lli/%lli -> %lli",tbf,ngc,num);
#endif

	for (int i = 0; i < elf_varlen(R->gc); i ++) {
		elf_Object *it = R->gc[i];
		LDODEBUG(
			if (it->headtrap != FLYTRAP) LNOBRANCH;
			if (it->tailtrap != FLYTRAP) LNOBRANCH;
		);

		if (it == lnil) continue;
		if (it->gccolor == GC_RED) {
			elf_logerror("internal error: gc failed");
			elf_debugger();
		}
		if (it->gccolor == GC_BLACK) {
	#if defined(LLOGGING)
			num --;
	#endif
			it->gccolor = GC_WHITE;
		} else
		if (it->gccolor == GC_WHITE) {
	if (it == (elf_Object*) R->M->g) {
		elf_logerror("internal error: gc failed");
		elf_debugger();
	}
			it->gccolor = GC_RED;
	#if defined(LLOGGING)
			tbf --;
	#endif
			/* todo: instead of doing it this way, remove
			objects in large ranges */
			elf_delobj(R,it);
			elf_remobj(R,i);
			-- i;
		}
	#if defined(LLOGGING)
		else nwo ++;
	#endif
	}

	elf_logdebug("	(%f) => leaked: %lli, %lli, %lli"
	, elf_timediffs(time_),tbf,nwo,num);
}

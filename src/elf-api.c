/*
** See Copyright Notice In elf.h
** elf-api.c
** Main user API
*/



elf_api elf_Object *elf_getthis(elf_ThreadState *R) {
	return R->call->obj;
}


elf_api llocalid elf_stklen(elf_ThreadState *R) {
	return R->top - R->call->locals;
}


elf_api elf_val elf_getval(elf_ThreadState *R, llocalid x) {
	return R->call->locals[x];
}


elf_api elf_String *elf_getstr(elf_ThreadState *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_STR) {
		elf_throw(R,NO_BYTE,elf_tpf("expected string at local %i",x));
		LNOBRANCH;
	}
	return v.s;
}


elf_api elf_Object *elf_getobj(elf_ThreadState *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && !elf_tagisobj(v.tag)) {
		elf_throw(R,NO_BYTE,elf_tpf("expected object at local %i",x));
		LNOBRANCH;
	}
	return v.x_obj;
}


elf_api elf_Table *elf_gettab(elf_ThreadState *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_TAB) {
		elf_throw(R,NO_BYTE,elf_tpf("expected table at local %i",x));
		LNOBRANCH;
	}
	return v.x_tab;
}


elf_api void elf_checkcl(elf_ThreadState *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_CLS) {
		elf_throw(R,NO_BYTE,elf_tpf("expected closure at local %i",x));
		LNOBRANCH;
	}
}


elf_api elf_Closure *elf_getcls(elf_ThreadState *R, llocalid x) {
	elf_checkcl(R,x);
	return R->call->locals[x].f;
}


elf_api elf_Handle elf_getsys(elf_ThreadState *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_SYS) {
		elf_throw(R,NO_BYTE,elf_tpf("expected system object at local %i",x));
		LNOBRANCH;
	}
	return v.h;
}


elf_api elf_String *elf_checkstr(elf_ThreadState *R, llocalid x) {
	elf_ensure(R->call->locals[x].tag == TAG_STR);
	return R->call->locals[x].s;
}


elf_api elf_int elf_getint(elf_ThreadState *R, int x) {
	elf_val v = R->call->locals[x];
	if (v.tag == TAG_NUM) {
		return (elf_int) v.n;
	}
	elf_ensure(v.tag == TAG_INT);
	return v.i;
}


elf_api elf_num elf_getnum(elf_ThreadState *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag == TAG_INT) return (elf_num) v.i;
	if (v.tag != TAG_NUM) {
		elf_throw(R,NO_BYTE,elf_tpf("expected number at local %i",x));
		LNOBRANCH;
	}
	return v.n;
}


llocalid elf_stkput(elf_ThreadState *R, int n) {
	llocalid stkptr = R->top - R->stk;
	if (stkptr <= R->stklen) {
		R->top += n;
	} else LNOBRANCH;
	return stkptr;
}


llocalid elf_locval(elf_ThreadState *R, elf_val v) {
	*R->top = v;
	return elf_stkput(R,1);
}

#define _INC_TOP do {\
	llocalid __i = R->top ++ - R->stk;\
	elf_ensure(__i < R->stklen);\
} while(0)

void elf_locnil(elf_ThreadState *R) {
	R->top->tag = TAG_NIL;
	_INC_TOP;
}


void elf_locint(elf_ThreadState *R, elf_int i) {
	R->top->tag = TAG_INT;
	R->top->i = i;
	_INC_TOP;
}


void elf_locnum(elf_ThreadState *R, elf_num n) {
	R->top->tag = TAG_NUM;
	R->top->n = n;
	_INC_TOP;
}


void elf_locsys(elf_ThreadState *R, elf_Handle h) {
	R->top->tag = TAG_SYS;
	R->top->h = h;
	_INC_TOP;
}


void elf_locstr(elf_ThreadState *R, elf_String *s) {
	R->top->tag = TAG_STR;
	R->top->s = s;
	// if (s->obj.gccolor == GC_BLACK) s->obj.gccolor = GC_WHITE;
	_INC_TOP;
}


elf_val *elf_gettop(elf_ThreadState *R) {
	return R->top;
}


void elf_settop(elf_ThreadState *R, elf_val *top) {
	R->top = top;
}


llocalid elf_loccls(elf_ThreadState *R, elf_Closure *cl) {
	R->top->tag = TAG_CLS;
	R->top->f   = cl;
	// if (cl->obj.gccolor == GC_BLACK) cl->obj.gccolor = GC_WHITE;
	llocalid id = R->top - R->call->locals;
	_INC_TOP;
	return id;
}


void elf_locobj(elf_ThreadState *R, elf_Object *obj) {
	R->top->tag = TAG_OBJ;
	R->top->x_obj = obj;
	_INC_TOP;
}


void elf_loctab(elf_ThreadState *R, elf_Table *tab) {
	R->top->tag = TAG_TAB;
	R->top->x_tab = tab;
	_INC_TOP;
}


void elf_locbinding(elf_ThreadState *R, lBinding b) {
	R->top->tag = TAG_BID;
	R->top->c = b;
	_INC_TOP;
}


elf_Object *elf_newlocobj(elf_ThreadState *R, elf_int tell) {
	elf_Object *obj = elf_newobj(R,OBJ_CUSTOM,tell);
	elf_locobj(R,obj);
	return obj;
}


elf_Table *elf_newloctab(elf_ThreadState *R) {
	elf_Table *tab = elf_newtab(R);
	elf_loctab(R,tab);
	return tab;
}


elf_String *elf_newlocstr(elf_ThreadState *R, char *junk) {
	elf_String *s = elf_newstr(R,junk);
	elf_locstr(R,s);
	return s;
}


llocalid elf_newloccls(elf_ThreadState *R, elf_Proto fn) {
	elf_Closure *cl = elf_newcls(R,fn);
	elf_ensure(elf_stklen(R) >= fn.ncaches);
	R->top -= fn.ncaches;
	int i;
	for (i=0; i<fn.ncaches; ++i) {
		cl->caches[i] = R->top[i];
	}
	return elf_loccls(R,cl);
}

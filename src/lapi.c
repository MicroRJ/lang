/*
** See Copyright Notice In elf.h
** lapi.c
** Main user API
*/


lapi elf_Object *elf_getthis(elf_Runtime *R) {
	return R->call->obj;
}


lapi llocalid elf_stklen(elf_Runtime *R) {
	return R->top - R->call->locals;
}


lapi elf_val elf_getval(elf_Runtime *R, llocalid x) {
	return R->call->locals[x];
}


lapi elf_String *elf_getstr(elf_Runtime *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_STR) {
		elf_throw(R,NO_BYTE,S_tpf("expected string at local %i",x));
		LNOBRANCH;
	}
	return v.s;
}


lapi elf_Object *elf_getobj(elf_Runtime *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && !elf_tagisobj(v.tag)) {
		elf_throw(R,NO_BYTE,S_tpf("expected object at local %i",x));
		LNOBRANCH;
	}
	return v.x_obj;
}


lapi elf_Table *elf_gettab(elf_Runtime *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_TAB) {
		elf_throw(R,NO_BYTE,S_tpf("expected table at local %i",x));
		LNOBRANCH;
	}
	return v.x_tab;
}


lapi void elf_checkcl(elf_Runtime *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_CLS) {
		elf_throw(R,NO_BYTE,S_tpf("expected closure at local %i",x));
		LNOBRANCH;
	}
}


lapi elf_Closure *elf_getcls(elf_Runtime *R, llocalid x) {
	elf_checkcl(R,x);
	return R->call->locals[x].f;
}


lapi elf_Handle elf_getsys(elf_Runtime *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_SYS) {
		elf_throw(R,NO_BYTE,S_tpf("expected system object at local %i",x));
		LNOBRANCH;
	}
	return v.h;
}


lapi elf_String *elf_checkstr(elf_Runtime *R, llocalid x) {
	elf_assert(R->call->locals[x].tag == TAG_STR);
	return R->call->locals[x].s;
}


lapi elf_int elf_getint(elf_Runtime *R, int x) {
	elf_val v = R->call->locals[x];
	if (v.tag == TAG_NUM) {
		return (elf_int) v.n;
	}
	elf_assert(v.tag == TAG_INT);
	return v.i;
}


lapi elf_num elf_getnum(elf_Runtime *R, llocalid x) {
	elf_val v = R->call->locals[x];
	if (v.tag == TAG_INT) return (elf_num) v.i;
	if (v.tag != TAG_NUM) {
		elf_throw(R,NO_BYTE,S_tpf("expected number at local %i",x));
		LNOBRANCH;
	}
	return v.n;
}


llocalid elf_stkput(elf_Runtime *R, int n) {
	llocalid stkptr = R->top - R->stk;
	if (stkptr <= R->stklen) {
		R->top += n;
	} else LNOBRANCH;
	return stkptr;
}


llocalid elf_putval(elf_Runtime *R, elf_val v) {
	*R->top = v;
	return elf_stkput(R,1);
}


void elf_putnil(elf_Runtime *R) {
	R->top->tag = TAG_NIL;
	++ R->top;
}


void elf_putint(elf_Runtime *R, elf_int i) {
	R->top->tag = TAG_INT;
	R->top->i = i;
	++ R->top;
}


void elf_putnum(elf_Runtime *R, elf_num n) {
	R->top->tag = TAG_NUM;
	R->top->n = n;
	++ R->top;
}


void elf_putsys(elf_Runtime *R, elf_Handle h) {
	R->top->tag = TAG_SYS;
	R->top->h = h;
	++ R->top;
}


void elf_putstr(elf_Runtime *R, elf_String *s) {
	R->top->tag = TAG_STR;
	R->top->s = s;
	++ R->top;
}


llocalid elf_putcl(elf_Runtime *R, elf_Closure *cl) {
	R->top->tag = TAG_CLS;
	R->top->f   = cl;
	return R->top ++ - R->call->locals;
}


void elf_puttab(elf_Runtime *R, elf_Table *t) {
	R->top->tag = TAG_TAB;
	R->top->t = t;
	++ R->top;
}


void elf_putbinding(elf_Runtime *R, lBinding b) {
	R->top->tag = TAG_BID;
	R->top->c = b;
	++ R->top;
}


elf_Table *elf_pushnewtab(elf_Runtime *R) {
	elf_Table *t = elf_newtab(R);
	elf_puttab(R,t);
	return t;
}


elf_String *elf_pushnewstr(elf_Runtime *R, char const *junk) {
	elf_String *s = elf_newstr(R,junk);
	elf_putstr(R,s);
	return s;
}


llocalid elf_pushnewcl(elf_Runtime *R, elf_Proto fn) {
	elf_Closure *cl = elf_newcl(R,fn);
	elf_assert(elf_stklen(R) >= fn.ncaches);
	R->top -= fn.ncaches;
	int i;
	for (i=0; i<fn.ncaches; ++i) {
		cl->caches[i] = R->top[i];
	}
	return elf_putcl(R,cl);
}

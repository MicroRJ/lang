/*
** See Copyright Notice In lang.h
** ldoaux.c
** Runtime
*/


LAPI Value lang_T(Table *t) {
	Value v = (Value){VALUE_TABLE};
	v.t = t;
	return v;
}


LAPI Value lang_C(CFunc c) {
	Value v = (Value){VALUE_CFUN};
	v.c = c;
	return v;
}


LAPI Value lang_S(String *s) {
	Value v = (Value){VALUE_STRING};
	v.s = s;
	return v;
}


LAPI Value lang_F(Closure *f) {
	Value v = (Value){VALUE_FUNC};
	v.f = f;
	return v;
}


LAPI Value lang_I(Integer i) {
	Value v = (Value){VALUE_INTEGER};
	v.i = i;
	return v;
}


LAPI Value lang_N(Number n) {
	Value v = (Value){VALUE_NUMBER};
	v.n = n;
	return v;
}


LAPI Integer langR_popI(Runtime *c) {
	Value v = *(-- c->v);
	LASSERT(v.tag == VALUE_INTEGER);
	return v.i;
}


LAPI Integer langR_stacksize(Runtime *c) {
	Integer n = c->v - c->s;
	if (c->f->cl != 0) n -= c->f->cl->fn.nlocals;
	return n;
}


LAPI Value langR_loadV(Runtime *c, int x) {
	return c->f->l[x];
}


LAPI String *langR_loadS(Runtime *c, int x) {
	Value v = c->f->l[x];
	LASSERT(v.tag == VALUE_STRING);
	return v.s;
}


LAPI Closure *langR_loadF(Runtime *c, int x) {
	Value v = c->f->l[x];
	LASSERT(v.tag == VALUE_FUNC);
	return v.f;
}


LAPI Integer langR_loadI(Runtime *c, int x) {
	Value v = c->f->l[x];
	if (v.tag == VALUE_NUMBER) {
		return (Integer) v.n;
	}
	LASSERT(v.tag == VALUE_INTEGER);
	return v.i;
}


void langR_pushV(Runtime *c, Value v) {
	*(c->v ++) = v;
}


void langR_pushNil(Runtime *c) {
	c->v->tag = VALUE_NONE;
	++ c->v;
}


void langR_pushI(Runtime *c, Integer i) {
	c->v->tag = VALUE_INTEGER;
	c->v->i = i;
	++ c->v;
}


void langR_pushN(Runtime *c, Number n) {
	c->v->tag = VALUE_NUMBER;
	c->v->n = n;
	++ c->v;
}


void langR_pushS(Runtime *c, String *s) {
	c->v->tag = VALUE_STRING;
	c->v->s = s;
	++ c->v;
}


void langR_pushF(Runtime *c, Closure *f) {
	c->v->tag = VALUE_FUNC;
	c->v->f = f;
	++ c->v;
}


void langR_pushH(Runtime *c, Table *t) {
	c->v->tag = VALUE_TABLE;
	c->v->t = t;
	++ c->v;
}


Table *langR_pushnewR(Runtime *c) {
	Table *t = langH_new(c);
	langR_pushH(c,t);
	return t;
}


String *langR_pushnewS(Runtime *c, char const *contents) {
	String *s = langS_new(c,contents);
	langR_pushS(c,s);
	return s;
}


Closure *langR_pushnewF(Runtime *c, Proto fn) {
	Closure *cl = langF_newclosure(c,fn);
	LASSERT(langR_stacksize(c) >= fn.ncaches);
	c->v -= fn.ncaches;
	int i;
	for (i=0; i<fn.ncaches; ++i) {
		cl->caches[i] = c->v[i];
	}
	langR_pushF(c,cl);
	return cl;
}
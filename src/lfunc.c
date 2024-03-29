/*
** See Copyright Notice In lang.h
** lfunc.c
** ?
*/


lapi Closure *langF_newclosure(Runtime *rt, Proto fn) {
	LASSERT(fn.ncaches >= 0);

	llong length = sizeof(Closure) + sizeof(Value) * (fn.ncaches-1);

	Closure *cl = langGC_allocobj(rt,OBJ_CLOSURE,length);
	cl->fn = fn;
	return cl;
}

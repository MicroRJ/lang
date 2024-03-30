/*
** See Copyright Notice In lang.h
** lfunc.c
** ?
*/


lapi lClosure *langF_newclosure(Runtime *rt, Proto fn) {
	LASSERT(fn.ncaches >= 0);

	llong length = sizeof(lClosure) + sizeof(lValue) * (fn.ncaches-1);

	lClosure *cl = langGC_allocobj(rt,OBJ_CLOSURE,length);
	cl->fn = fn;
	return cl;
}

/*
** See Copyright Notice In lang.h
** lfunc.c
** ?
*/


lapi lClosure *langF_newclosure(lRuntime *rt, lProto fn) {
	LASSERT(fn.ncaches >= 0);

	llongint length = sizeof(lClosure) + sizeof(lValue) * (fn.ncaches-1);

	lClosure *cl = langGC_allocobj(rt,OBJ_CLOSURE,length);
	cl->fn = fn;
	return cl;
}

/*
** See Copyright Notice In lang.h
** lfunc.c
** ?
*/


LAPI Closure *langF_newclosure(Runtime *fs, Proto fn) {
	LASSERT(fn.ncaches >= 0);

	Integer length = sizeof(Closure) + sizeof(Value) * (fn.ncaches-1);

	Closure *cl = langGC_allocobj(fs,OBJ_CLOSURE,length);
	cl->fn = fn;
	return cl;
}

/*
** See Copyright Notice In lang.h
** ljitlib.c
** JIT Experiments Addon Library
*/


/* this inclusion is temporary */
#include <src/ljittoy.h>
#include <src/ljittoy.c>



/* -- Clearly, this isn't how you
- do jitting, this is just me playing
- around */
lBinding jit(lModule *md, lProto fn);
int jitlib_jit(lRuntime *rt) {
	lValue v = lang_loadfile(rt,0);
	lBinding b = jit(rt->md,v.f->fn);
	lang_pushbinding(rt,b);
	// __debugbreak();
	(void) v;
	return 1;
}


lapi void jitlib_load(lRuntime *rt) {
	lModule *md = rt->md;
	lang_addglobal(md,lang_pushnewS(rt,"jit"),lang_C(jitlib_jit));
}

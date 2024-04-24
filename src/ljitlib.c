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
lBinding jit(elf_Module *md, elf_Proto fn);
int jitlib_jit(lRuntime *rt) {
	elf_val v = elf_loadfile(rt,0);
	lBinding b = jit(rt->md,v.f->fn);
	elf_putbinding(rt,b);
	// __debugbreak();
	(void) v;
	return 1;
}


lapi void jitlib_load(lRuntime *rt) {
	elf_Module *md = rt->md;
	lang_addglobal(md,elf_pushnewstr(rt,"jit"),lang_C(jitlib_jit));
}

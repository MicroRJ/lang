/*
** See Copyright Notice In elf.h
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
int jitlib_jit(elf_Runtime *rt) {
	elf_val v = elf_loadfile(rt,0);
	lBinding b = jit(rt->md,v.f->fn);
	elf_locbinding(rt,b);
	// __debugbreak();
	(void) v;
	return 1;
}


elf_api void jitlib_load(elf_Runtime *rt) {
	elf_Module *md = rt->md;
	lang_addglobal(md,elf_newlocstr(rt,"jit"),elf_valbid(jitlib_jit));
}

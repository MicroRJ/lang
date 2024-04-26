/*
** See Copyright Notice In elf.h
** lfunc.c
** ?
*/


lapi elf_Closure *elf_newcl(elf_Runtime *rt, elf_Proto fn) {
	elf_assert(fn.ncaches >= 0);

	elf_int length = sizeof(elf_Closure) + sizeof(elf_val) * (fn.ncaches-1);

	elf_Closure *cl = elf_newobj(rt,OBJ_CLOSURE,length);
	cl->fn = fn;
	return cl;
}

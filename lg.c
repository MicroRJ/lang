/*
** See Copyright Notice In lang.h
** lg.c
** Compiles and runs .lang files
*/


#include <lang.h>


int main(int n, char **c) {
	(void) n;

	elf_inimem();

	elf_Module md = {0};

	lRuntime rt = {&md};
	rt.logging = lfalse;
	rt.stklen = 4096;
	rt.stk = rt.top = elf_newmemzro(lHEAP,sizeof(elf_val) * rt.stklen);
	rt.metatable_str = elf_newstrmetatab(&rt);
	rt.metatable_tab = elf_newtabmetatab(&rt);
	elf_CallFrame root = {0};
	root.base = rt.top;
	rt.frame = &root;

	md.g = elf_pushnewtab(&rt);

	/* todo: eventually we'll call this from the source file,
	each lib will be built independently, or maybe we can add
	a flag so that we don't load these libraries. */
	netlib_load(&rt);
	syslib_load(&rt);
	tstlib_load(&rt);
	crtlib_load(&rt);

	elf_CallFrame frame = {0};
	frame.base = rt.top;
	rt.frame = &frame;
	elf_str *filename = elf_pushnewstr(&rt,c[1]);
	/* todo: mark this as trap to figure out why
	gc is freeing this up! */
	filename->obj.gccolor = GC_PINK;
	elf_FileState fs = {0};
	elf_loadfile(&rt,&fs,filename,0,0);

	FILE *file;
	fopen_s(&file,S_tpf("%s.module.ignore",filename->c),"wb");
	if (file != lnil) lang_dumpmodule(&md,file);
	fclose(file);

	printf("exited\n");
	return 0;
}


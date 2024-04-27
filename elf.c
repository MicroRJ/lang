/*
** See Copyright Notice In elf.h
** elf.c
** Compiles and runs .elf files
*/


#include "elf.h"


int main(int n, char **c) {
	(void) n;

	elf_inimem();

	elf_Module M = {0};
	elf_Runtime R = {&M};
	R.logging = lfalse;
	R.stklen = 4096;
	R.stk = R.top = elf_newmemzro(lHEAP,sizeof(elf_val)*R.stklen);
	R.metatable_str = elf_newstrmetatab(&R);
	R.metatable_tab = elf_newtabmetatab(&R);
	elf_CallFrame root = {0};
	root.base = R.top;
	R.frame = &root;
	M.g = elf_pushnewtab(&R);
	/* todo: eventually we'll load these from the
	source code, each lib will be built independently,
	or maybe we can add a flag so that we don't load
	these libraries. */
	netlib_load(&R);
	syslib_load(&R);
	tstlib_load(&R);
	crtlib_load(&R);

	elf_CallFrame frame = {0};
	frame.base = R.top;
	R.frame = &frame;
	elf_String *filename = elf_pushnewstr(&R,c[1]);
	/* todo: mark this as trap to figure out why
	gc is freeing this up! */
	filename->obj.gccolor = GC_PINK;
	elf_FileState fs = {0};
	elf_loadfile(&R,&fs,filename,0,0);

	FILE *file;
	fopen_s(&file,S_tpf("%s.module.ignore",filename->c),"wb");
	if (file != lnil) lang_dumpmodule(&M,file);
	fclose(file);

	printf("exited\n");
	return 0;
}


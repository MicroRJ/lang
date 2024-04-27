/*
** See Copyright Notice In elf.h
** elf.c
** Compiles and runs .elf files
*/


#include "elf.h"


int main(int n, char **c) {
	(void) n;

	#if defined(PLATFORM_WEB)
	sys_consolelog(ELF_LOGDBUG,"WEB!");
	int *var = {0};
	elf_varadd(var,1);
	if (var[0] != 1) printf("FAILED: '%i': var.add!\n",var[0]);
	if (elf_varlen(var) != 1) printf("FAILED: '%lli': var.len!\n",elf_varlen(var));
	int arr[1] = {1};
	if (arr[0] != 1) printf("FAILED: '%i': array!\n",arr[0]);
	#endif

	#if 1
	elf_inimem();

	elf_Module M = {0};
	elf_Runtime R = {{&M}};
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

	#if defined(PLATFORM_WEB)
	// elf_String *contents = elf_pushnewstr(&R,"pf(\"Hello, World! - elf\")");
	// elf_loadexpr(&R,contents,0,0);

	elf_String *filename = elf_pushnewstr(&R,"/code/tests/fib.test.lang");
	filename->obj.gccolor = GC_PINK;

	elf_FileState fs = {0};
	elf_loadfile(&R,&fs,filename,0,0);
	#else
	/* todo: mark this as trap to figure out why
	gc is freeing this up! */
	elf_String *filename = elf_pushnewstr(&R,c[1]);
	filename->obj.gccolor = GC_PINK;
	elf_FileState fs = {0};
	elf_loadfile(&R,&fs,filename,0,0);
	#endif

	#if defined(_MSC_VER)
	FILE *file;
	fopen_s(&file,S_tpf("%s.module.ignore",filename->c),"wb");
	if (file != lnil) lang_dumpmodule(&M,file);
	fclose(file);
	#endif

	printf("exited\n");

	#endif
	return 0;
}


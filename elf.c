/*
** See Copyright Notice In elf.h
** elf.c
** Compiles and runs .elf files
*/


#define ELF_KEEPWARNINGS
#include "elf.h"


int main(int n, char **c) {
	(void) n;

	#if defined(_DEBUG)
	sys_consolelog(ELF_LOGDBUG,"COMPILER CHECK:");
	int *var = {0};
	elf_varadd(var,1);
	if (var[0] != 1) sys_consolelog(ELF_LOGERROR,"FAILED: var.add!\n");
	if (elf_varlen(var) != 1) sys_consolelog(ELF_LOGERROR,"FAILED: 'var.len!\n");
	int arr[1] = {1};
	if (arr[0] != 1) sys_consolelog(ELF_LOGERROR,"FAILED: arr!\n");
	#endif

	if (n == 1) {
		printf("\n\tUSAGE: elf.exe [options] file...\n");
		return 0;
	}

	elf_inimem();
	elf_Module M = {0};
	elf_Runtime R = {{&M}};
	R.logging = lfalse;
	R.stklen = 4096;
	R.stk = R.top = elf_clearalloc(lHEAP,sizeof(elf_val)*R.stklen);
	R.metatable_str = elf_newstrmetatab(&R);
	R.metatable_tab = elf_newtabmetatab(&R);
	elf_CallFrame root = {0};
	root.base = R.top;
	R.frame = &root;
	M.g = elf_locnewtab(&R);
	/* todo: eventually we'll load these from the
	source code, each lib will be built independently,
	or maybe we can add a flag so that we don't load
	these libraries. */
	netlib_load(&R);
	elflib_load(&R);
	tstlib_load(&R);
	crtlib_load(&R);

	elf_CallFrame frame = {0};
	frame.base = R.top;
	R.frame = &frame;

	/* todo: mark this as trap to figure out why
	gc is freeing this up! */
	elf_String *filename = elf_locnewstr(&R,c[1]);
	filename->obj.gccolor = GC_PINK;
	elf_FileState fs = {0};
	elf_loadfile(&R,&fs,filename,0,0);

#if !defined(PLATFORM_WEB)
	FILE *file;
	fopen_s(&file,elf_tpf("%s.module.ignore",filename->c),"wb");
	if (file != lnil) lang_dumpmodule(&M,file);
	fclose(file);
#endif

	sys_consolelog(ELF_LOGINFO,"exited");

	return 0;
}



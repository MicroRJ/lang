/*
** See Copyright Notice In elf.h
** elf-web.c
** Web API, For The Web.
*/


#define ELF_KEEPWARNINGS
#include "elf.h"


#if !defined(ELF_NOMAIN)
int main(int n, char **c) {
	sys_consolelog(ELF_LOGDBUG,"WEB!");

	#if defined(_DEBUG)
	sys_consolelog(ELF_LOGDBUG,"COMPILER CHECK:");
	int *var = {0};
	elf_varadd(var,1);
	if (var[0] != 1) sys_consolelog(ELF_LOGERROR,"FAILED: var.add!\n");
	if (elf_varlen(var) != 1) sys_consolelog(ELF_LOGERROR,"FAILED: 'var.len!\n");
	int arr[1] = {1};
	if (arr[0] != 1) sys_consolelog(ELF_LOGERROR,"FAILED: arr!\n");
	#endif
}
#endif


struct {
	elf_Runtime R;
	elf_Module M;
	elf_CallFrame C;
} elf_globaldecl elf = {{&elf.M}};


elf_api void elfweb_ini() {
	elf_inimem();
	elf.R.logging = lfalse;
	elf.R.stklen = 4096;
	elf.R.stk = elf.R.top = elf_clearalloc(lHEAP,sizeof(elf_val)*elf.R.stklen);

	elf.C.base = elf.R.top;
	elf.R.call = &elf.C;

	elf.R.metatable_str = elf_newstrmetatab(&elf.R);
	elf.R.metatable_tab = elf_newtabmetatab(&elf.R);
	elf.M.globals = elf_newloctab(&elf.R);
	netlib_load(&elf.R);
	elflib_load(&elf.R);
	tstlib_load(&elf.R);
	crtlib_load(&elf.R);
}


/* todo: copy the contents */
elf_api int elfweb_loadcode(char *codename, char *contents) {
	/* run in a separate call frame */
	elf_CallFrame call = {0};
	call.caller = elf.R.call;
	call.base = elf.R.top;
	call.top = elf.R.top;
	elf.R.call = &call;
	elf_String *name = elf_newlocstr(&elf.R,codename);
	elf_FileState fs = {0};
	int result = elf_loadcode(&elf.R,&fs,name,0,0,contents);
	elf.R.call = call.caller;
	return result;
}

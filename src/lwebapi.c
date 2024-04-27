/*
** See Copyright Notice In elf.h
** elf.c
** Web API, For The Web.
*/


elf_globaldecl elf_Module web_M = {0};
elf_globaldecl elf_Runtime web_R = {{&web_M}};
elf_globaldecl elf_CallFrame root_call = {0};


elf_api void elfweb_ini() {
	elf_inimem();
	web_R.logging = lfalse;
	web_R.stklen = 4096;
	web_R.stk = web_R.top = elf_clearalloc(lHEAP,sizeof(elf_val)*web_R.stklen);

	root_call.base = web_R.top;
	web_R.frame = &root_call;

	web_R.metatable_str = elf_newstrmetatab(&web_R);
	web_R.metatable_tab = elf_newtabmetatab(&web_R);
	web_M.globals = elf_putnewtab(&web_R);
	netlib_load(&web_R);
	syslib_load(&web_R);
	tstlib_load(&web_R);
	crtlib_load(&web_R);
}


/* todo: copy the contents */
elf_api int elfweb_loadcode(char *codename, char *contents) {


	/* run in a separate call frame */
	elf_CallFrame call = {0};
	call.caller = web_R.call;
	call.base = web_R.top;
	web_R.call = &call;


	/* todo: mark this as trap to figure out why
	gc is freeing this up! */
	elf_String *name = elf_putnewstr(&web_R,codename);
	name->obj.gccolor = GC_PINK;

	elf_FileState fs = {0};
	int result = elf_loadcode(&web_R,&fs,name,0,0,contents);

	// elf_String *str = elf_putnewstr(&web_R,contents);
	// int result = elf_loadexpr(&web_R,str,0,0);
	web_R.call = call.caller;
	return result;
}

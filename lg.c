/*
** See Copyright Notice In lang.h
** lg.c
** Compiles and runs .lang files
*/

#include <lang.h>


int main(int n, char **c) {
	(void) n;

	printf(".");

	langM_initmemory();

	jittest();

	lModule md = {0};

	lRuntime rt = {&md};
	rt.logging = ltrue;
	rt.stklen = 4096;
	rt.stk = rt.top = langM_clearalloc(lHEAP,sizeof(lValue) * rt.stklen);
	lContext root = {0};
	root.base = rt.top;
	rt.frame = &root;

	md.g = lang_pushnewtable(&rt);

	/* todo: eventually we'll call this from the source file,
	each lib will be built independently, or maybe we can add
	a flag so that we don't load these libraries. */
	syslib_load(&rt);
	tstlib_load(&rt);
	crtlib_load(&rt);
	jitlib_load(&rt);

	char pwd[0x100];
	sys_workdir(sizeof(pwd),pwd);
	printf("working in: %s\n",pwd);

	lString *filename = lang_pushnewS(&rt,c[1]);

	FileState fs = {0};
	lang_loadfile(&rt,&fs,filename,0);
	md.file = &fs;

	Handle file = sys_fopen(".module.ignore","wb");
	lang_dumpmodule(&md,file);


	printf("exited\n");
	return 0;
}


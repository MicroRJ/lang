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
	rt.z = 4096;
	rt.s = langM_clearalloc(lHEAP,sizeof(lValue) * rt.z);
	rt.v = rt.s;

	md.g = lang_pushnewtable(&rt);

	/* todo: eventually we'll call this
	from the source file, each lib will
	be built independently, or maybe we
	can add a flag so that we don't load
	these libraries. */
	syslib_load(&rt);
	tstlib_load(&rt);
	crtlib_load(&rt);
	jitlib_load(&rt);

	char pwd[0x100];
	sys_workdir(sizeof(pwd),pwd);
	printf("working in: %s\n",pwd);

	lString *filename = lang_pushnewS(&rt,c[1]);
	lang_loadfile(&rt,filename,0);

	Handle file = sys_fopen(".module.ignore","wb");
	lang_dumpmodule(&md,file);


	printf("exited\n");
	return 0;
}


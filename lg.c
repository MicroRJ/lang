/*
** See Copyright Notice In lang.h
** lg.c
** Compiles and runs .lang files
*/


#include <lang.h>


int main(int n, char **c) {
	(void) n;

	langM_initmemory();

	// char pwd[0x100];
	// sys_pwd(sizeof(pwd),pwd);
	// printf("working in: %s\n",pwd);

	lModule md = {0};

	lRuntime rt = {&md};
	rt.logging = lfalse;
	rt.stklen = 4096;
	rt.stk = rt.top = langM_clearalloc(lHEAP,sizeof(lValue) * rt.stklen);
	rt.classofS = langS_newclass(&rt);
	rt.classofH = langH_newclass(&rt);
	lCallFrame root = {0};
	root.base = rt.top;
	rt.frame = &root;

	md.g = lang_pushnewtable(&rt);

	/* todo: eventually we'll call this from the source file,
	each lib will be built independently, or maybe we can add
	a flag so that we don't load these libraries. */
	netlib_load(&rt);
	syslib_load(&rt);
	tstlib_load(&rt);
	crtlib_load(&rt);

	lCallFrame frame = {0};
	frame.base = rt.top;
	rt.frame = &frame;
	lString *filename = lang_pushnewS(&rt,c[1]);
	FileState fs = {0};
	lang_loadfile(&rt,&fs,filename,0,0);

	FILE *file;
	fopen_s(&file,S_tpf("%s.module.ignore",filename->c),"wb");
	if (file != lnil) lang_dumpmodule(&md,file);
	fclose(file);

	printf("exited\n");
	return 0;
}


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

	Module md = {0};

	Runtime rt = {&md};
	rt.z = 4096;
	rt.s = langM_clearalloc(lHEAP,sizeof(Value) * rt.z);
	rt.v = rt.s;

	md.g = langH_new(&rt);
	langGC_markpink((Object*)md.g);

	syslib_load(&rt);
	tstlib_load(&rt);

	char pwd[0x100];
	sys_pwd(sizeof(pwd),pwd);
	printf("working in: %s\n",pwd);

	String *filename = langR_pushnewS(&rt,c[1]);
	langR_loadfile(&rt,filename);

	printf("exited\n");
	return 0;
}


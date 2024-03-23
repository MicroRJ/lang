/*
** See Copyright Notice In lang.h
** lg.c
** Compiles and runs .lang files
*/

#include <lang.h>


int main(int n, char **c) {
	(void) n;

	langM_initmemory();

	Module md = {0};

	Runtime rt = {&md};
	rt.z = 4096;
	rt.s = langM_clearalloc(elHEAP,sizeof(Value) * rt.z);
	rt.v = rt.s;

	md.g = langH_new(&rt);
	langGC_markpink((Object*)md.g);

	syslib_open(&rt);
	testlib_open(&rt);

	langR_loadfile(&rt,langS_new(0,c[1]));

	printf("exited\n");
	return 0;
}


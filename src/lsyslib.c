/*
** See Copyright Notice In lang.h
** lsyslib.c
** System Library
*/


int syslib_fopen(Runtime *c) {
	String *name = langR_loadS(c,0);
	String *flags = langR_loadS(c,1);
	Handle h = sys_fopen(name->string,flags->string);
	langR_pushI(c,(Integer)h);
	return 1;
}


int syslib_fsize(Runtime *c) {
	FILE *file = (FILE *) langR_loadI(c,0);
	fseek(file,0,SEEK_END);
	langR_pushI(c,ftell(file));
	return 1;
}


void syslib_pfv_(FILE *file, Value v, Bool quotes) {
	switch (v.tag) {
		case VALUE_NONE: {
			fprintf(file,"nil");
		} break;
		case VALUE_INTEGER: {
			fprintf(file,"%lli",v.i);
		} break;
		case VALUE_NUMBER: {
			fprintf(file,"%f",v.n);
		} break;
		case VALUE_TABLE: {
			Table *t = v.t;
			fprintf(file,"{");
			langA_varifor (t->v) {
				if (i != 0) fprintf(file,", ");
				syslib_pfv_(file,t->v[i],True);
			}
			fprintf(file,"}");
		} break;
		case VALUE_FUNC: {
			fprintf(file,"F()");
		} break;
		case VALUE_CFUN: {
			fprintf(file,"C()");
		} break;
		case VALUE_STRING: {
			if (quotes) {
				fprintf(file,"\"%s\"",v.s->string);
			} else {
				fprintf(file,"%s",v.s->string);
			}
		} break;
		default: {
			fprintf(file,"(unknown)");
		} break;
	}
}


int syslib_fpf(Runtime *rt) {
	FILE *file = (FILE *) langR_loadI(rt,0);
	for (int i = 1; i < rt->f->n; i ++) {
		syslib_pfv_(file,langR_loadV(rt,i),False);
	}
	return 0;
}


int syslib_pf_(Runtime *rt) {
	for (int i = 0; i < rt->f->n; i ++) {
		syslib_pfv_(stdout,langR_loadV(rt,i),False);
	}
	fprintf(stdout,"\n");
	return 0;
}


LAPI int syslib_sleep(Runtime *rt) {
	LASSERT(rt->f->n == 1);
	sys_sleep(langR_loadI(rt,0));
	return 0;
}


LAPI int syslib_clocktime(Runtime *rt) {
	langR_pushI(rt,sys_clocktime());
	return 1;
}


LAPI int syslib_timediffs(Runtime *rt) {
	LASSERT(rt->f->n == 1);
	Integer i = langR_loadI(rt,0);
	langR_pushN(rt,(sys_clocktime() - i) / (Number) sys_clockhz());
	return 1;
}


Bool isvirtual(char const *fn) {
	while (*fn == '.') ++ fn;
	return *fn == 0;
}


static String *keyname;
static String *keypath;
static String *keyisdir;
void syslib_listdir_(Runtime *c, Table *list, Table *flags, String *d, Closure *cl) {
	Module *md = c->md;

	WIN32_FIND_DATAA f;
	HANDLE h = FindFirstFileA(S_tpf("%s\\*",d->string),&f);

	if (h != INVALID_HANDLE_VALUE) do {
		if (isvirtual(f.cFileName)) continue;

		int isdir = 0 != (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		String *name = langS_new(c,f.cFileName);
		String *path = langS_new(c,S_tpf("%s\\%s",d->string,f.cFileName));

		Table *file = langR_pushnewR(c);
		langH_insert(file,lang_S(keyname),lang_S(name));
		langH_insert(file,lang_S(keypath),lang_S(path));
		langH_insert(file,lang_S(keyisdir),lang_I(isdir));
		langR_pushH(c,flags);

		int r = langR_callfunc(c,cl,2);
		LASSERT(r == 1);
		int addtolist = langR_popI(c);
		if (addtolist) {
			langH_insert(list,lang_S(path),lang_T(file));
			syslib_listdir_(c,list,flags,path,cl);
		}
	} while (FindNextFileA(h,&f));
}


LAPI int syslib_listdir(Runtime *c) {
	LASSERT(c->f->n == 2);

	String *d = langR_loadS(c,0);
	Closure *f = langR_loadF(c,1);

	/* push these keys temporarily so they won't
	be gc'd but also to to avoid creating them
	 so often  */
	keyname = langR_pushnewS(c,"name");
	keypath = langR_pushnewS(c,"path");
	keyisdir = langR_pushnewS(c,"isdir");

	/* push flags temporarily so that it won't be gc'd,
	in between calls to listdir garbage collection
	may trigger, in which case it could get deallocated. */
	Table *flags = langR_pushnewR(c);

	/* push list last to serve as return value */
	Table *list = langR_pushnewR(c);
	syslib_listdir_(c,list,flags,d,f);

	return 1;
}


LAPI void syslib_load(Runtime *rt) {
	Module *md = rt->md;
	lang_addglobal(md,langR_pushnewS(rt,"fopen"),lang_C(syslib_fopen));
	lang_addglobal(md,langR_pushnewS(rt,"fsize"),lang_C(syslib_fsize));
	lang_addglobal(md,langR_pushnewS(rt,"fpf"),lang_C(syslib_fpf));

	lang_addglobal(md,langR_pushnewS(rt,"pf"),lang_C(syslib_pf_));

	lang_addglobal(md,langR_pushnewS(rt,"clocktime"),lang_C(syslib_clocktime));
	lang_addglobal(md,langR_pushnewS(rt,"timediffs"),lang_C(syslib_timediffs));
	lang_addglobal(md,langR_pushnewS(rt,"sleep"),lang_C(syslib_sleep));
	lang_addglobal(md,langR_pushnewS(rt,"listdir"),lang_C(syslib_listdir));
}

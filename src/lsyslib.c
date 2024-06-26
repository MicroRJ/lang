/*
** See Copyright Notice In lang.h
** lsyslib.c
** System Library
*/


int syslib_libfn(Runtime *rt) {
	Handle lib = lang_loadhandle(rt,0);
	lString *name = lang_loadS(rt,1);
	lBinding fn = (lBinding) sys_libfn(lib,name->c);
	if (fn != lnil) {
		lang_pushbinding(rt,fn);
	} else {
		lang_pushnil(rt);
	}
	return 1;
}


int syslib_loadlib(Runtime *rt) {
	lString *name = lang_loadS(rt,0);
	Handle lib = sys_loadlib(name->c);
	if (lib != INVALID_HANDLE_VALUE) {
		lang_pushhandle(rt,lib);
	} else {
		lang_pushnil(rt);
	}
	return 1;
}


int syslib_workdir(Runtime *rt) {
	char buf[MAX_PATH];
	sys_workdir(sizeof(buf),buf);
	lang_pushnewS(rt,buf);
	if (rt->f->x == 1) {
		lString *s = lang_loadS(rt,0);
		sys_setworkdir(s->c);
	}
	return 1;
}


int syslib_pwd(Runtime *rt) {
	char buf[MAX_PATH];
	sys_workdir(sizeof(buf),buf);
	lang_pushnewS(rt,buf);
	return 1;
}


int syslib_setpwd(Runtime *rt) {
	lString *s = lang_loadS(rt,0);
	sys_setworkdir(s->c);
	return 0;
}


int syslib_fopen(Runtime *c) {
	lString *name = lang_loadS(c,0);
	lString *flags = lang_loadS(c,1);
	Handle h = sys_fopen(name->c,flags->c);
	lang_pushhandle(c,h);
	return 1;
}


int syslib_fclose(Runtime *c) {
	Handle file = lang_loadhandle(c,0);
	sys_fclose(file);
	return 0;
}


int syslib_fsize(Runtime *c) {
	Handle file = lang_loadhandle(c,0);
	fseek(file,0,SEEK_END);
	lang_pushlong(c,ftell(file));
	return 1;
}


int syslib_fpfv_(FILE *file, lValue v, lbool quotes) {
	switch (v.tag) {
		case VALUE_NONE: return fprintf(file,"nil");
		case VALUE_HANDLE: return fprintf(file,"h%llX",v.i);
		case VALUE_LONG: return fprintf(file,"%lli",v.i);
		case VALUE_REAL: return fprintf(file,"%f",v.n);
		case VALUE_FUNC: return fprintf(file,"F()");
		case VALUE_BINDING: return fprintf(file,"C()");
		case VALUE_TABLE: {
			int wrote = 0;
			Table *t = v.t;
			wrote += fprintf(file,"{");
			langA_varifor(t->v) {
				if (i != 0) wrote += fprintf(file,", ");
				wrote += syslib_fpfv_(file,t->v[i],ltrue);
			}
			wrote += fprintf(file,"}");
			return wrote;
		} break;
		case VALUE_STRING: {
			if (quotes) {
				return fprintf(file,"\"%s\"",v.s->string);
			} else {
				return fprintf(file,"%s",v.s->string);
			}
		} break;
		default: return fprintf(file,"(?)");
	}
}


int syslib_fpf(Runtime *rt) {
	Handle file = lang_loadhandle(rt,0);
	int wrote = 0;
	for (int i = 1; i < rt->f->x; i ++) {
		wrote += syslib_fpfv_(file,lang_load(rt,i),lfalse);
	}
	lang_pushlong(rt,wrote);
	return 1;
}


int syslib_pf_(Runtime *rt) {
	for (int i = 0; i < rt->f->x; i ++) {
		syslib_fpfv_(stdout,lang_load(rt,i),lfalse);
	}
	fprintf(stdout,"\n");
	return 0;
}


lapi int syslib_sleep(Runtime *rt) {
	LASSERT(rt->f->x == 1);
	sys_sleep(lang_loadlong(rt,0));
	return 0;
}


lapi int syslib_clocktime(Runtime *rt) {
	lang_pushlong(rt,sys_clocktime());
	return 1;
}


lapi int syslib_timediffs(Runtime *rt) {
	LASSERT(rt->f->x == 1);
	llong i = lang_loadlong(rt,0);
	lang_pushnum(rt,(sys_clocktime() - i) / (lnumber) sys_clockhz());
	return 1;
}


lbool isvirtual(char const *fn) {
	while (*fn == '.') ++ fn;
	return *fn == 0;
}


static lString *keyname;
static lString *keypath;
static lString *keyisdir;
void syslib_listdir_(Runtime *c, Table *list, Table *flags, lString *d, lClosure *cl) {
	lModule *md = c->md;

	WIN32_FIND_DATAA f;
	HANDLE h = FindFirstFileA(S_tpf("%s\\*",d->string),&f);

	if (h != INVALID_HANDLE_VALUE) do {
		if (isvirtual(f.cFileName)) continue;

		int isdir = 0 != (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		lString *name = langS_new(c,f.cFileName);
		lString *path = langS_new(c,S_tpf("%s\\%s",d->string,f.cFileName));

		lang_pushlong(c,ltrue);
		Table *file = lang_pushnewtable(c);
		langH_insert(file,lang_S(keyname),lang_S(name));
		langH_insert(file,lang_S(keypath),lang_S(path));
		langH_insert(file,lang_S(keyisdir),lang_I(isdir));
		lang_pushtable(c,flags);

		int r = lang_call(c,0,cl,2,1);
		LASSERT(r == 1);
		int addit = lang_poplong(c);
		if (addit) {
			langH_insert(list,lang_S(path),lang_T(file));
			if (isdir) {
				syslib_listdir_(c,list,flags,path,cl);
			}
		}
	} while (FindNextFileA(h,&f));
}


lapi int syslib_listdir(Runtime *c) {
	LASSERT(c->f->x == 2);

	lString *d = lang_loadS(c,0);
	lClosure *f = lang_loadcl(c,1);

	/* push these keys temporarily so they won't
	be gc'd and also to to avoid creating them so often  */
	keyname = lang_pushnewS(c,"name");
	keypath = lang_pushnewS(c,"path");
	keyisdir = lang_pushnewS(c,"isdir");

	/* push flags temporarily so that it won't be gc'd
	in between calls to listdir. */
	Table *flags = lang_pushnewtable(c);

	/* push list last to serve as return value */
	Table *list = lang_pushnewtable(c);
	syslib_listdir_(c,list,flags,d,f);

	return 1;
}


/* todo: can we do this from code */
lapi void syslib_load(Runtime *rt) {
	lModule *md = rt->md;

	lang_addglobal(md,lang_pushnewS(rt,"loadlib"),lang_C(syslib_loadlib));
	lang_addglobal(md,lang_pushnewS(rt,"libfn"),lang_C(syslib_libfn));

	lang_addglobal(md,lang_pushnewS(rt,"workdir"),lang_C(syslib_workdir));
	lang_addglobal(md,lang_pushnewS(rt,"pwd"),lang_C(syslib_pwd));
	lang_addglobal(md,lang_pushnewS(rt,"setpwd"),lang_C(syslib_setpwd));

	lang_addglobal(md,lang_pushnewS(rt,"fopen"),lang_C(syslib_fopen));
	lang_addglobal(md,lang_pushnewS(rt,"fclose"),lang_C(syslib_fclose));
	lang_addglobal(md,lang_pushnewS(rt,"fsize"),lang_C(syslib_fsize));
	lang_addglobal(md,lang_pushnewS(rt,"fpf"),lang_C(syslib_fpf));

	lang_addglobal(md,lang_pushnewS(rt,"pf"),lang_C(syslib_pf_));

	lang_addglobal(md,lang_pushnewS(rt,"clocktime"),lang_C(syslib_clocktime));
	lang_addglobal(md,lang_pushnewS(rt,"timediffs"),lang_C(syslib_timediffs));
	lang_addglobal(md,lang_pushnewS(rt,"sleep"),lang_C(syslib_sleep));
	lang_addglobal(md,lang_pushnewS(rt,"listdir"),lang_C(syslib_listdir));
}

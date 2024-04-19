/*
** See Copyright Notice In lang.h
** lsyslib.c
** System Library
*/


#if 0
lTable *createthread(lRuntime *R) {
}
DWORD WINAPI syslib_threadproxyproc_win32(lRuntime *R) {
	return 1;
}
int syslib_thread(lRuntime *R) {
	lClosure *fn = lang_loadcl(R,0);
	HANDLE thread = CreateThread(NULL,0,syslib_threadproxyproc_win32,R,CREATE_SUSPENDED,NULL);
	return 1;
}
#endif

int syslib_loadexpr(lRuntime *R) {
	lString *contents = lang_getstr(R,0);
	lang_loadexpr(R,contents,-1,R->call->y);
	/* no need to do hoisting */
	return 0;
}


int syslib_libfn(lRuntime *rt) {
	lsysobj lib = lang_getsysobj(rt,0);
	lString *name = lang_getstr(rt,1);
	lBinding fn = (lBinding) sys_libfn(lib,name->c);
	if (fn != lnil) {
		lang_pushbinding(rt,fn);
	} else {
		lang_pushnil(rt);
	}
	return 1;
}


int syslib_loadlib(lRuntime *rt) {
	lString *name = lang_getstr(rt,0);
	lsysobj lib = sys_loadlib(name->c);
	if (lib != INVALID_HANDLE_VALUE) {
		lang_pushsysobj(rt,lib);
	} else {
		lang_pushnil(rt);
	}
	return 1;
}


int syslib_exec(lRuntime *R) {
	lString *cmd = lang_getstr(R,0);
	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi = {0};
	int result = CreateProcess(NULL,cmd->c,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	lang_pushlong(R,result);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 1;
}


int syslib_freadall(lRuntime *R) {
	FILE *file = (FILE*) lang_getsysobj(R,0);
	if (file != 0) {
		fseek(file,0,SEEK_END);
		long size = ftell(file);
		fseek(file,0,SEEK_SET);
		lString *buf = langS_new2(R,size);
		fread(buf->c,1,size,file);
		lang_pushString(R,buf);
	} else lang_pushnil(R);
	return 1;
}


int syslib_ftemp(lRuntime *R) {
	FILE *file = {0};
	tmpfile_s(&file);
	lang_pushsysobj(R,(lsysobj)file);
	return 1;
}


int syslib_workdir(lRuntime *rt) {
	char buf[MAX_PATH];
	sys_workdir(sizeof(buf),buf);
	lang_pushnewS(rt,buf);
	if (rt->f->x == 1) {
		lString *s = lang_getstr(rt,0);
		sys_setworkdir(s->c);
	}
	return 1;
}


int syslib_pwd(lRuntime *rt) {
	char buf[MAX_PATH];
	sys_workdir(sizeof(buf),buf);
	lang_pushnewS(rt,buf);
	return 1;
}


int syslib_setpwd(lRuntime *rt) {
	lString *s = lang_getstr(rt,0);
	sys_setworkdir(s->c);
	return 0;
}


int syslib_fpfv_(FILE *file, lValue v, lbool quotes) {
	switch (v.tag) {
		case TAG_NIL: return fprintf(file,"nil");
		case TAG_SYS: return fprintf(file,"h%llX",v.i);
		case TAG_INT: return fprintf(file,"%lli",v.i);
		case TAG_NUM: return fprintf(file,"%f",v.n);
		case TAG_CLS: return fprintf(file,"F()");
		case TAG_BID: return fprintf(file,"C()");
		case TAG_TAB: {
			int wrote = 0;
			lTable *t = v.t;
			wrote += fprintf(file,"{");
			langA_varifor(t->v) {
				if (i != 0) wrote += fprintf(file,", ");
				wrote += syslib_fpfv_(file,t->v[i],ltrue);
			}
			wrote += fprintf(file,"}");
			return wrote;
		} break;
		case TAG_STR: {
			if (quotes) {
				return fprintf(file,"\"%s\"",v.s->string);
			} else {
				return fprintf(file,"%s",v.s->string);
			}
		} break;
		default: return fprintf(file,"(?)");
	}
}


int syslib_fpf(lRuntime *rt) {
	lsysobj file = lang_getsysobj(rt,0);
	int wrote = 0;
	for (int i = 1; i < rt->f->x; i ++) {
		wrote += syslib_fpfv_(file,lang_load(rt,i),lfalse);
	}
	lang_pushlong(rt,wrote);
	return 1;
}


int syslib_lpf(lRuntime *rt) {
	for (int i = 0; i < rt->f->x; i ++) {
		if (i != 0) fprintf(stdout,"\n");
		syslib_fpfv_(stdout,lang_load(rt,i),lfalse);
	}
	fprintf(stdout,"\n");
	return 0;
}


int syslib_pf(lRuntime *rt) {
	for (int i = 0; i < rt->f->x; i ++) {
		syslib_fpfv_(stdout,lang_load(rt,i),lfalse);
	}
	fprintf(stdout,"\n");
	return 0;
}


lapi int syslib_sleep(lRuntime *rt) {
	LASSERT(rt->f->x == 1);
	sys_sleep(lang_loadlong(rt,0));
	return 0;
}


lapi int syslib_clocktime(lRuntime *rt) {
	lang_pushlong(rt,sys_clocktime());
	return 1;
}


lapi int syslib_timediffs(lRuntime *rt) {
	LASSERT(rt->f->x == 1);
	llongint i = lang_loadlong(rt,0);
	lang_pushnum(rt,(sys_clocktime() - i) / (lnumber) sys_clockhz());
	return 1;
}


lbool isvirtual(char const *fn) {
	while (*fn == '.') ++ fn;
	return *fn == 0;
}


lglobaldecl lString *keyname;
lglobaldecl lString *keypath;
lglobaldecl lString *keyisdir;
void syslib_listdir_(lRuntime *R, lString *d, lTable *list, lTable *flags, lClosure *cl) {
	lModule *md = R->md;

	char *dir = S_tpf("%s\\*",d->c);
	WIN32_FIND_DATAA f;
	HANDLE h = FindFirstFileA(dir,&f);

	if (h != INVALID_HANDLE_VALUE) do {
		if (isvirtual(f.cFileName)) continue;

		int isdir = 0 != (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		lString *name = langS_new(R,f.cFileName);
		lString *path = langS_new(R,S_tpf("%s\\%s",d->string,f.cFileName));

		llocalid base = lang_pushclosure(R,cl);
		lTable *file = lang_pushnewtable(R);
		lang_pushtable(R,flags);
		langH_insert(file,lang_S(keyname),lang_S(name));
		langH_insert(file,lang_S(keypath),lang_S(path));
		langH_insert(file,lang_S(keyisdir),lang_I(isdir));

		int r = lang_rootcall(R,base,2,1);
		if ((r > 0) && lang_loadlong(R,base)) {
			langH_insert(list,lang_S(path),lang_T(file));
			if (isdir) {
				syslib_listdir_(R,path,list,flags,cl);
			}
		}
	} while (FindNextFileA(h,&f));
}


lapi int syslib_listdir(lRuntime *R) {
	LASSERT(R->frame->x == 2);
	/* push these keys temporarily so they won't
	be gc'd and also to to avoid creating them so often  */
	keyname = lang_pushnewS(R,"name");
	keypath = lang_pushnewS(R,"path");
	keyisdir = lang_pushnewS(R,"isdir");
	lClosure *cl = lang_loadcl(R,1);
	lString *dir = lang_getstr(R,0);
	lTable *flags = lang_pushnewtable(R);
	/* push list last to serve as return value */
	lTable *list = lang_pushnewtable(R);
	syslib_listdir_(R,dir,list,flags,cl);
	return 1;
}


/* todo: can we do this from code */
lapi void syslib_load(lRuntime *rt) {
	lModule *md = rt->md;

	lang_addglobal(md,lang_pushnewS(rt,"loadlib"),lang_C(syslib_loadlib));
	lang_addglobal(md,lang_pushnewS(rt,"libfn"),lang_C(syslib_libfn));
	lang_addglobal(md,lang_pushnewS(rt,"loadexpr"),lang_C(syslib_loadexpr));

	lang_addglobal(md,lang_pushnewS(rt,"workdir"),lang_C(syslib_workdir));
	lang_addglobal(md,lang_pushnewS(rt,"pwd"),lang_C(syslib_pwd));
	lang_addglobal(md,lang_pushnewS(rt,"setpwd"),lang_C(syslib_setpwd));

	lang_addglobal(md,lang_pushnewS(rt,"freadall"),lang_C(syslib_freadall));
	lang_addglobal(md,lang_pushnewS(rt,"ftemp"),lang_C(syslib_ftemp));

	lang_addglobal(md,lang_pushnewS(rt,"fpf"),lang_C(syslib_fpf));
	lang_addglobal(md,lang_pushnewS(rt,"pf"),lang_C(syslib_pf));
	lang_addglobal(md,lang_pushnewS(rt,"lpf"),lang_C(syslib_lpf));

	lang_addglobal(md,lang_pushnewS(rt,"clocktime"),lang_C(syslib_clocktime));
	lang_addglobal(md,lang_pushnewS(rt,"timediffs"),lang_C(syslib_timediffs));
	lang_addglobal(md,lang_pushnewS(rt,"listdir"),lang_C(syslib_listdir));
	lang_addglobal(md,lang_pushnewS(rt,"sleep"),lang_C(syslib_sleep));
	lang_addglobal(md,lang_pushnewS(rt,"exec"),lang_C(syslib_exec));
}

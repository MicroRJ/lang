/*
** See Copyright Notice In lang.h
** lsyslib.c
** System Library
*/


#if 0
elf_tab *createthread(lRuntime *R) {
}
DWORD WINAPI syslib_threadproxyproc_win32(lRuntime *R) {
	return 1;
}
int syslib_thread(lRuntime *R) {
	elf_Closure *fn = elf_getcls(R,0);
	HANDLE thread = CreateThread(NULL,0,syslib_threadproxyproc_win32,R,CREATE_SUSPENDED,NULL);
	return 1;
}
#endif


int syslib_iton(lRuntime *R) {
	elf_val v = elf_getval(R,0);
	if (v.tag == TAG_INT) {
		elf_putnum(R,(elf_num)v.i);
	} else elf_putnum(R,v.n);
	return 1;
}


int syslib_ntoi(lRuntime *R) {
	elf_val v = elf_getval(R,0);
	if (v.tag == TAG_NUM) {
		elf_putint(R,(elf_int)v.n);
	} else elf_putint(R,v.i);
	return 1;
}


int syslib_loadexpr(lRuntime *R) {
	elf_str *contents = elf_getstr(R,0);
	elf_loadexpr(R,contents,-1,R->call->y);
	/* no need to do hoisting */
	return 0;
}


int syslib_libfn(lRuntime *rt) {
	elf_Handle lib = elf_getsys(rt,0);
	elf_str *name = elf_getstr(rt,1);
	lBinding fn = (lBinding) sys_libfn(lib,name->c);
	if (fn != lnil) {
		elf_putbinding(rt,fn);
	} else {
		elf_putnil(rt);
	}
	return 1;
}


int syslib_loadlib(lRuntime *rt) {
	elf_str *name = elf_getstr(rt,0);
	elf_Handle lib = sys_loadlib(name->c);
	if (lib != INVALID_HANDLE_VALUE) {
		elf_putsys(rt,lib);
	} else {
		elf_putnil(rt);
	}
	return 1;
}


int syslib_exec(lRuntime *R) {
	elf_str *cmd = elf_getstr(R,0);
	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi = {0};
	int result = CreateProcess(NULL,cmd->c,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	elf_putint(R,result);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 1;
}


int syslib_freadall(lRuntime *R) {
	FILE *file = (FILE*) elf_getsys(R,0);
	if (file != 0) {
		fseek(file,0,SEEK_END);
		long size = ftell(file);
		fseek(file,0,SEEK_SET);
		elf_str *buf = elf_newstrlen(R,size);
		fread(buf->c,1,size,file);
		elf_putstr(R,buf);
	} else elf_putnil(R);
	return 1;
}


int syslib_ftemp(lRuntime *R) {
	FILE *file = {0};
	tmpfile_s(&file);
	elf_putsys(R,(elf_Handle)file);
	return 1;
}


int syslib_workdir(lRuntime *rt) {
	lang_logerror("this function is deprecated");

	char buf[MAX_PATH];
	sys_pwd(sizeof(buf),buf);
	elf_pushnewstr(rt,buf);
	if (rt->f->x == 1) {
		elf_str *s = elf_getstr(rt,0);
		sys_setpwd(s->c);
	}
	return 1;
}


int syslib_pwd(lRuntime *rt) {
	char buf[MAX_PATH];
	sys_pwd(sizeof(buf),buf);
	elf_pushnewstr(rt,buf);
	return 1;
}


int syslib_setpwd(lRuntime *rt) {
	elf_str *s = elf_getstr(rt,0);
	sys_setpwd(s->c);
	return 0;
}


int syslib_fpfv_(FILE *file, elf_val v, elf_bool quotes) {
	switch (v.tag) {
		case TAG_NIL: return fprintf(file,"nil");
		case TAG_SYS: return fprintf(file,"h%llX",v.i);
		case TAG_INT: return fprintf(file,"%lli",v.i);
		case TAG_NUM: return fprintf(file,"%f",v.n);
		case TAG_CLS: return fprintf(file,"F()");
		case TAG_BID: return fprintf(file,"C()");
		case TAG_TAB: {
			int wrote = 0;
			elf_tab *t = v.t;
			wrote += fprintf(file,"{");
			elf_forivar(t->v) {
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
	elf_Handle file = elf_getsys(rt,0);
	int wrote = 0;
	for (int i = 1; i < rt->f->x; i ++) {
		wrote += syslib_fpfv_(file,elf_getval(rt,i),lfalse);
	}
	elf_putint(rt,wrote);
	return 1;
}


int syslib_lpf(lRuntime *rt) {
	for (int i = 0; i < rt->f->x; i ++) {
		if (i != 0) fprintf(stdout,"\n");
		syslib_fpfv_(stdout,elf_getval(rt,i),lfalse);
	}
	fprintf(stdout,"\n");
	return 0;
}


int syslib_pf(lRuntime *rt) {
	for (int i = 0; i < rt->f->x; i ++) {
		syslib_fpfv_(stdout,elf_getval(rt,i),lfalse);
	}
	fprintf(stdout,"\n");
	return 0;
}


lapi int syslib_sleep(lRuntime *rt) {
	elf_assert(rt->f->x == 1);
	sys_sleep(elf_getint(rt,0));
	return 0;
}


lapi int syslib_clocktime(lRuntime *rt) {
	elf_putint(rt,sys_clocktime());
	return 1;
}


lapi int syslib_timediffs(lRuntime *rt) {
	elf_assert(rt->f->x == 1);
	elf_int i = elf_getint(rt,0);
	elf_putnum(rt,(sys_clocktime() - i) / (elf_num) sys_clockhz());
	return 1;
}


elf_bool isvirtual(char const *fn) {
	while (*fn == '.') ++ fn;
	return *fn == 0;
}


lglobaldecl elf_str *keyname;
lglobaldecl elf_str *keypath;
lglobaldecl elf_str *keyisdir;
void syslib_listdir_(lRuntime *R, elf_str *d, elf_tab *list, elf_tab *flags, elf_Closure *cl) {
	elf_Module *md = R->md;

	char *dir = S_tpf("%s\\*",d->c);
	WIN32_FIND_DATAA f;
	HANDLE h = FindFirstFileA(dir,&f);

	if (h != INVALID_HANDLE_VALUE) do {
		if (isvirtual(f.cFileName)) continue;

		int isdir = 0 != (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		elf_str *name = elf_newstr(R,f.cFileName);
		elf_str *path = elf_newstr(R,S_tpf("%s\\%s",d->string,f.cFileName));

		llocalid base = elf_putcl(R,cl);
		elf_tab *file = elf_pushnewtab(R);
		elf_puttab(R,flags);
		elf_tabput(file,lang_S(keyname),lang_S(name));
		elf_tabput(file,lang_S(keypath),lang_S(path));
		elf_tabput(file,lang_S(keyisdir),lang_I(isdir));

		int r = elf_rootcall(R,base,2,1);
		if ((r > 0) && elf_getint(R,base)) {
			elf_tabput(list,lang_S(path),lang_T(file));
			if (isdir) {
				syslib_listdir_(R,path,list,flags,cl);
			}
		}
	} while (FindNextFileA(h,&f));
}


lapi int syslib_listdir(lRuntime *R) {
	elf_assert(R->frame->x == 2);
	/* push these keys temporarily so they won't
	be gc'd and also to to avoid creating them so often  */
	keyname = elf_pushnewstr(R,"name");
	keypath = elf_pushnewstr(R,"path");
	keyisdir = elf_pushnewstr(R,"isdir");
	elf_Closure *cl = elf_getcls(R,1);
	elf_str *dir = elf_getstr(R,0);
	elf_tab *flags = elf_pushnewtab(R);
	/* push list last to serve as return value */
	elf_tab *list = elf_pushnewtab(R);
	syslib_listdir_(R,dir,list,flags,cl);
	return 1;
}


/* todo: can we do this from code */
lapi void syslib_load(lRuntime *rt) {
	elf_Module *md = rt->md;

	lang_addglobal(md,elf_pushnewstr(rt,"ntoi"),lang_C(syslib_ntoi));
	lang_addglobal(md,elf_pushnewstr(rt,"iton"),lang_C(syslib_iton));

	lang_addglobal(md,elf_pushnewstr(rt,"loadlib"),lang_C(syslib_loadlib));
	lang_addglobal(md,elf_pushnewstr(rt,"libfn"),lang_C(syslib_libfn));
	lang_addglobal(md,elf_pushnewstr(rt,"loadexpr"),lang_C(syslib_loadexpr));

	lang_addglobal(md,elf_pushnewstr(rt,"workdir"),lang_C(syslib_workdir));
	lang_addglobal(md,elf_pushnewstr(rt,"pwd"),lang_C(syslib_pwd));
	lang_addglobal(md,elf_pushnewstr(rt,"setpwd"),lang_C(syslib_setpwd));

	lang_addglobal(md,elf_pushnewstr(rt,"freadall"),lang_C(syslib_freadall));
	lang_addglobal(md,elf_pushnewstr(rt,"ftemp"),lang_C(syslib_ftemp));

	lang_addglobal(md,elf_pushnewstr(rt,"fpf"),lang_C(syslib_fpf));
	lang_addglobal(md,elf_pushnewstr(rt,"pf"),lang_C(syslib_pf));
	lang_addglobal(md,elf_pushnewstr(rt,"lpf"),lang_C(syslib_lpf));

	lang_addglobal(md,elf_pushnewstr(rt,"clocktime"),lang_C(syslib_clocktime));
	lang_addglobal(md,elf_pushnewstr(rt,"timediffs"),lang_C(syslib_timediffs));
	lang_addglobal(md,elf_pushnewstr(rt,"listdir"),lang_C(syslib_listdir));
	lang_addglobal(md,elf_pushnewstr(rt,"sleep"),lang_C(syslib_sleep));
	lang_addglobal(md,elf_pushnewstr(rt,"exec"),lang_C(syslib_exec));
}

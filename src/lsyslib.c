/*
** See Copyright Notice In elf.h
** lsyslib.c
** System Library
*/


#if 0
elf_Table *createthread(elf_Runtime *R) {
}
DWORD WINAPI syslib_threadproxyproc_win32(elf_Runtime *R) {
	return 1;
}
int syslib_thread(elf_Runtime *R) {
	elf_Closure *fn = elf_getcls(R,0);
	HANDLE thread = CreateThread(NULL,0,syslib_threadproxyproc_win32,R,CREATE_SUSPENDED,NULL);
	return 1;
}
#endif


int syslib_iton(elf_Runtime *R) {
	elf_val v = elf_getval(R,0);
	if (v.tag == TAG_INT) {
		elf_putnum(R,(elf_num)v.i);
	} else elf_putnum(R,v.n);
	return 1;
}


int syslib_ntoi(elf_Runtime *R) {
	elf_val v = elf_getval(R,0);
	if (v.tag == TAG_NUM) {
		elf_putint(R,(elf_int)v.n);
	} else elf_putint(R,v.i);
	return 1;
}


int syslib_loadexpr(elf_Runtime *R) {
	elf_String *contents = elf_getstr(R,0);
	elf_loadexpr(R,contents,R->call->ry,R->call->ny);
	/* no need to do hoisting */
	return 0;
}


int syslib_loadfile(elf_Runtime *R) {
	elf_String *filename = elf_getstr(R,0);
	elf_FileState fs = {0};
	elf_loadfile(R,&fs,filename,R->call->ry,R->call->ny);
	/* no need to do hoisting */
	return 0;
}


int syslib_libfn(elf_Runtime *rt) {
	elf_Handle lib = elf_getsys(rt,0);
	elf_String *name = elf_getstr(rt,1);
	lBinding fn = (lBinding) sys_libfn(lib,name->c);
	if (fn != lnil) {
		elf_putbinding(rt,fn);
	} else {
		elf_putnil(rt);
	}
	return 1;
}


int syslib_loadlib(elf_Runtime *rt) {
	elf_String *name = elf_getstr(rt,0);
	elf_Handle lib = sys_loadlib(name->c);
	if (lib != lnil) {
		elf_putsys(rt,lib);
	} else elf_putnil(rt);
	return 1;
}


int syslib_exec(elf_Runtime *R) {
#if defined(_WIN32)
	elf_String *cmd = elf_getstr(R,0);
	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi = {0};
	int result = CreateProcess(NULL,cmd->c,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	elf_putint(R,result);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#else
	elf_putint(R,0);
#endif
	return 1;
}


int syslib_freadall(elf_Runtime *R) {
	FILE *file = (FILE*) elf_getsys(R,0);
	if (file != 0) {
		fseek(file,0,SEEK_END);
		long size = ftell(file);
		fseek(file,0,SEEK_SET);
		elf_String *buf = elf_newstrlen(R,size);
		fread(buf->c,1,size,file);
		elf_putstr(R,buf);
	} else elf_putnil(R);
	return 1;
}


int syslib_ftemp(elf_Runtime *R) {
	FILE *file = {0};
#if defined(_MSC_VER)
	tmpfile_s(&file);
#else
	file = tmpfile();
#endif
	elf_putsys(R,(elf_Handle)file);
	return 1;
}


int syslib_workdir(elf_Runtime *rt) {
	elf_logerror("this function is deprecated");
	char buf[MAX_PATH];
	sys_pwd(sizeof(buf),buf);
	elf_pushnewstr(rt,buf);
	if (rt->f->x == 1) {
		elf_String *s = elf_getstr(rt,0);
		sys_setpwd(s->c);
	}
	return 1;
}


int syslib_pwd(elf_Runtime *rt) {
	char buf[MAX_PATH];
	sys_pwd(sizeof(buf),buf);
	elf_pushnewstr(rt,buf);
	return 1;
}


int syslib_setpwd(elf_Runtime *rt) {
	elf_String *s = elf_getstr(rt,0);
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
			elf_Table *t = v.t;
			wrote += fprintf(file,"{");
			elf_arrfori(t->v) {
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


int syslib_fpf(elf_Runtime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	int wrote = 0;
	for (int i = 1; i < rt->f->x; i ++) {
		wrote += syslib_fpfv_(file,elf_getval(rt,i),lfalse);
	}
	elf_putint(rt,wrote);
	return 1;
}


int syslib_lpf(elf_Runtime *rt) {
	for (int i = 0; i < rt->f->x; i ++) {
		if (i != 0) fprintf(stdout,"\n");
		syslib_fpfv_(stdout,elf_getval(rt,i),lfalse);
	}
	fprintf(stdout,"\n");
	return 0;
}


int syslib_pf(elf_Runtime *rt) {
	for (int i = 0; i < rt->f->x; i ++) {
		syslib_fpfv_(stdout,elf_getval(rt,i),lfalse);
	}
	fprintf(stdout,"\n");
	return 0;
}


lapi int syslib_sleep(elf_Runtime *rt) {
	elf_assert(rt->f->x == 1);
	sys_sleep(elf_getint(rt,0));
	return 0;
}


lapi int syslib_clocktime(elf_Runtime *rt) {
	elf_putint(rt,sys_clocktime());
	return 1;
}


lapi int syslib_timediffs(elf_Runtime *rt) {
	elf_assert(rt->f->x == 1);
	elf_int i = elf_getint(rt,0);
	elf_putnum(rt,(sys_clocktime() - i) / (elf_num) sys_clockhz());
	return 1;
}


elf_bool isvirtual(char const *fn) {
	while (*fn == '.') ++ fn;
	return *fn == 0;
}


elf_globaldecl elf_String *keyname;
elf_globaldecl elf_String *keypath;
elf_globaldecl elf_String *keyisdir;
void syslib_listdir_(elf_Runtime *R, elf_String *d, elf_Table *list, elf_Table *flags, elf_Closure *cl) {
#if defined(_WIN32)
	elf_Module *md = R->md;

	char *dir = S_tpf("%s\\*",d->c);
	WIN32_FIND_DATAA f;
	HANDLE h = FindFirstFileA(dir,&f);

	if (h != INVALID_HANDLE_VALUE) do {
		if (isvirtual(f.cFileName)) continue;

		int isdir = 0 != (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		elf_String *name = elf_newstr(R,f.cFileName);
		elf_String *path = elf_newstr(R,S_tpf("%s\\%s",d->string,f.cFileName));

		llocalid base = elf_putcl(R,cl);
		elf_Table *file = elf_pushnewtab(R);
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
#endif
}


lapi int syslib_listdir(elf_Runtime *R) {
	elf_assert(R->frame->x == 2);
	/* push these keys temporarily so they won't
	be gc'd and also to to avoid creating them so often  */
	keyname = elf_pushnewstr(R,"name");
	keypath = elf_pushnewstr(R,"path");
	keyisdir = elf_pushnewstr(R,"isdir");
	elf_Closure *cl = elf_getcls(R,1);
	elf_String *dir = elf_getstr(R,0);
	elf_Table *flags = elf_pushnewtab(R);
	/* push list last to serve as return value */
	elf_Table *list = elf_pushnewtab(R);
	syslib_listdir_(R,dir,list,flags,cl);
	return 1;
}


/* todo: can we do this from code */
lapi void syslib_load(elf_Runtime *R) {
	/* todo: these shouldn't be here */
	elf_register(R,"ntoi",syslib_ntoi);
	elf_register(R,"iton",syslib_iton);

	/* todo: deprecated */
	elf_register(R,"loadexpr",syslib_loadexpr);
	elf_register(R,"loadfile",syslib_loadfile);
	elf_register(R,"clocktime",syslib_clocktime);
	elf_register(R,"timediffs",syslib_timediffs);
	elf_register(R,"fpf",syslib_fpf);
	elf_register(R,"pf",syslib_pf);
	elf_register(R,"lpf",syslib_lpf);
	elf_register(R,"loadlib",syslib_loadlib);
	elf_register(R,"libfn",syslib_libfn);


	elf_register(R,"elf.loadlib",syslib_loadlib);
	elf_register(R,"elf.libfn",syslib_libfn);
	elf_register(R,"elf.loadexpr",syslib_loadexpr);
	elf_register(R,"elf.loadfile",syslib_loadfile);
	elf_register(R,"elf.sys.clocktime",syslib_clocktime);
	elf_register(R,"elf.sys.timediffs",syslib_timediffs);
	elf_register(R,"elf.sys.fpf",syslib_fpf);
	elf_register(R,"elf.sys.lpf",syslib_lpf);
	elf_register(R,"elf.sys.pf",syslib_pf);



	elf_register(R,"workdir",syslib_workdir);
	elf_register(R,"pwd",syslib_pwd);
	elf_register(R,"setpwd",syslib_setpwd);
	elf_register(R,"freadall",syslib_freadall);
	elf_register(R,"ftemp",syslib_ftemp);
	elf_register(R,"listdir",syslib_listdir);
	elf_register(R,"sleep",syslib_sleep);
	elf_register(R,"exec",syslib_exec);
}

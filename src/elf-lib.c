/*
** See Copyright Notice In elf.h
** elf-lib.c
** elf lib
*/


int elflib_GCM(elf_Runtime *R) {
	elf_putint(R,R->gcmemory);
	return 1;
}


int elflib_GCT(elf_Runtime *R) {
	elf_putint(R,R->gcthreshold);
	return 1;
}


int elflib_GCN(elf_Runtime *R) {
	elf_putint(R,elf_varlen(R->gc));
	return 1;
}




int elflib_iton(elf_Runtime *R) {
	elf_val v = elf_getval(R,0);
	if (v.tag == TAG_INT) {
		elf_putnum(R,(elf_num)v.i);
	} else elf_putnum(R,v.n);
	return 1;
}


int elflib_ntoi(elf_Runtime *R) {
	elf_val v = elf_getval(R,0);
	if (v.tag == TAG_NUM) {
		elf_putint(R,(elf_int)v.n);
	} else elf_putint(R,v.i);
	return 1;
}


int elflib_loadexpr(elf_Runtime *R) {
	elf_String *contents = elf_getstr(R,0);
	elf_loadexpr(R,contents,R->call->ry,R->call->ny);
	/* no need to do hoisting */
	return 0;
}


int elflib_loadfile(elf_Runtime *R) {
	elf_String *filename = elf_getstr(R,0);
	elf_FileState fs = {0};
	elf_loadfile(R,&fs,filename,R->call->ry,R->call->ny);
	/* no need to do hoisting */
	return 0;
}


int elflib_libfn(elf_Runtime *rt) {
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



// typedef void (*em_dlopen_callback)(void* handle, void* user_data);
// void emscripten_dlopen(const char *filename, int flags, void* user_data, em_dlopen_callback onsuccess, em_arg_callback_func onerror);


int elflib_loadlib(elf_Runtime *R) {
	elf_String *name = elf_getstr(R,0);
	elf_Closure *callback = elf_getcls(R,1);
	elf_Handle lib = sys_loadlib(name->c);
	if (lib != lnil) {
		elf_putsys(R,lib);
	} else elf_putnil(R);
	return 1;
}


int elflib_exec(elf_Runtime *R) {
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


int elflib_freadall(elf_Runtime *R) {
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


int elflib_ftemp(elf_Runtime *R) {
	FILE *file = {0};
#if defined(PLATFORM_WEB)
	file = tmpfile();
#else
	tmpfile_s(&file);
#endif
	elf_putsys(R,(elf_Handle)file);
	return 1;
}


int elflib_workdir(elf_Runtime *rt) {
	elf_logerror("this function is deprecated");
	char buf[MAX_PATH];
	sys_pwd(sizeof(buf),buf);
	elf_locnewstr(rt,buf);
	if (rt->f->x == 1) {
		elf_String *s = elf_getstr(rt,0);
		sys_setpwd(s->c);
	}
	return 1;
}


int elflib_pwd(elf_Runtime *rt) {
	char buf[MAX_PATH];
	sys_pwd(sizeof(buf),buf);
	elf_locnewstr(rt,buf);
	return 1;
}


int elflib_setpwd(elf_Runtime *rt) {
	elf_String *s = elf_getstr(rt,0);
	sys_setpwd(s->c);
	return 0;
}


int elflib_fpf(elf_Runtime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	int wrote = 0;
	for (int i = 1; i < rt->f->x; i ++) {
		wrote += elf_valfpf(file,elf_getval(rt,i),lfalse);
	}
	elf_putint(rt,wrote);
	return 1;
}


int elflib_lpf(elf_Runtime *rt) {
	for (int i = 0; i < rt->f->x; i ++) {
		if (i != 0) fprintf(stdout,"\n");
		elf_valfpf(stdout,elf_getval(rt,i),lfalse);
	}
	fprintf(stdout,"\n");
	return 0;
}


int elflib_pf(elf_Runtime *rt) {
	for (int i = 0; i < rt->f->x; i ++) {
		elf_valfpf(stdout,elf_getval(rt,i),lfalse);
	}
	fprintf(stdout,"\n");
	return 0;
}


elf_api int elflib_sleep(elf_Runtime *rt) {
	elf_ensure(rt->f->x == 1);
	sys_sleep(elf_getint(rt,0));
	return 0;
}


elf_api int elflib_clocktime(elf_Runtime *rt) {
	elf_putint(rt,sys_clocktime());
	return 1;
}


elf_api int elflib_timediffs(elf_Runtime *rt) {
	elf_ensure(rt->f->x == 1);
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

void elflib_listdir_(elf_Runtime *R, elf_String *d, elf_Table *list, elf_Table *flags, elf_Closure *cl) {
#if defined(PLATFORM_DESKTOP)
	WIN32_FIND_DATAA f;
	HANDLE h = FindFirstFileA(elf_tpf("%s\\*",d->c),&f);
	if (h != INVALID_HANDLE_VALUE) do {
		if (isvirtual(f.cFileName)) continue;

		int isdir = 0 != (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		elf_String *name = elf_newstr(R,f.cFileName);
		elf_String *path = elf_newstr(R,elf_tpf("%s\\%s",d->c,f.cFileName));

		llocalid base = elf_putcls(R,cl);
		elf_Table *file = elf_locnewtab(R);
		elf_puttab(R,flags);
		elf_tabsetstrfld(file,keyname,name);
		elf_tabsetstrfld(file,keypath,path);
		elf_tabsetintfld(file,keyisdir,isdir);

		int r = elf_callfn(R,base,2,1);
		if ((r > 0) && elf_getint(R,base)) {
			elf_tabset(list,elf_valstr(path),lang_T(file));
			if (isdir) {
				elflib_listdir_(R,path,list,flags,cl);
			}
		}
	} while (FindNextFileA(h,&f));
#elif defined(PLATFORM_WEB)
	DIR *dir = opendir(d->c);
	if (dir != lnil) {
		struct dirent *entry;
		while ((entry = readdir(dir)) != lnil) {
			if (isvirtual(entry->d_name)) {
			 	continue;
			}
			elf_bool isdir = (entry->d_type & DT_DIR) != lfalse;
			elf_val *top = elf_gettop(R);
			elf_String *name = elf_locnewstr(R,entry->d_name);
			elf_String *path = elf_locnewstr(R,elf_tpf("%s/%s",d->c,entry->d_name));

			llocalid base = elf_putcls(R,cl);
			/* file and flags arguments */
			elf_Table *file = elf_locnewtab(R);
			elf_puttab(R,flags);
			elf_tabsetstrfld(file,keyname,name);
			elf_tabsetstrfld(file,keypath,path);
			elf_tabsetintfld(file,keyisdir,isdir);

			int r = elf_callfn(R,base,2,1);
			if ((r > 0) && elf_getint(R,base)) {
				elf_tabset(list,elf_valstr(path),lang_T(file));
				if (isdir) {
					elflib_listdir_(R,path,list,flags,cl);
				}
			}
			elf_settop(R,top);
		}
		closedir(dir);
	}
#endif
}


elf_api int elflib_listdir(elf_Runtime *R) {
	elf_ensure(R->frame->x == 2);
	/* push these keys temporarily so they won't
	be gc'd and also to to avoid creating them so often  */
	keyname = elf_locnewstr(R,"name");
	keypath = elf_locnewstr(R,"path");
	keyisdir = elf_locnewstr(R,"isdir");
	elf_Closure *cl = elf_getcls(R,1);
	elf_String *dir = elf_getstr(R,0);
	elf_Table *flags = elf_locnewtab(R);
	/* push list last to serve as return value */
	elf_Table *list = elf_locnewtab(R);
	elflib_listdir_(R,dir,list,flags,cl);
	return 1;
}


/* todo: can we do this from code */
elf_api void elflib_load(elf_Runtime *R) {
#if defined(PLATFORM_WEB)
	elf_registerint(R,"elf.PLATFORM_WEB",1);
#endif
	/* todo: these shouldn't be here */
	elf_register(R,"ntoi",elflib_ntoi);
	elf_register(R,"iton",elflib_iton);

	/* todo: deprecated */
	elf_register(R,"loadexpr",elflib_loadexpr);
	elf_register(R,"loadfile",elflib_loadfile);
	elf_register(R,"clocktime",elflib_clocktime);
	elf_register(R,"timediffs",elflib_timediffs);
	elf_register(R,"fpf",elflib_fpf);
	elf_register(R,"pf",elflib_pf);
	elf_register(R,"lpf",elflib_lpf);
	elf_register(R,"loadlib",elflib_loadlib);
	elf_register(R,"libfn",elflib_libfn);


	elf_register(R,"elf.loadlib",elflib_loadlib);
	elf_register(R,"elf.libfn",elflib_libfn);
	elf_register(R,"elf.loadexpr",elflib_loadexpr);
	elf_register(R,"elf.loadfile",elflib_loadfile);
	elf_register(R,"elf.sys.clocktime",elflib_clocktime);
	elf_register(R,"elf.sys.timediffs",elflib_timediffs);
	elf_register(R,"elf.fpf",elflib_fpf);
	elf_register(R,"elf.lpf",elflib_lpf);
	elf_register(R,"elf.pf",elflib_pf);
	elf_register(R,"elf.GCN",elflib_GCN);
	elf_register(R,"elf.GCT",elflib_GCT);
	elf_register(R,"elf.GCM",elflib_GCM);



	elf_register(R,"workdir",elflib_workdir);
	elf_register(R,"pwd",elflib_pwd);
	elf_register(R,"setpwd",elflib_setpwd);
	elf_register(R,"freadall",elflib_freadall);
	elf_register(R,"ftemp",elflib_ftemp);
	elf_register(R,"listdir",elflib_listdir);
	elf_register(R,"sleep",elflib_sleep);
	elf_register(R,"exec",elflib_exec);
}

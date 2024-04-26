/*
** See Copyright Notice In lang.h
** ltest.c
** Test Tools
*/




int testlib___of(lRuntime *R) {
	elf_getobj(R,1)->metatable = elf_gettab(R,0);
	elf_putval(R,elf_getval(R,1));
	return 1;
}





void sets(lRuntime *c, elf_tab *table, char const *k) {
	elf_tabput(table,lang_S(elf_newstr(c,k)),* -- c->v);
}


int testlib_logging(lRuntime *R) {
	elf_int logging = elf_getint(R,0);
	R->call->caller->logging = logging;
	return 0;
}


int testlib_globallogging(lRuntime *R) {
	elf_int logging = elf_getint(R,0);
	R->logging = logging;
	return 0;
}


int testlib_debugbreak(lRuntime *R) {
	R->debugbreak = ltrue;
	return 0;
}



#define BUFFER 0x10000


/* todo: this is so unsafe is crazy */
void strcatf(char *buffer, char *fmt, ...) {
	char *cursor = buffer;
	while (*cursor != 0) ++ cursor;
	va_list v;
	va_start(v,fmt);
	stbsp_vsnprintf(cursor,BUFFER-(cursor-buffer),fmt,v);
	va_end(v);
}


int testlib_disasm(lRuntime *c) {
	elf_Module *md = c->md;
	elf_Closure *cl = elf_getcls(c,0);
	elf_Proto p = cl->fn;
	char file[BUFFER];
	langM_clear(file,sizeof(file));
	int j;
	for (j = 0; j < p.nbytes; ++j) {
		if (j != 0) strcatf(file,"\n");
		lBytecode b = md->bytes[p.bytes+j];
		switch (b.k) {
			case BC_LOADFILE:
			case BC_METACALL:
			case BC_CALL: {
				strcatf(file,"%s(%i,%i)", lang_bytename(b.k), b.x,b.y);
			} break;
			default: {
				strcatf(file,"%s(%lli)", lang_bytename(b.k), b.i);
			} break;
		}
	}
	elf_pushnewstr(c,file);
	return 1;
}


int testlib_absslot(lRuntime *c) {
	llocalid slot = elf_getint(c,0);
	elf_putval(c,c->s[slot]);
	return 1;
}


int testlib_absslotid(lRuntime *c) {
	elf_putint(c,c->v-c->s);
	return 1;
}


int testlib_pc(lRuntime *c) {
	elf_putint(c,c->f->j);
	return 1;
}


int testlib_gcpause(lRuntime *c) {
	langGC_pause(c);
	return 0;
}


int testlib_gcunpause(lRuntime *c) {
	langGC_unpause(c);
	return 0;
}


int testlib_gc(lRuntime *c) {
	elf_collect(c);
	return 0;
}


int _gidof(elf_Module *fs, elf_obj *j) {
	elf_forivar(fs->g->v) {
		if (fs->g->v[i].j == j) {
			return i;
		}
	}
	return -1;
}


int _gtable(lRuntime *c) {
	elf_puttab(c,c->md->g);
	return 1;
}


char *gccolor2s(GCColor c) {
	return
	c == GC_BLACK ? "black" :
	c == GC_WHITE ? "white" :
	c == GC_PINK  ? "pink"  :
	c == GC_RED   ? "red"   : "unknown";
}


void tstlib_load(lRuntime *rt) {
	elf_Module *md = rt->md;
	/* todo: ugly */
	lang_addglobal(md,elf_pushnewstr(rt,"__of"),lang_C(testlib___of));

	lang_addglobal(md,elf_pushnewstr(rt,"gc"),lang_C(testlib_gc));
	lang_addglobal(md,elf_pushnewstr(rt,"gcpause"),lang_C(testlib_gcpause));
	lang_addglobal(md,elf_pushnewstr(rt,"gcunpause"),lang_C(testlib_gcunpause));
	lang_addglobal(md,elf_pushnewstr(rt,"__disasm"),lang_C(testlib_disasm));
	lang_addglobal(md,elf_pushnewstr(rt,"__logging"),lang_C(testlib_logging));
	lang_addglobal(md,elf_pushnewstr(rt,"__globallogging"),lang_C(testlib_globallogging));

	lang_addglobal(md,elf_pushnewstr(rt,"__debugbreak"),lang_C(testlib_debugbreak));
	lang_addglobal(md,elf_pushnewstr(rt,"absslotid"),lang_C(testlib_absslotid));
	lang_addglobal(md,elf_pushnewstr(rt,"absslot"),lang_C(testlib_absslot));
	lang_addglobal(md,elf_pushnewstr(rt,"pc"),lang_C(testlib_pc));
	lang_addglobal(md,elf_pushnewstr(rt,"_gtable"),lang_C(_gtable));
}
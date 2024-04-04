/*
** See Copyright Notice In lang.h
** ltest.c
** Test Tools
*/


void sets(lRuntime *c, Table *table, char const *k) {
	langH_insert(table,lang_S(langS_new(c,k)),* -- c->v);
}


int testlib_logging(lRuntime *c) {
	llongint logging = lang_loadlong(c,0);
	c->logging = logging;
	return 0;
}


int testlib_debugbreak(lRuntime *c) {
	__debugbreak();
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
	lModule *md = c->md;
	lClosure *cl = lang_loadcl(c,0);
	lProto p = cl->fn;
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
	lang_pushnewS(c,file);
	return 1;
}


int testlib_absslot(lRuntime *c) {
	llocalid slot = lang_loadlong(c,0);
	lang_pushvalue(c,c->s[slot]);
	return 1;
}


int testlib_absslotid(lRuntime *c) {
	lang_pushlong(c,c->v-c->s);
	return 1;
}


int testlib_pc(lRuntime *c) {
	lang_pushlong(c,c->f->j);
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
	langGC_collect(c);
	return 0;
}


int _gidof(lModule *fs, lObject *j) {
	langA_varifor(fs->g->v) {
		if (fs->g->v[i].j == j) {
			return i;
		}
	}
	return -1;
}


int _gtable(lRuntime *c) {
	lang_pushtable(c,c->md->g);
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
	lModule *md = rt->md;
	/* todo: ugly */
	lang_addglobal(md,lang_pushnewS(rt,"gc"),lang_C(testlib_gc));
	lang_addglobal(md,lang_pushnewS(rt,"gcpause"),lang_C(testlib_gcpause));
	lang_addglobal(md,lang_pushnewS(rt,"gcunpause"),lang_C(testlib_gcunpause));
	lang_addglobal(md,lang_pushnewS(rt,"__disasm"),lang_C(testlib_disasm));
	lang_addglobal(md,lang_pushnewS(rt,"__logging"),lang_C(testlib_logging));

	lang_addglobal(md,lang_pushnewS(rt,"__debugbreak"),lang_C(testlib_debugbreak));
	lang_addglobal(md,lang_pushnewS(rt,"absslotid"),lang_C(testlib_absslotid));
	lang_addglobal(md,lang_pushnewS(rt,"absslot"),lang_C(testlib_absslot));
	lang_addglobal(md,lang_pushnewS(rt,"pc"),lang_C(testlib_pc));
	lang_addglobal(md,lang_pushnewS(rt,"_gtable"),lang_C(_gtable));
}
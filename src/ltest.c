/*
** See Copyright Notice In lang.h
** ltest.c
** Test Tools
*/


void sets(Runtime *c, Table *table, char const *k) {
	langH_insert(table,lang_S(langS_new(c,k)),* -- c->v);
}


int testlib_stkid(Runtime *c) {
	langR_pushI(c,c->v-c->s);
	return 1;
}


int testlib_pc(Runtime *c) {
	langR_pushI(c,c->f->j);
	return 1;
}


int testlib_gcpause(Runtime *c) {
	langGC_pause(c);
	return 0;
}


int testlib_gcunpause(Runtime *c) {
	langGC_unpause(c);
	return 0;
}


int testlib_gc(Runtime *c) {
	langGC_collect(c);
	return 0;
}


int _gidof(Module *fs, Object *j) {
	langA_varifor(fs->g->v) {
		if (fs->g->v[i].j == j) {
			return i;
		}
	}
	return -1;
}


int _gtable(Runtime *c) {
	langR_pushH(c,c->md->g);
	return 1;
}


char *gccolor2s(GCColor c) {
	return
	c == GC_BLACK ? "black" :
	c == GC_WHITE ? "white" :
	c == GC_PINK  ? "pink"  :
	c == GC_RED   ? "red"   : "unknown";
}


LAPI void testlib_open(Runtime *rt) {
	Module *md = rt->md;
	/* todo: ugly */
	lang_addglobal(md,langR_pushnewS(rt,"gc"),lang_C(testlib_gc));
	lang_addglobal(md,langR_pushnewS(rt,"gcpause"),lang_C(testlib_gcpause));
	lang_addglobal(md,langR_pushnewS(rt,"gcunpause"),lang_C(testlib_gcunpause));

	lang_addglobal(md,langR_pushnewS(rt,"stkid"),lang_C(testlib_stkid));
	lang_addglobal(md,langR_pushnewS(rt,"pc"),lang_C(testlib_pc));
	lang_addglobal(md,langR_pushnewS(rt,"_gtable"),lang_C(_gtable));
}
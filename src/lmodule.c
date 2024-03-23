/*
** See Copyright Notice In lang.h
** lmodule.c
** Object/Bytecode Module
*/


/* todo: ensure that we don't have replace
symbols */
Symbol lang_addsymbol(Module *md, String *name) {
	if (name == 0) {
		return langA_variadd(md->g->v,1);
	}
	return langH_take(md->g,lang_S(name));
}


Symbol lang_addglobal(Module *md, String *name, Value v) {
	Symbol i = lang_addsymbol(md,name);
	md->g->v[i] = v;
	return i;
}


Symbol lang_addproto(Module *md, Proto p) {
	Symbol i = langA_variadd(md->p,1);
	md->p[i] = p;
	return i;
}
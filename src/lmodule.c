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


void syslib_fpfv_(FILE *file, Value v, Bool quotes);
void lang_dumpmodule(Module *md, char *name) {
	Handle file = sys_fopen(name,"wb");
	fprintf(file,"Module:\n");
	fprintf(file,"Globals:\n");
	langA_varifor(md->g->v) {
		fprintf(file,"%lli: ", i);
		syslib_fpfv_(file,md->g->v[i],True);
		fprintf(file,"\n");
	}

	langA_varifor(md->p) {
		Proto p = md->p[i];
		fprintf(file,"Proto: [%i] %i,%i (%i:%i):\n",(int)i,p.bytes,p.nbytes,p.arity,p.nlocals);
		int j;
		for (j = 0; j < p.nbytes; ++j) {
			Bytecode b = md->bytes[p.bytes+j];
			fprintf(file,"+%i: %s, %lli ;  "
			, j, lang_bytename(b.k), b.i);
			if (b.k == BYTE_GGET) {
				syslib_fpfv_(file,md->g->v[b.i],True);
			}
			fprintf(file,"\n");
		}
		fprintf(file,"end\n");
	}

}
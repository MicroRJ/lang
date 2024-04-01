/*
** See Copyright Notice In lang.h
** lmodule.c
** lObject/Bytecode lModule
*/


/* todo: ensure that we don't have replace
symbols */
lglobalid lang_addsymbol(lModule *md, lString *name) {
	if (name == 0) {
		return langA_variadd(md->g->v,1);
	}
	return langH_take(md->g,lang_S(name));
}


lglobalid lang_addglobal(lModule *md, lString *name, lValue v) {
	lglobalid i = lang_addsymbol(md,name);
	md->g->v[i] = v;
	return i;
}


lglobalid lang_addproto(lModule *md, Proto p) {
	lglobalid i = langA_variadd(md->p,1);
	md->p[i] = p;
	return i;
}


char *linestart(char *p) {
	return p;
}


int linelen(char *p, int m) {
	int n = 0;
	while (p[n] != 0 && p[n] != '\r' && p[n] != '\n') ++ n;
	return m < n ? m : n;
}


/* this is silly */
int syslib_fpfv_(FILE *file, lValue v, lbool quotes);
void lang_dumpmodule(lModule *md, Handle file) {
	fprintf(file,"lModule:\n");
	fprintf(file,"Globals:\n");
	langA_varifor(md->g->v) {
		fprintf(file,"%04llX: ", i);
		syslib_fpfv_(file,md->g->v[i],ltrue);
		fprintf(file,"\n");
	}

	langA_varifor(md->p) {
		Proto p = md->p[i];
		fprintf(file,"FUNC: [%i] %i,%i (%i:%i):\n",(int)i,p.bytes,p.nbytes,p.x,p.nlocals);
		int j;
		for (j = 0; j < p.nbytes; ++j) {
			Bytecode b = md->bytes[p.bytes+j];

			fprintf(file,"%04X: %-18s (%llX) ;  "
			, j, lang_bytename(b.k), b.i);

			if (b.k == BYTE_GLOBAL) {
				syslib_fpfv_(file,md->g->v[b.i],ltrue);
			}
			fprintf(file,"\n");
		}
		fprintf(file,"end\n");
	}

}
/*
** See Copyright Notice In lang.h
** lmodule.c
** lObject/lBytecode lModule
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


lglobalid lang_addproto(lModule *md, lProto p) {
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


int syslib_fpfv_(FILE *file, lValue v, lbool quotes);
void bytefpf(lModule *md, FILE *file, lbyteid id, lBytecode b) {
	fprintf(file," | %-4i: %s"
	,	id, lang_bytename(b.k));
	if (lang_byteclass(b.k) == BYTE_CLASS_XY) {
		fprintf(file,"(x=%i,y=%i)",b.x,b.y);
	} else {
		fprintf(file,"(i=%lli)",b.i);
	}
	if (b.k == BYTE_GLOBAL) {
		fprintf(file,"  // ");
		syslib_fpfv_(file,md->g->v[b.i],ltrue);
	}
	fprintf(file,"\n");
}


void lang_dumpmodule(lModule *md, Handle file) {
	fprintf(file,"lModule:\n");
	fprintf(file,"Globals:\n");
	langA_varifor(md->g->v) {
		fprintf(file,"%04llX: ", i);
		syslib_fpfv_(file,md->g->v[i],ltrue);
		fprintf(file,"\n");
	}

	langA_varifor(md->p) {
		lProto p = md->p[i];
		fprintf(file,"FUNC: [%i] %i,%i (%i:%i):\n",(int)i,p.bytes,p.nbytes,p.x,p.nlocals);

		for (lbyteid j = 0; j < p.nbytes; ++j) {
			lBytecode b = md->bytes[p.bytes+j];
			bytefpf(md,file,j,b);
		}

		fprintf(file,"end\n");
	}

}
/*
** See Copyright Notice In lang.h
** lmodule.c
** lObject/lBytecode lModule
*/


/* todo: ensure that we don't have to replace symbols */
lglobalid lang_addsymbol(lModule *md, lString *name) {
	if (name == 0) {
		return langA_variadd(md->g->v,1);
	}
	lglobalid id = langH_take(md->g,lang_S(name));
	// lang_logdebug("global '%s' -> %i",name->c,id);
	return id;
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
	fprintf(file,"%04i\t%s"
	,	id, lang_bytename(b.k));
	if (lang_byteclass(b.k) == BC_CLASS_XYZ) {
		fprintf(file,"(x=%i,y=%i,z=%i)",b.x,b.y,b.z);
	} else
	if (lang_byteclass(b.k) == BC_CLASS_XY) {
		fprintf(file,"(x=%i,y=%i)",b.x,b.y);
	} else {
		fprintf(file,"(x=%lli)",b.i);
	}
	if (b.k == BC_LOADGLOBAL) {
		fprintf(file,"  // ");
		syslib_fpfv_(file,md->g->v[b.y],ltrue);
	}
	fprintf(file,"\n");
}


void langX_getlocinfo(FileState *fs, char *loc, int *linenum, char **lineloc);


void lang_dumpmodule(lModule *md, Handle file) {
#if 0
	fprintf(file,"lModule:\n");
	fprintf(file,"Globals:\n");
	langA_varifor(md->g->v) {
		fprintf(file,"%04llX: ", i);
		syslib_fpfv_(file,md->g->v[i],ltrue);
		fprintf(file,"\n");
	}
#endif
	fprintf(file,"-- BYTECODE --\n");
	langA_varifor(md->files) {
		lFile ff = md->files[i];
		fprintf(file,"- FILE (%s):\n",ff.name);
		fprintf(file,"INDEX INSTRUCTION\n");
		for (lbyteid j = 0; j < ff.nbytes; ++j) {
			lBytecode b = md->bytes[ff.bytes+j];
			// int linenum;
			// char *lineloc;
			// langX_getlocinfo(md->file,md->lines[j],&linenum,&lineloc);
			// fprintf(file,"%-3i:%-3i",linenum,(int)(md->lines[j]-lineloc));
			bytefpf(md,file,j,b);
		}
	}
#if 0
	langA_varifor(md->p) {
		lProto p = md->p[i];
		fprintf(file,"FUNC: [%i] %i,%i (%i:%i):\n",(int)i,p.bytes,p.nbytes,p.x,p.nlocals);
		for (lbyteid j = 0; j < p.nbytes; ++j) {
			lBytecode b = md->bytes[p.bytes+j];
			bytefpf(md,file,j,b);
		}
		fprintf(file,"end\n");
	}
#endif
}
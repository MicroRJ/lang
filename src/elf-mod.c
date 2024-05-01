/*
** See Copyright Notice In elf.h
** elf-mod.c
** Module
*/


/* todo: ensure that we don't have to replace symbols */
lglobalid elf_setsym(elf_Module *M, elf_String *name) {
	if (name != 0) {
		return elf_tabtake(M->g,elf_valstr(name));
	} else return elf_varaddi(M->globals->array,1);
}


lglobalid lang_addglobal(elf_Module *M, elf_String *name, elf_val v) {
	lglobalid i = elf_setsym(M,name);
	M->globals->array[i] = v;
	return i;
}


lglobalid lang_addproto(elf_Module *M, elf_Proto p) {
	lglobalid i = elf_varaddi(M->p,1);
	M->p[i] = p;
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


int elf_valfpf(FILE *file, elf_val v, elf_bool quotes);
void elf_bytefpf(elf_Module *md, FILE *file, lbyteid id, lBytecode b) {

	int fileid = elf_fndlinefile(md,md->lines[id]);
	elf_File codefile = md->files[fileid];
	int linenum;
	elf_getlinelocinfo(codefile.lines,md->lines[id],&linenum,0);

	fprintf(file,"%04i: \t",linenum);
	fprintf(file,"%08i %04i\t%s"
	, md->track[id],	id, lang_bytename(b.k));
	if (lang_byteclass(b.k) == BC_CLASS_XYZ) {
		fprintf(file,"(x=%i,y=%i,z=%i)",b.x,b.y,b.z);
	} else
	if (lang_byteclass(b.k) == BC_CLASS_XY) {
		fprintf(file,"(x=%i,y=%i)",b.x,b.y);
	} else {
		fprintf(file,"(x=%lli)",b.i);
	}
	if (b.k == BC_TYPEGUARD) {
		fprintf(file," #%s",tag2s[b.y]);
	} else
	if (b.k == BC_LOADINT) {
		fprintf(file," #%lli",md->ki[b.y]);
	} else
	if (b.k == BC_LOADNUM) {
		fprintf(file," #%f",md->kn[b.y]);
	} else
	if (b.k == BC_LOADGLOBAL) {
		elf_val val = md->globals->array[b.y];
		fprintf(file,"  // %s ",tag2s[val.tag]);
		/* todo: just pass in a flag to val fpf that tells
		it to shorten the thing for printing purposes */
		if ((val.tag == TAG_STR) || (val.tag == TAG_NUM) || (val.tag == TAG_INT)) {
			elf_valfpf(file,val,ltrue);
		}
	}
	fprintf(file,"\n");
}


void elf_getlinelocinfo(char *q, char *p, int *linenum, char **lineloc);


void lang_dumpmodule(elf_Module *md, elf_Handle file) {
#if 0
	fprintf(file,"elf_Module:\n");
	fprintf(file,"Globals:\n");
	elf_arrfori(md->g->v) {
		fprintf(file,"%04llX: ", i);
		elf_valfpf(file,md->g->v[i],ltrue);
		fprintf(file,"\n");
	}
#endif
	fprintf(file,"-- BYTECODE --\n");
	fprintf(file,"- INSTR: %i\n",md->nbytes);
	fprintf(file,"- PID: %i\n",sys_getmypid());
	elf_arrfori(md->files) {
		elf_File ff = md->files[i];
		fprintf(file,"- FILE (%s):\n",ff.name);
		fprintf(file,"INDEX INSTRUCTION\n");
		for (lbyteid j = 0; j < ff.nbytes; ++j) {
			lBytecode b = md->bytes[ff.bytes+j];
			// int linenum;
			// char *lineloc;
			// elf_getlinelocinfo(md->file,md->lines[j],&linenum,&lineloc);
			// fprintf(file,"%-3i:%-3i",linenum,(int)(md->lines[j]-lineloc));
			elf_bytefpf(md,file,j,b);
		}
	}
#if 0
	elf_arrfori(md->p) {
		elf_Proto p = md->p[i];
		fprintf(file,"FUNC: [%i] %i,%i (%i:%i):\n",(int)i,p.bytes,p.nbytes,p.x,p.nlocals);
		for (lbyteid j = 0; j < p.nbytes; ++j) {
			lBytecode b = md->bytes[p.bytes+j];
			elf_bytefpf(md,file,j,b);
		}
		fprintf(file,"end\n");
	}
#endif
}
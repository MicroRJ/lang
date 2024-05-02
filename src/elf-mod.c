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



int elf_valfpf(FILE *file, elf_val v, elf_bool quotes);
void elf_bytefpf(FILE *io, elf_Module *md, elf_int fid, lbyteid id, lBytecode b) {

	if (fid != -1) {
		elf_File codefile = md->files[fid];
		int linenum;
		elf_getlinelocinfo(codefile.lines,md->lines[id],&linenum,0);
		fprintf(io,"%s %04i: \t",codefile.name,linenum);
	}

	fprintf(io,"%08i %04i\t%s"
	, md->track[id],	id, lang_bytename(b.k));
	if (lang_byteclass(b.k) == BC_CLASS_XYZ) {
		fprintf(io,"(x=%i,y=%i,z=%i)",b.x,b.y,b.z);
	} else
	if (lang_byteclass(b.k) == BC_CLASS_XY) {
		fprintf(io,"(x=%i,y=%i)",b.x,b.y);
	} else {
		fprintf(io,"(x=%lli)",b.i);
	}
	if (b.k == BC_TYPEGUARD) {
		fprintf(io," #%s",tag2s[b.y]);
	} else
	if (b.k == BC_LOADINT) {
		fprintf(io," #%lli",md->ki[b.y]);
	} else
	if (b.k == BC_LOADNUM) {
		fprintf(io," #%f",md->kn[b.y]);
	} else
	if (b.k == BC_LOADGLOBAL) {
		elf_val val = md->globals->array[b.y];
		fprintf(io,"  // %s ",tag2s[val.tag]);
		/* todo: just pass in a flag to val fpf that tells
		it to shorten the thing for printing purposes */
		if ((val.tag == TAG_STR) || (val.tag == TAG_NUM) || (val.tag == TAG_INT)) {
			elf_valfpf(io,val,ltrue);
		}
	}
	fprintf(io,"\n");
}


void elf_getlinelocinfo(char *q, char *p, int *linenum, char **lineloc);


void lang_dumpmodule(elf_Module *md, elf_Handle io) {
#if 0
	fprintf(file,"elf_Module:\n");
	fprintf(file,"Globals:\n");
	elf_arrfori(md->g->v) {
		fprintf(file,"%04llX: ", i);
		elf_valfpf(file,md->g->v[i],ltrue);
		fprintf(file,"\n");
	}
#endif
	fprintf(io,"-- BYTECODE --\n");
	fprintf(io,"- INSTR: %i\n",md->nbytes);
	fprintf(io,"- PID: %i\n",sys_getmypid());
	elf_arrfori(md->files) {
		elf_File ff = md->files[i];
		fprintf(io,"- FILE (%s):\n",ff.name);
		fprintf(io,"INDEX INSTRUCTION\n");
		for (lbyteid j = 0; j < ff.nbytes; ++j) {
			lBytecode b = md->bytes[ff.bytes+j];
			// int linenum;
			// char *lineloc;
			// elf_getlinelocinfo(md->file,md->lines[j],&linenum,&lineloc);
			// fprintf(file,"%-3i:%-3i",linenum,(int)(md->lines[j]-lineloc));
			elf_bytefpf(io,md,i,j,b);
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
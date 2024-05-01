/*
** See Copyright Notice In elf.h
** elf-run.c
** Auxiliary Functions
*/





void elf_debugger(char *message) {
	sys_consolelog(ELF_LOGDBUG,"debugger: ");
	sys_consolelog(ELF_LOGDBUG,message);
	sys_consolelog(ELF_LOGDBUG,"end");
	sys_debugger();
}


void elf_registerint(elf_Runtime *R, char *name, int val) {
	lang_addglobal(R->M,elf_newlocstr(R,name),elf_valint(val));
}


void elf_register(elf_Runtime *R, char *name, lBinding fn) {
	lang_addglobal(R->M,elf_newlocstr(R,name),elf_valbid(fn));
}


void elf_tabmfld(elf_Runtime *R, elf_Table *obj, char *name, lBinding b) {
	elf_tabset(obj,elf_valstr(elf_newstr(R,name)),elf_valbid(b));
}


elf_int elf_clocktime() {
	return sys_clocktime();
}


/* todo: clockhz can be cached */
elf_num elf_timediffs(elf_int begin) {
	return (sys_clocktime() - begin) / (elf_num) sys_clockhz();
}


int elf_fndlinefile(elf_Module *md, llineid line) {
	elf_File *files = md->files;
	int nfiles = elf_varlen(files);
	for (int x = 0; x < nfiles; ++ x) {
		if (line < files[x].lines) continue;
		if (line > files[x].lines+files[x].nlines-1) continue;
		return x;
	}
	return -1;
}


elf_bool elf_chriseol(char x) {
	return x == '\r' || x == '\n' || x == '\0';
}


/* finds line number and line loc from single source location */
void elf_getlinelocinfo(char *q, char *loc, int *linenum, char **lineloc) {
	char *c = q;
	int n = 0;
	while (q < loc) {
		while (!elf_chriseol(*q) && q < loc) q ++;
		if (*q == 0) break;
		if ((*q != '\n') || (c = ++ q, n ++, 1)) {
			if ((*q == '\r') && (c = ++ q, n ++, 1)) {
				if (*q == '\n') c = ++ q;
			} else q ++;
		}
	}
	if (linenum != 0) *linenum = n + 1;
	if (lineloc != 0) *lineloc = c;
}


/* diagnostics function for syntax errors */
void elf_lineerror2(char *filename, char *contents, char *loc, char const *fmt, ...) {
	int linenum;
	char *lineloc;
	elf_getlinelocinfo(contents,loc,&linenum,&lineloc);

	/* skip initial blank characters for optimal gimmicky */
	while (*lineloc == '\t' || *lineloc == ' ') {
		lineloc += 1;
	}

	char u[0x40];

	int underline = loc - lineloc;
	if (underline >= sizeof(u)) {
		underline = sizeof(u)-1;
		lineloc = loc - underline;
	}

	int linelen = 0;
	for (;; ++ linelen) {
		if (lineloc[linelen] == '\0') break;
		if (lineloc[linelen] == '\r') break;
		if (lineloc[linelen] == '\n') break;
	}

	for (int i = 0; i < underline; ++ i) {
		u[i] = lineloc[i] == '\t' ? '\t' : '-';
	}
	u[underline]='^';

	if (fmt !=  0) {
		char b[0x1000];
		va_list v;
		va_start(v,fmt);
		stbsp_vsnprintf(b,sizeof(b),fmt,v);
		va_end(v);
		printf("%s [%i:%lli]: %s\n",filename,linenum,(elf_int)(1+loc-lineloc),b);
	}
	printf("| %.*s\n",linelen,lineloc);
	printf("| %.*s\n",underline+1,u);
}


void elf_throw(elf_Runtime *R, lbyteid id, char *error) {
	elf_Module *md = R->md;
	if (id == NO_BYTE) id = R->j;
	llineid line = md->lines[id];
	int fileid = elf_fndlinefile(md,line);
	if (fileid != -1) {
		elf_File *file = &md->files[fileid];
		elf_lineerror2(file->name,file->lines,line,error);
	}
	// elf_debugger("runtime throw");
}

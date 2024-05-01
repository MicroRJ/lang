/*
** See Copyright Notice In elf.h
** elf.c
** Compiles and runs .elf files
*/


#define ELF_KEEPWARNINGS
#include "elf.h"


int main(int n, char **c) {
	(void) n;
	if (n == 1) {
		printf("\n\tUSAGE: elf.exe [options] file...\n");
		return 0;
	}

	elf_inimem();
	elf_Module M = {0};
	elf_Runtime R = {0};
	elf_runini(&R,&M);

	elf_CallFrame frame = {0};
	frame.base = R.top;
	R.frame = &frame;

	elf_bool dump = lfalse;
	FILE *dumpf = lnil;
	char *codefile = "unnamed";
	char *contents = lnil;
	#define ISOP(N) (!strncmp(c[i],"--"N,2+sizeof(N)-1))
	#define ISARG() (strncmp(c[i],"--",2))
	#define HASARG() ((i+1<n) && strncmp(c[i+1],"--",2))
	#define GETARG(E) (HASARG() ? c[++i] : E)

	for (int i = 1; i < n; ++i) {
		if (ISARG()) {
			codefile = c[i];
		} else if (ISOP("dump")) {
			elf_loginfo("--dump: dumping module");
			dump = ltrue;
			char *filename = GETARG(0);
			if (filename != lnil) {
				if (!strcmp(filename,"stdout")) {
					dumpf = stdout;
				} else dumpf = fopen(filename,"wb");
				if (dumpf == lnil) {
					elf_logerror("'%s': file specified could not be opened for writting",filename);
				}
			}
		} else if (ISOP("code")) {
			elf_loginfo("--code: evaluating code (use string)");
			contents = GETARG(0);
			goto leave;
		}
	}

	elf_FileState fs = {0};
	elf_loadfile(&R,&fs,elf_newlocstr(&R,codefile),0,0);
	leave:
	if (contents != lnil) {
		elf_FileState fs = {0};
		elf_loadcode(&R,&fs,elf_newlocstr(&R,codefile),0,0,contents);
	}

	if (dump) {
		if (dump && !dumpf) {
			dumpf = fopen(elf_tpf("%s.module.ignore",codefile),"wb");
		}
		if (dumpf != lnil) lang_dumpmodule(&M,dumpf);
		if (dumpf != lnil) fclose(dumpf);
	}
	sys_consolelog(ELF_LOGINFO,"exited");
	return 0;
}



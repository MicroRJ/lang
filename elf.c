/*
** See Copyright Notice In elf.h
** elf.c
** Compiles and runs .elf files
*/


#define ELF_KEEPWARNINGS
#include "elf.h"

typedef struct cli_t {
	char *filename;
	int dump;
	int logging;
	char *dumpfilename;
} cli_t;
int parsecli(cli_t *cl, int n, char **c);


int main(int n, char **c) {
	(void) n;
	cli_t cli = {0};
	if (parsecli(&cli,n,c)) return 0;

	elf_inimem();
	elf_Module M = {0};
	elf_ThreadState R = {0};
	elf_runini(&R,&M);
	if (cli.logging) R.logging = ltrue;

	elf_CallFrame frame = {0};
	frame.base = R.top;
	R.frame = &frame;

	if (cli.filename != lnil) {
		elf_String *filename = elf_newlocstr(&R,cli.filename);
		filename->obj.gccolor = GC_PINK;
		elf_FileState fs = {0};
		elf_loadfile(&R,&fs,filename,0,0);
	}
	// leave:
	// if (contents != lnil) {
	// 	elf_String *filename = elf_newlocstr(&R,codefile);
	// 	filename->obj.gccolor = GC_PINK;
	// 	elf_FileState fs = {0};
	// 	elf_loadcode(&R,&fs,filename,0,0,contents);
	// }

	if (cli.dump) {
		FILE *dumpf = stdout;
		if (strcmp(cli.dumpfilename,"stdout")) {
			dumpf = fopen(elf_tpf("%s.module.ignore",cli.dumpfilename),"wb");
		}
		if (dumpf == lnil) {
			printf("error: could open specified dump file for writting");
		} else {
			lang_dumpmodule(&M,dumpf);
			if (dumpf != stdout) fclose(dumpf);
		}
	}
	sys_consolelog(ELF_LOGINFO,"exited");
	return 0;
}


int parsecli(cli_t *cli, int n, char **c) {
	int i = 1;
	int NOP = 0; (void) NOP;
	#define HAS() (i < n)
	#define NEXT() (i ++)
	#define ISOP(N) (!strncmp(c[i],"--"N,2+sizeof(N)-1))
	#define GETOP(N) (ISOP(N) ? NEXT() : NOP)
	#define ISARG() (strncmp(c[i],"--",2))
	#define GETARG(E) (ISARG() ? c[NEXT()] : E)
	#define FAIL(S) (printf("error: "S),lnil)
	#define SETFIELD(N,V) (cli->N = V)


#define OPLIST(_) \
	_("help", "displays command line interface help",{})\
	_("logging", "log bytecode instructions as they execute (only in debug mode)",{SETFIELD(logging,ltrue);})\
	_("dump", "[filename] whether to dump the resulting module (when program exits)",\
	{	SETFIELD(dump,ltrue);\
		SETFIELD(dumpfilename,GETARG(FAIL("dump: missing filename, tip: you can use [stdout]"))); })

	if (!HAS()) goto help;

	if (ISARG()) {
		cli->filename = GETARG(0);
		printf("using file: %s\n",cli->filename);
	}
	while (HAS()) {
		if (GETOP("help")) {
			help:
			printf("usage: --op [args...]\n");
#define DEFOP(N,I,A) printf("  --%s: %s\n", N,I);
			OPLIST(DEFOP)
#undef DEFOP
			return 1;
		}
#define DEFOP(N,I,A) if (GETOP(N)) { printf("%s:\n",N); A; continue; };
		OPLIST(DEFOP)
#undef DEFOP
		printf("error: unrecognized verb: %s\n",c[i]);
		break;
	}

	return 0;
}
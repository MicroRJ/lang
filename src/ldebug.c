/*
** See Copyright Notice In elf.h
** ldebug.c
** Debug Tools
*/


elf_globaldecl int (*lang_globalassertionhook)(ldebugloc);


void lang_setasserthook(int (*hook)(ldebugloc)) {
	lang_globalassertionhook = hook;
}


void lang_assertfn(ldebugloc ind, char const *name, elf_bool expr) {
	if (!expr) {
		printf("%s[%i] %s(): '%s' triggered assertion\n",ind.fileName,ind.lineNumber,ind.func,name);
		if (lang_globalassertionhook != lnil) {
			lang_globalassertionhook(ind);
		} else elf_debugger("assertion triggered");
	}
}

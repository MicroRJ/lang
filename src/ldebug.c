/*
** See Copyright Notice In lang.h
** ldebug.c
** Debug Tools
*/


lglobaldecl int (*lang_globalassertionhook)(Debugloc);


void lang_setasserthook(int (*hook)(Debugloc)) {
	lang_globalassertionhook = hook;
}


void lang_assertfn(Debugloc ind, char const *name, Bool expr) {
	if (!expr) {
		printf("%s[%i] %s(): '%s' triggered assertion\n",ind.fileName,ind.lineNumber,ind.func,name);
		if (lang_globalassertionhook != Null) {
			lang_globalassertionhook(ind);
		} else {
			__debugbreak();
		}
	}
}

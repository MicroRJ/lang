/*
** See Copyright Notice In elf.h
** elf_api.h
** Main user API
*/


elf_num elf_tonum(elf_val v) {
	return v.tag == TAG_INT ? (elf_num) v.i : v.n;
}


elf_int elf_toint(elf_val v) {
	return v.tag == TAG_NUM ? (elf_int) v.n : v.i;
}


elf_api elf_Object *elf_getthis(elf_Runtime *R);

elf_api elf_val elf_getval(elf_Runtime *R, llocalid x);
elf_api elf_int elf_getint(elf_Runtime *R, llocalid x);
elf_api elf_num elf_getnum(elf_Runtime *R, llocalid x);
elf_api elf_String *elf_getstr(elf_Runtime *R, llocalid x);
elf_api elf_Object *elf_getobj(elf_Runtime *R, llocalid x);
elf_api elf_Table *elf_gettab(elf_Runtime *R, llocalid x);
elf_api elf_Handle elf_getsys(elf_Runtime *R, llocalid x);
elf_api elf_Closure *elf_getcls(elf_Runtime *R, llocalid x);


/*
** Loads an expression from source string.
** The expression is converted to a
** function and is called as a root
** function, can only reference global
** expressions.
** The expression can be a function itself,
** in which case you call the function and
** pass in arguments.
*/
elf_api int elf_loadexpr(elf_Runtime *, elf_String *contents, llocalid x, llocalid y);



/*
** Loads a file and calls its function,
** returns the number of results.
*/
int elf_loadcode(elf_Runtime *, elf_FileState *fs, elf_String *filename, llocalid rx, int ny, char *contents);


/*
** Loads a file from disk and calls its function,
** returns the number of results.
*/
int elf_loadfile(elf_Runtime *, elf_FileState *fs, elf_String *filename, llocalid rx, int ny);


/*
** Ultimately, calls a function of any kind.
** Takes an optional object for meta calls,
** nx and ny are the number of in and out
** values respectively.
** nx does not include the function nor the
** optional object.
** rx is the frame register, the function
** should reside in that register at call
** time. arguments should come after that
** register.
** ry is the first yield register, where
** the results are written to.
** ry can be equal to rx.
*/
elf_api int elf_callex(elf_Runtime *, elf_Object *obj, llocalid rx, llocalid ry, int nx, int ny);


/*
** Performs a root call, where rx and ry are the same
** and obj is nil.
*/
elf_api int elf_callfn(elf_Runtime *, llocalid rx, int nx, int ny);


elf_api int elf_run(elf_Runtime *);


elf_api void elf_checkcl(elf_Runtime *c, llocalid x);
elf_api elf_String *elf_checkstr(elf_Runtime *c, llocalid x);


elf_api llocalid elf_stkput(elf_Runtime *R, int n);
elf_api llocalid elf_stklen(elf_Runtime *c);
elf_val *elf_gettop(elf_Runtime *R);
void elf_settop(elf_Runtime *R, elf_val *top);

elf_api llocalid elf_locval(elf_Runtime *, elf_val v);
elf_api void elf_locnil(elf_Runtime *);
elf_api void elf_locint(elf_Runtime *, elf_int i);
elf_api void elf_locnum(elf_Runtime *, elf_num n);
elf_api void elf_locsys(elf_Runtime *c, elf_Handle h);
elf_api void elf_loctab(elf_Runtime *, elf_Table *t);
elf_api void elf_locobj(elf_Runtime *, elf_Object *t);
elf_api llocalid elf_loccls(elf_Runtime *, elf_Closure *f);
elf_api void elf_locstr(elf_Runtime *, elf_String *s);
elf_api void elf_locbinding(elf_Runtime *, lBinding c);


elf_api elf_Object *elf_newlocobj(elf_Runtime *, elf_int tell);
elf_api elf_Table *elf_newloctab(elf_Runtime *);
elf_api elf_String *elf_newlocstr(elf_Runtime *, char *c);
elf_api llocalid elf_newloccls(elf_Runtime *, elf_Proto fn);


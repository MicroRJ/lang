/*
** See Copyright Notice In elf.h
** lapi.h
** Main user API
*/


lapi elf_Object *elf_getthis(elf_Runtime *R);

lapi elf_val elf_getval(elf_Runtime *R, llocalid x);
lapi elf_int elf_getint(elf_Runtime *R, llocalid x);
lapi elf_num elf_getnum(elf_Runtime *R, llocalid x);
lapi elf_String *elf_getstr(elf_Runtime *R, llocalid x);
lapi elf_Object *elf_getobj(elf_Runtime *R, llocalid x);
lapi elf_Table *elf_gettab(elf_Runtime *R, llocalid x);
lapi elf_Handle elf_getsys(elf_Runtime *R, llocalid x);
lapi elf_Closure *elf_getcls(elf_Runtime *R, llocalid x);


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
int elf_loadexpr(elf_Runtime *, elf_String *contents, llocalid x, llocalid y);


/*
** Loads a file and calls its function,
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
lapi int elf_callfn(elf_Runtime *, elf_Object *obj, llocalid rx, llocalid ry, int nx, int ny);


/*
** Performs a root call, where rx and ry are the same
** and obj is nil.
*/
lapi int elf_rootcall(elf_Runtime *, llocalid rx, int nx, int ny);


lapi int elf_run(elf_Runtime *);


lapi void elf_checkcl(elf_Runtime *c, llocalid x);
lapi elf_String *elf_checkstr(elf_Runtime *c, llocalid x);


lapi llocalid elf_stkput(elf_Runtime *R, int n);
lapi llocalid elf_stklen(elf_Runtime *c);

lapi llocalid elf_putval(elf_Runtime *, elf_val v);
lapi void elf_putnil(elf_Runtime *);
lapi void elf_putint(elf_Runtime *, elf_int i);
lapi void elf_putnum(elf_Runtime *, elf_num n);
lapi void elf_putbinding(elf_Runtime *, lBinding c);
lapi void elf_putsys(elf_Runtime *c, elf_Handle h);
lapi void elf_puttab(elf_Runtime *, elf_Table *t);
lapi llocalid elf_putcl(elf_Runtime *, elf_Closure *f);
lapi void elf_putstr(elf_Runtime *, elf_String *s);


lapi elf_Table *elf_pushnewtab(elf_Runtime *);
lapi elf_String *elf_pushnewstr(elf_Runtime *, char const *c);
lapi llocalid elf_pushnewcl(elf_Runtime *, elf_Proto fn);


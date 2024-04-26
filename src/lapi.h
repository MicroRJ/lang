/*
** See Copyright Notice In elf.h
** lapi.h
** Main user API
*/


lapi elf_Object *elf_getthis(lRuntime *R);

lapi elf_val elf_getval(lRuntime *R, llocalid x);
lapi elf_int elf_getint(lRuntime *R, llocalid x);
lapi elf_num elf_getnum(lRuntime *R, llocalid x);
lapi elf_String *elf_getstr(lRuntime *R, llocalid x);
lapi elf_Object *elf_getobj(lRuntime *R, llocalid x);
lapi elf_tab *elf_gettab(lRuntime *R, llocalid x);
lapi elf_Handle elf_getsys(lRuntime *R, llocalid x);
lapi elf_Closure *elf_getcls(lRuntime *R, llocalid x);


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
int elf_loadexpr(lRuntime *, elf_String *contents, llocalid x, llocalid y);


/*
** Loads a file and calls its function,
** returns the number of results.
*/
int elf_loadfile(lRuntime *, elf_FileState *fs, elf_String *filename, llocalid rx, int ny);


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
lapi int elf_callfn(lRuntime *, elf_Object *obj, llocalid rx, llocalid ry, int nx, int ny);


/*
** Performs a root call, where rx and ry are the same
** and obj is nil.
*/
lapi int elf_rootcall(lRuntime *, llocalid rx, int nx, int ny);


lapi int elf_run(lRuntime *);


lapi void elf_checkcl(lRuntime *c, llocalid x);
lapi elf_String *elf_checkstr(lRuntime *c, llocalid x);


lapi llocalid elf_stkput(lRuntime *R, int n);
lapi llocalid elf_stklen(lRuntime *c);

lapi llocalid elf_putval(lRuntime *, elf_val v);
lapi void elf_putnil(lRuntime *);
lapi void elf_putint(lRuntime *, elf_int i);
lapi void elf_putnum(lRuntime *, elf_num n);
lapi void elf_putbinding(lRuntime *, lBinding c);
lapi void elf_putsys(lRuntime *c, elf_Handle h);
lapi void elf_puttab(lRuntime *, elf_tab *t);
lapi llocalid elf_putcl(lRuntime *, elf_Closure *f);
lapi void elf_putstr(lRuntime *, elf_String *s);


lapi elf_tab *elf_pushnewtab(lRuntime *);
lapi elf_String *elf_pushnewstr(lRuntime *, char const *c);
lapi llocalid elf_pushnewcl(lRuntime *, elf_Proto fn);


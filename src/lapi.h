/*
** See Copyright Notice In lang.h
** lapi.h
** Main user API
*/


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
lapi int lang_call(lRuntime *, lObject *obj, llocalid rx, llocalid ry, int nx, int ny);


/*
** Performs a root call, where rx and ry are the same
** and obj is nil.
*/
lapi int lang_rootcall(lRuntime *, llocalid rx, int nx, int ny);

lapi llongint lang_toplen(lRuntime *c);

lapi int lang_resume(lRuntime *);
int lang_loadexpr(lRuntime *, lString *contents, llocalid x, llocalid y);
int lang_loadfile(lRuntime *, FileState *fs, lString *filename, llocalid x, int y);

lapi llongint lang_poplong(lRuntime *c);

lapi void lang_checkcl(lRuntime *c, llocalid x);
lapi lString *lang_checkString(lRuntime *c, llocalid x);

lapi lValue lang_load(lRuntime *c, llocalid x);
lapi lsysobj lang_getsysobj(lRuntime *c, llocalid x);
lapi lClosure *lang_loadcl(lRuntime *c, llocalid x);
lapi llongint lang_getlong(lRuntime *c, llocalid x);
lapi lnumber lang_getnum(lRuntime *c, llocalid x);
lapi lString *lang_getstr(lRuntime *c, llocalid x);
llocalid lang_stkalloc(lRuntime *R, int n);

lapi llocalid lang_pushvalue(lRuntime *, lValue v);
lapi void lang_pushnil(lRuntime *);
lapi void lang_pushlong(lRuntime *, llongint i);
lapi void lang_pushnum(lRuntime *, lnumber n);
lapi void lang_pushbinding(lRuntime *, lBinding c);
lapi void lang_pushsysobj(lRuntime *c, lsysobj h);
lapi void lang_pushtable(lRuntime *, lTable *t);
lapi llocalid lang_pushclosure(lRuntime *, lClosure *f);
lapi void lang_pushString(lRuntime *, lString *s);


lapi lTable *lang_pushnewtable(lRuntime *);
lapi lString *lang_pushnewS(lRuntime *, char const *c);
lapi llocalid lang_pushnewclosure(lRuntime *, lProto fn);


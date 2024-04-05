/*
** See Copyright Notice In lang.h
** lapi.h
** Main user API
*/


lapi llongint lang_leftover(lRuntime *c);

lapi int lang_resume(lRuntime *);
lapi int lang_call(lRuntime *, lObject *obj, llocalid r, int x, int y);
lapi int lang_callargs(lRuntime *, lClosure *cl, int n, int y, ...);


lapi llongint lang_poplong(lRuntime *c);

lapi void lang_checkcl(lRuntime *c, llocalid x);

lapi lValue lang_load(lRuntime *c, llocalid x);
lapi Handle lang_loadhandle(lRuntime *c, llocalid x);
lapi lClosure *lang_loadcl(lRuntime *c, llocalid x);
lapi llongint lang_loadlong(lRuntime *c, llocalid x);
lapi lnumber lang_loadnum(lRuntime *c, llocalid x);
lapi lString *lang_loadS(lRuntime *c, llocalid x);

lapi void lang_pushvalue(lRuntime *, lValue v);
lapi void lang_pushnil(lRuntime *);
lapi void lang_pushlong(lRuntime *, llongint i);
lapi void lang_pushnum(lRuntime *, lnumber n);
lapi void lang_pushbinding(lRuntime *, lBinding c);
lapi void lang_pushhandle(lRuntime *c, Handle h);
lapi void lang_pushtable(lRuntime *, Table *t);
lapi void lang_pushclosure(lRuntime *, lClosure *f);
lapi void lang_pushString(lRuntime *, lString *s);


lapi Table *lang_pushnewtable(lRuntime *);
lapi lString *lang_pushnewS(lRuntime *, char const *c);
lapi lClosure *lang_pushnewcl(lRuntime *, lProto fn);


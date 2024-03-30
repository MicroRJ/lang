/*
** See Copyright Notice In lang.h
** lapi.h
** Main user API
*/


lapi llong lang_leftover(Runtime *c);

lapi int lang_exec(Runtime *);
lapi int lang_call(Runtime *, lObject *obj, lClosure *, int x, int y);
lapi int lang_bind(Runtime *, lObject *obj, lBinding, int x, int y);
lapi int lang_callargs(Runtime *, lClosure *cl, int n, int y, ...);


lapi llong lang_poplong(Runtime *c);

lapi lValue lang_load(Runtime *c, llocalid x);
lapi Handle lang_loadhandle(Runtime *c, llocalid x);
lapi lClosure *lang_loadcl(Runtime *c, llocalid x);
lapi llong lang_loadlong(Runtime *c, llocalid x);
lapi lnumber lang_loadnum(Runtime *c, llocalid x);
lapi lString *lang_loadS(Runtime *c, llocalid x);

lapi void lang_pushvalue(Runtime *, lValue v);
lapi void lang_pushnil(Runtime *);
lapi void lang_pushlong(Runtime *, llong i);
lapi void lang_pushnum(Runtime *, lnumber n);
lapi void lang_pushbinding(Runtime *, lBinding c);
lapi void lang_pushhandle(Runtime *c, Handle h);
lapi void lang_pushtable(Runtime *, Table *t);
lapi void lang_pushclosure(Runtime *, lClosure *f);
lapi void lang_pushString(Runtime *, lString *s);


lapi Table *lang_pushnewtable(Runtime *);
lapi lString *lang_pushnewS(Runtime *, char const *c);
lapi lClosure *lang_pushnewcl(Runtime *, Proto fn);


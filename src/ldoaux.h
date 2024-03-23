/*
** See Copyright Notice In lang.h
** ldoaux.h
** Runtime API
*/


LAPI int langR_resume(Runtime *c);

LAPI int langR_callfunc(Runtime *c, Closure *, int n);
LAPI int langR_callCfunc(Runtime *c, CFunc , int n);

LAPI Integer langR_stacksize(Runtime *c);

LAPI Integer langR_popI(Runtime *c);

LAPI Value langR_loadV(Runtime *c, StackId x);
LAPI String *langR_loadS(Runtime *c, StackId x);
LAPI Closure *langR_loadF(Runtime *c, StackId x);
LAPI Integer langR_loadI(Runtime *c, StackId x);

LAPI void langR_pushV(Runtime *c, Value v);
LAPI void langR_pushNil(Runtime *c);
LAPI void langR_pushI(Runtime *c, Integer i);
LAPI void langR_pushN(Runtime *c, Number n);
LAPI void langR_pushS(Runtime *c, String *s);
LAPI void langR_pushF(Runtime *c, Closure *f);
LAPI void langR_pushH(Runtime *c, Table *t);

LAPI Table *langR_pushnewR(Runtime *c);
LAPI String *langR_pushnewS(Runtime *c, char const *contents);
LAPI Closure *langR_pushnewF(Runtime *c, Proto fn);



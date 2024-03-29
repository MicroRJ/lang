/*
** See Copyright Notice In lang.h
** lruntime.h
** Runtime Structures
*/


typedef struct CallFrame {
	/* closure this call frame belongs to */
	Closure *cl;
	/* object for meta functions */
	Object *obj;
	/* pointer to base stack address, which is
	also the regress address. when the stack
	frame is popped the stack pointer is set
	to l, and the return values should before l. */
	Value *l;
	/* next instruction index, not really
	used now, but I guess for coroutines? */
	llong j;
	/* the number of inputs (x) in and
	the number of expected outputs (y).
	A function or binding can yield many more
	or less values.
	For bindings you must return the number
	of actual values yielded so that the runtime
	either generates the extra values
	or discards the excess.  */
	int x,y;
} CallFrame;


typedef struct Runtime {
	Module *md;
	Value *s,*v;
	llocalid z;
	CallFrame *f;
	Object **gc;
	llong gcthreshold;
	lbool isgcpaused;
	lbool logging;
} Runtime;


lapi llong lang_leftover(Runtime *c);

lapi int lang_exec(Runtime *);
lapi int lang_call(Runtime *, Object *obj, Closure *, int x, int y);
lapi int lang_bind(Runtime *, Object *obj, Binding, int x, int y);
lapi int lang_callargs(Runtime *, Closure *cl, int n, int y, ...);


lapi llong lang_poplong(Runtime *c);

lapi Value lang_load(Runtime *c, llocalid x);
lapi Handle lang_loadhandle(Runtime *c, llocalid x);
lapi Closure *lang_loadcl(Runtime *c, llocalid x);
lapi llong lang_loadlong(Runtime *c, llocalid x);
lapi lnumber lang_loadnum(Runtime *c, llocalid x);
lapi String *lang_loadS(Runtime *c, llocalid x);

lapi void lang_pushvalue(Runtime *, Value v);
lapi void lang_pushnil(Runtime *);
lapi void lang_pushlong(Runtime *, llong i);
lapi void lang_pushnum(Runtime *, lnumber n);
lapi void lang_pushbinding(Runtime *, Binding c);
lapi void lang_pushhandle(Runtime *c, Handle h);
lapi void lang_pushtable(Runtime *, Table *t);
lapi void lang_pushclosure(Runtime *, Closure *f);
lapi void lang_pushString(Runtime *, String *s);


lapi Table *lang_pushnewtable(Runtime *);
lapi String *lang_pushnewS(Runtime *, char const *c);
lapi Closure *lang_pushnewcl(Runtime *, Proto fn);


lnumber ltoreal(Value v) {
	if (v.tag == VALUE_LONG) {
		return (lnumber) v.i;
	}
	return v.n;
}


llong ltolong(Value v) {
	if (v.tag == VALUE_REAL) {
		return (llong) v.n;
	}
	return v.i;
}
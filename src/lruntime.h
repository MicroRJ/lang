/*
** See Copyright Notice In lang.h
** lruntime.h
** Runtime Structures
*/


typedef struct CallFrame {
	/* closure this call frame belongs to */
	lClosure *cl;
	/* object for meta functions */
	lObject *obj;
	/* pointer to base stack address, which is
	also the regress address. when the stack
	frame is popped the stack pointer is set
	to l, and the return values should before l. */
	lValue *l;
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
	lValue *s,*v;
	llocalid z;
	CallFrame *f;
	lObject **gc;
	llong gcthreshold;
	lbool isgcpaused;
	lbool logging;
} Runtime;

lnumber ltoreal(lValue v) {
	if (v.tag == VALUE_LONG) {
		return (lnumber) v.i;
	}
	return v.n;
}


llong ltolong(lValue v) {
	if (v.tag == VALUE_REAL) {
		return (llong) v.n;
	}
	return v.i;
}
/*
** See Copyright Notice In lang.h
** lruntime.h
** Runtime Structures
*/




typedef struct ldelaylist ldelaylist;
typedef struct ldelaylist {
	ldelaylist *n;
	lbyteid j;
} ldelaylist;


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


	/* byte address to jump to once all delayed
	blocks have been executed */
	lbyteid dj;
	/* list of delayed blocks to be executed
	on return. */
	ldelaylist *dl;
} CallFrame;


typedef struct Runtime {
	lModule *md;
	lValue *s,*v;
	llocalid z;
	CallFrame *f;
	lObject **gc;
	llong gcthreshold;
	lbool isgcpaused;
	lbool logging;
} Runtime;



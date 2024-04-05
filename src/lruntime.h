/*
** See Copyright Notice In lang.h
** lruntime.h
** lRuntime Structures
*/


typedef struct ldelaylist ldelaylist;
typedef struct ldelaylist {
	ldelaylist *n;
	lbyteid j;
} ldelaylist;


/* -- pretty much a call frame */
typedef struct lContext {
	/* -- closure this call frame belongs to */
	lClosure *cl;
	/* -- object for meta functions and table calls */
	lObject *obj;
	/* -- pointer to base stack address, 'rbp', yields
	- to here at -1. should have base[-1..y) registers
	- to write to. */
	union {
		lValue *base,*l;
	};
	/* -- regress address */
	lValue *top;
	/* -- next instruction index, not really
	- used now, but I guess for coroutines? */
	llongint j;
	/* -- The number of inputs (x) and
	- the number of expected outputs (y).
	- Output registers are allocated by the
	- caller and runtime writes to them when
	- the callee \yields.
	- A function or binding can yield many more
	- or less values, it does not matter.
	- For bindings you must return the number
	- of actual values yielded so that runtime
	- can hoist the return values. */
	int x,y;
	/* -- list of delayed jumps to be executed
	- on return, 'finally' statements produce
	- these. */
	ldelaylist *dl;
} lContext;


typedef struct lRuntime {
	lModule *md;
	union {
		lValue *stk,*s;
	};
	llocalid stklen;
	union {
		lValue *top,*v;
	};
	union {
		lContext *frame,*f;
	};
	lObject **gc;
	llongint gcthreshold;
	lbool isgcpaused;
	lbool logging;
} lRuntime;



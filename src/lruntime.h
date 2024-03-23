/*
** See Copyright Notice In lang.h
** lruntime.h
** Runtime Structures
*/


typedef int StackId;


typedef struct CallFrame {
	/* closure this call frame belongs to */
	Closure *cl;
	/* object for meta functions */
	Object *obj;
	/* pointer to base stack address, which is
	also the regress address. when the stack
	frame is popped the stack pointer is set
	to l, and the return values should be after l. */
	Value *l;
	/* next instruction index, not really
	used now, but I guess for coroutines? */
	Integer j;
	/* the number of functions passed in,
	for meaningful for c functions */
	int n;
} CallFrame;


typedef struct Runtime {
	Module    *md;
	Value   *s,*v;
	StackId     z;
	CallFrame  *f;
	Object   **gc;
	Integer 	  gcthreshold;
	Bool 		isgcpaused;
} Runtime;
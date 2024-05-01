/*
** See Copyright Notice In elf.h
** elf-run.h
** Runtime
*/


typedef struct ldelaylist ldelaylist;
typedef struct ldelaylist {
	ldelaylist *n;
	lbyteid j;
} ldelaylist;


typedef struct elf_CallFrame elf_CallFrame;


typedef struct elf_CallFrame {
	/* todo: this is only here so that we can write
	to caller->locals[rx/ry] directly, rx and ry
	could be relative to this.locals */
	elf_CallFrame *caller;
	/* the closure this call frame belongs to */
	elf_Closure *cl;
	/* the object for meta fields, meta calls and the likes */
	elf_Object *obj;
	/* pointer to base stack address, the callee should
	yield starting at base[-1], should have base[-1..y)
	registers to write to. */
	union {
		elf_val *base,*l,*locals;
	};
	/* todo: rename top to regress */
	elf_val *top;
	/* -- next instruction index, not really
	- used now, but I guess for coroutines? */
	elf_int j;
	llocalid rx,ry;
	/* x and y names are deprecated */
	/* -- The number of inputs (nx) and
	- the number of expected outputs (ny).
	- Output registers are allocated by the
	- caller and runtime writes to them when
	- the callee \yields.
	- A function or binding can yield many more
	- or less values, it does not matter.
	- For bindings you must return the number
	- of actual values yielded so that runtime
	- can hoist the return values. */
	union { int nx,x; };
	union { int ny,y; };
	/* -- list of delayed jumps to be executed
	- on return, 'finally' statements produce
	- these. */
	ldelaylist *dl;

	elf_bool logging;
} elf_CallFrame;


typedef struct lThread {
	union { elf_Runtime *R, *rt; };
	union { elf_Module  *M, *md; };
	union { elf_CallFrame *call; };
	union { elf_val *stk;      };
	llocalid stklen;
	elf_int threadid;
	lbyteid  curbyte;
} lThread;

typedef struct elf_Runtime {
	union { elf_Module *M, *md; };
	union { elf_val *stk,*s; };
	llocalid stklen;
	union { elf_val *top,*v; };
	union { elf_CallFrame *call,*frame,*f; };
	elf_bool debugbreak;
	elf_Table *metatable_str;
	elf_Table *metatable_tab;
	struct {
		elf_String *__add,*__sub,*__mul,*__div;
		elf_String *__add1,*__sub1,*__mul1,*__div1;
	} cache;
	/* current byte */
	elf_int j;
	elf_bool logging;
	elf_Object **gc;
	elf_bool     gcflags;
	elf_int      gcmemory;
	elf_int      gcthreshold;
} elf_Runtime;



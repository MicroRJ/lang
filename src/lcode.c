/*
** See Copyright Notice In lang.h
** (L) lcode.c
** Bytecode Generator (tree -> bytecode)
*/


lbyteop treetobyte(ltreetype tt);


/* todo: deprecated */
lbyteid langL_getlabel(FileState *fs) {
	return fs->md->nbytes;
}


llocalid langL_localalloc(FileState *fs, llocalid n) {
	LASSERT(n > NO_SLOT);

	FileFunc *fn = fs->fn;

	llocalid id = fn->xlocals;
	fn->xlocals += n;

	if (fn->nlocals < fn->xlocals) {
		fn->nlocals = fn->xlocals;
	}
	return id;
}


void langL_localdealloc(FileState *fs, llocalid x) {
	LASSERT(x > NO_SLOT);

	FileFunc *fn = fs->fn;

	LASSERT(x == fn->xlocals-1);
	fn->xlocals = x;
}


lbyteid langL_addbyte(FileState *fs, llineid line, lBytecode byte) {
	FileFunc *fn = fs->fn;
	lModule *md = fs->md;
	// bytefpf(md,stdout,md->nbytes-fn->bytes,byte);
	langA_varadd(md->lines,line);
	langA_varadd(md->bytes,byte);
	return md->nbytes ++;
}


lbyteid langL_byte(FileState *fs, llineid line, lbyteop k, llongint i) {
	return langL_addbyte(fs,line,(lBytecode){k,i});
}


lbyteid langL_bytexy(FileState *fs, llineid line, lbyteop k, int x, int y) {
	lBytecode b = (lBytecode){k};
	b.x = x;
	b.y = y;
	return langL_addbyte(fs,line,b);
}


lbyteid langL_bytexyz(FileState *fs, llineid line, lbyteop k, int x, int y, int z) {
	lBytecode b = (lBytecode){k};
	b.x = x;
	b.y = y;
	b.z = z;
	return langL_addbyte(fs,line,b);
}


void langL_tieloosejto(FileState *fs, lbyteid id, lbyteid j) {
	lModule *md = fs->md;
	lBytecode *bytes = md->bytes;
	lBytecode b = bytes[id];

	lbyteid l = j - id;
	// if (l == NO_JUMP) {
	// 	langX_error(fs,md->lines[id],"opt, no jump");
	// }
	switch (b.k) {
		case BC_J:
		case BC_DELAY: {
			bytes[id].i = l;
		} break;
		case BC_JZ:
		case BC_JNZ:
		case BC_YIELD: {
			bytes[id].x = l;
		} break;
		default: LNOBRANCH;
	}
}


void langL_tieloosej(FileState *fs, lbyteid i) {
	langL_tieloosejto(fs,i,langL_getlabel(fs));
}


void langL_tieloosejs(FileState *fs, lbyteid *js) {
	langA_varifor(js) {
		langL_tieloosej(fs,js[i]);
	}
}


void langL_tieloosejsto(FileState *fs, lbyteid *js, lbyteid j) {
	langA_varifor(js) {
		langL_tieloosejto(fs,js[i],j);
	}
}


lbyteid langL_jump(FileState *fs, llineid line, lbyteid j) {
	return langL_byte(fs,line,BC_J,j-fs->md->nbytes);
}


void langL_fnepiloge(FileState *fs, llineid line) {
	lModule *md = fs->md;
	FileFunc *fn = fs->fn;

	/* todo: this is unncessary, we know were
	the return instruction is at. */
	langL_tieloosejs(fs,fn->yj);
	langA_vardel(fn->yj);
	fn->yj = lnil;

	/* finally, return control flow */
	langL_byte(fs,line,BC_LEAVE,0);
}


/* --todo: adapt this comment:
if we got here it means that the expression is not naturally
a short-circuit and has to be made into one, in which case we
default to evaluating a boolean expression and adding boolean
jumps (unless the context calls for a boolean expression).
a boolean expression is defined as an expression which yields
either true or false, in which case we need a register to
put that expression in there and later call either jz or jnz
on it, unless we want a boolean expression, in which case
we can write directly to x.
for instance, `x = b == c`, in this case, there's no need for
short circuiting, and in many cases there's the contrary,
no need to generate boolean expressions, for instance
`if b == c`, this is only true if all the expressions can
be short-circuited, which is true for all expressions because
all boolean expressions can be made into short-curcuits by
adding boolean jumps, this is only beneficial if the context
calls for it, such as 'and' and 'or' in an if statement.
if the register x is NO_SLOT, it means the context calls for
short circuting, in otherwords, it is more beneficial to
avoid generating boolean expressions, otherwise we can
directly evaluate a boolean expression into register x,
of course, this only pays of if the expression is entirely
boolean, because if not all branches lead to some boolean
output, we have to synthesize additional ones, we can check
whether the expression is entirely boolean or not by checking
the true and false jump lists, if both of them are nil it
means the expressions is entirely boolean, and there's no
need to turn it into one. */




/*
** Evaluates the given boolean or logical expression using short
** circuit evaluation, creating branches as it evaluates each
** expression, only one register is necessary, if no registers
** are given one is allocated.
*/
lbyteid langL_branchif(FileState *fs, ljlist *js, lbool z, llocalid x, ltreeid id) {
	Tree v = fs->nodes[id];
	lbyteid j = NO_BYTE;

	/* allocate temporary register here before we split, this will
	make is so that we only allocate one temporary register for the
	whole evaluation, recursive calls to this function won't keep
	allocating registers, this works because of short circuiting as
	expressible in the bytecode. */
	llocalid xx = x;
	if (xx == NO_SLOT) x = langL_localalloc(fs,1);

	switch (v.k) {
		case Y_LOG_AND: {
			langL_jumpiffalse(fs,js,x,v.x);
			j = langL_branchif(fs,js,z,x,v.y);
		} break;
		case Y_LOG_OR: {
			langL_jumpiftrue(fs,js,x,v.x);
			j = langL_branchif(fs,js,z,x,v.y);
		} break;
		default: {
			langL_load(fs,NO_LINE,lfalse,x,1,id);

			if (z != 0) {
				j = langL_bytexy(fs,v.line,BC_JNZ,NO_JUMP,x);
				langA_varadd(js->t,j);
			} else {
				j = langL_bytexy(fs,v.line,BC_JZ,NO_JUMP,x);
				langA_varadd(js->f,j);
			}
		} break;
	}

	if (xx == NO_SLOT) langL_localdealloc(fs,x);
	return j;
}


lbyteid langL_branchiffalse(FileState *fs, ljlist *js, llocalid x, ltreeid id) {
	return langL_branchif(fs,js,lfalse,x,id);
}


lbyteid langL_branchiftrue(FileState *fs, ljlist *js, llocalid x, ltreeid id) {
	return langL_branchif(fs,js,ltrue,x,id);
}


/*
** Similar to branch if true, but additionally this byte
** address becomes the false target, thus all false branches
** converge here. all true branches are returned.
*/
lbyteid *langL_jumpiftrue(FileState *fs, ljlist *js, llocalid x, ltreeid id) {
	langL_branchiftrue(fs,js,x,id);
	langL_tieloosejs(fs,js->f);
	langA_vardel(js->f);
	js->f = lnil;
	return js->t;
}


lbyteid *langL_jumpiffalse(FileState *fs, ljlist *js, llocalid x, ltreeid id) {
	langL_branchiffalse(fs,js,x,id);
	langL_tieloosejs(fs,js->t);
	langA_vardel(js->t);
	js->t = lnil;
	return js->f;
}



/* todo: add support for multiple results */
void langL_yield(FileState *fs, llineid line, ltreeid t) {
	int n = 0;
	llocalid x = 0;
	if (t != NO_TREE) {
		/* todo: determine the number of values in
		tree, and allocate that many registers?
		what if we return a function that returns
		multiple values? */
		x = langL_localalloc(fs,n=1);
		langL_load(fs,line,ltrue,x,n,t);
		langL_localdealloc(fs,x);
	}
	if (fs->fn->nyield < n) {
		fs->fn->nyield = n;
	}
	lbyteid j = langL_bytexyz(fs,line,BC_YIELD,NO_JUMP,x,n);
	langA_varadd(fs->fn->yj,j);
}


/*
** allocates and loads all one value for each tree
** in z contiguously, returns number of values in z.
** y is preallocated space that can be used to put
** z values in, z values are then deallocated.
*/
int langL_localloadadj(FileState *fs, llocalid y, ltreeid *z) {
	int n = langA_varlen(z);
	int x = n - y;
	if (x < 0) x = 0;
	llocalid id = langL_localalloc(fs,x) - y;
	langA_varifor(z)
	/**/langL_load(fs,NO_LINE,ltrue,id+i,1,z[i]);
	fs->fn->xlocals = id + y;
	return n;
}


llocalid langL_localize(FileState *fs, llineid line, ltreeid id) {
	llocalid x = fs->nodes[id].x;
	if (fs->nodes[id].k != Y_LOCAL) {
		x = langL_localalloc(fs,1);
		langL_load(fs,line,lfalse,x,1,id);
	}
	return x;
}


/*
** x is preallocated stack slot to put the result
** in, y the number of slots allocated after x.
** if y is NO_SLOT the instruction is ommitted unless
** it has side-effects, such as a function call,
** but no registers are allocated for it.
** if reload is specified, when the value is already
** on the stack, then it is reloaded onto x.
*/
void langL_load(FileState *fs, llineid line, lbool reload, llocalid x, llocalid y, ltreeid id) {
	/* -- Some instructions have no side effects,
	- in which case, if the yield count is 0, the
	- instruction is not emitted */
	LASSERT(x > NO_SLOT);
	Tree v = fs->nodes[id];
	LASSERT(v.level <= fs->level);
	if (line == 0) line = v.line;

	lModule *md = fs->md;


	switch (v.k) {
		case Y_LOCAL: {
			if (y == 0) goto leave;
			if (reload) {
				langL_bytexy(fs,line,BC_RELOAD,x,v.x);
			}
		} break;
		case Y_GLOBAL: {
			if (y == 0) goto leave;
			langL_bytexy(fs,line,BC_LOADGLOBAL,x,v.x);
		} break;
		case Y_CACHED: {
			if (y == 0) goto leave;
			langL_bytexy(fs,line,BC_LOADCACHED,x,v.x);
		} break;
		case Y_NIL: {
			if (y == 0) goto leave;
			/* -- todo: coalesce */
			langL_bytexy(fs,line,BC_LOADNIL,x,y);
		} break;
		case Y_INTEGER: {
			if (y == 0) goto leave;
			langL_bytexy(fs,line,BC_LOADINT,x,v.lit.i);
		} break;
		case Y_NUMBER: {
			if (y == 0) goto leave;
			langL_bytexy(fs,line,BC_LOADNUM,x,v.lit.i);
		} break;
		case Y_STRING: {
			if (y == 0) goto leave;
			/* -- todo: allocate this in constant pool */
			int g = lang_addglobal(fs->md,0,lang_S(langS_new(fs->rt,v.lit.s)));

			langL_bytexy(fs,line,BC_LOADGLOBAL,x,g);
		} break;
		case Y_FUNCTION: {
			if (y == 0) goto leave;
			llocalid n = langL_localloadadj(fs,y,v.z);
			langL_bytexyz(fs,line,BC_CLOSURE,x,v.x,n);
		} break;
		case Y_TABLE: {
			LASSERT(v.line != 0);
			/* -- todo: make this faster? */
			langL_bytexy(fs,line,BC_TABLE,x,0);
			langA_varifor(v.z) {
				(void) i;
				LNOBRANCH;
				#if 0
				ltreeid z = v.z[i];
				LASSERT(z != x);
				Tree q = fs->nodes[z];

				/* todo: this is temporary */
				if (q.k == Y_DESIG) {
					langL_load2any(fs,q.line,q.x,1);
					langL_load2any(fs,q.line,q.y,1);
					langL_bytexy(fs,q.line,BC_SETFIELD,0);
				} else {
					langL_byte(fs,q.line,BC_INT,i);
					langL_localload(fs,q.line,z,1);
					langL_byte(fs,q.line,BC_SETINDEX,0);
				}
				#endif
			}
		} break;
		case Y_FIELD: {
			llocalid mem = fs->fn->xlocals;
			llocalid xx = langL_localize(fs,line,v.x);
			llocalid yy = langL_localize(fs,line,v.y);
			langL_bytexyz(fs,line,BC_FIELD,x,xx,yy);
			fs->fn->xlocals = mem;
		} break;
		case Y_INDEX: {
			LNOBRANCH;
			// langL_load2any(fs,NO_LINE,v.x,1);
			// langL_load2any(fs,NO_LINE,v.y,1);
			// langL_byte(fs,line,BC_INDEX,0);
		} break;
		case Y_GROUP: {
			langL_load(fs,line,reload,x,y,v.x);
		} break;
		/* -- todo: this is silly, probably just
		- do this through a lib or something */
		case Y_BUILTIN: {
			int n = langL_localloadadj(fs,y,v.z);
			if (v.x == TK_STKLEN) {
				langL_bytexy(fs,line,x,BC_STKLEN,MAX(n,y));
			} else
			if (v.x == TK_STKGET) {
				langL_bytexy(fs,line,x,BC_STKGET,MAX(n,y));
			} else LNOBRANCH;
		} break;
		case Y_LOADFILE: {
			LNOBRANCH;
			// langL_load(fs,line,x,v.x,1);
			// langL_bytexy(fs,line,BC_LOADFILE,x,y);
		} break;
		case Y_MCALL: {
			LNOBRANCH;
			// int n = langA_varlen(v.z);
			// llocalid id = langL_localalloc(n);
			// langA_varifor(v.z) {
			// 	langL_load2any(fs,NO_LINE,id,v.z[i]);
			// }
			// langL_localalloc(-id);
			// langL_load2any(fs,NO_LINE,v.x,1);
			// langL_load2any(fs,NO_LINE,v.y,1);
			// langL_bytexyz(fs,line,BC_METACALL,x,n,v.x,v.y);
		} break;
		case Y_CALL: {
			langL_load(fs,line,ltrue,x,1,v.x);
			int n = langL_localloadadj(fs,0,v.z);
			langL_bytexyz(fs,line,BC_CALL,x,n,y);
		} break;
		case Y_LOG_AND: case Y_LOG_OR: {
			/* todo: we need to optimize this, better support for
			boolean expressions */
			ljlist js = {0};
			langL_branchiffalse(fs,&js,x,id);
			langL_tieloosejs(fs,js.t);
			langA_vardel(js.t);
			js.t = lnil;
			langL_bytexy(fs,line,BC_LOADINT,x,ltrue);
			int j = langL_jump(fs,line,-1);
			langL_tieloosejs(fs,js.f);
			langA_vardel(js.f);
			js.f = lnil;
			langL_bytexy(fs,line,BC_LOADINT,x,lfalse);
			langL_tieloosej(fs,j);

		} break;
		case Y_NEQ: case Y_EQ:
		case Y_GT: case Y_LT: case Y_GTEQ: case Y_LTEQ:
		case Y_DIV: case Y_MUL: case Y_MOD:
		case Y_SUB: case Y_ADD:
		case Y_BSHL: case Y_BSHR: case Y_BXOR: {
			/* save memory state */
			llocalid mem = fs->fn->xlocals;
			if ((v.k == Y_GT) || (v.k == Y_GTEQ)) {
				llocalid xx = langL_localize(fs,line,v.y);
				llocalid yy = langL_localize(fs,line,v.x);
				/* -- see lbyte.h for (v.k^1) */
				langL_bytexyz(fs,line,treetobyte(v.k^1),x,xx,yy);
			} else
			if (v.k == Y_EQ) {
				if (fs->nodes[v.y].k == Y_NIL) {
					llocalid xx = langL_localize(fs,line,x);
					langL_bytexy(fs,line,BC_ISNIL,x,xx);
				} else goto _join;
			} else { _join:
				llocalid xx = langL_localize(fs,line,v.x);
				llocalid yy = langL_localize(fs,line,v.y);
				langL_bytexyz(fs,line,treetobyte(v.k),x,xx,yy);
			}
			fs->fn->xlocals = mem;
		} break;
		default: {
			langX_error(fs,v.line,"unsupported node");
			LNOBRANCH;
		} break;
	}

	leave:;
}


void langL_loadinto(FileState *fs, llineid line, ltreeid x, ltreeid y) {
	Tree v = fs->nodes[x];
	LASSERT(v.level <= fs->level);
	LASSERT(x >= 0);
	LASSERT(y >= 0);
	if (line == 0) line = v.line;

	switch (v.k) {
		case Y_GLOBAL: {
			llocalid r = langL_localalloc(fs,1);
			langL_load(fs,line,lfalse,r,y,1);
			langL_localdealloc(fs,r);
			langL_bytexy(fs,line,BC_SETGLOBAL,v.x,r);
		} break;
		case Y_LOCAL: {
			/* -- todo: account for x = x opt ?  */
			langL_load(fs,line,ltrue,v.x,1,y);
		} break;
		case Y_INDEX: case Y_FIELD: {
			llocalid mem = fs->fn->xlocals;
			llocalid xxx = langL_localize(fs,line,v.x);
			llocalid yyy = langL_localize(fs,line,v.y);
			llocalid yy = langL_localize(fs,line,y);
			langL_bytexyz(fs,line
			, v.k == Y_INDEX ? BC_SETINDEX : BC_SETFIELD,xxx,yyy,yy);
			fs->fn->xlocals = mem;
		} break;
		// {x}[{x}..{x}] = {y}
		case Y_RANGE_INDEX: {
			LNOBRANCH;
			// Loop l = {0};
			// langL_load2any(fs,line,v.x,1);/* table */
			// langL_beginrangedloop(fs,line,&l,-1,v.y);
			// langL_load2any(fs,line,l.x,1);/* index */
			// langL_load2any(fs,line,y,1);/* value */
			// langL_byte(fs,line,BC_SETINDEX,0);
			// langL_closerangedloop(fs,line,&l);
		} break;
		default: {
			LNOBRANCH;
		} break;
	}
}


void langL_beginif(FileState *fs, llineid line, Select *s, ltreeid x, int z) {
	ljlist js = {0};
	langL_branchif(fs,&js,z,NO_SLOT,x);
	// if  0 = jz
	// iff 1 = jnz
	if (z == L_IF) {
		LASSERT(js.f != 0);
		langL_tieloosejs(fs,js.t);
		langA_vardel(js.t);
		js.t = 0;
		s->jz = js.f;
	} else {
		LASSERT(js.t != 0);
		langL_tieloosejs(fs,js.f);
		langA_vardel(js.f);
		js.f = 0;
		s->jz = js.t;
	}
}


/*
** Closes previous conditional block by emitting
** escape jump, patches previous jz (jump if false)
** list to enter this block.
*/
void langL_addelse(FileState *fs, llineid line, Select *s) {
	LASSERT(s->jz != 0);
	int j = langL_jump(fs,line,-1);
	langA_varadd(s->j,j);

	langL_tieloosejs(fs,s->jz);
	langA_vardel(s->jz);
	s->jz = lnil;
}


void langL_addelif(FileState *fs, llineid line, Select *s, int x) {
	langL_addelse(fs,line,s);
	langL_beginif(fs,line,s,x,L_IF);
}


void langL_addthen(FileState *fs, llineid line, Select *s) {
	/* we don't need to close the previous block, it can just fall
	through to our branch, do collect all the other exit jumps and
	tie them to this branch block, naturally we don't need to add
	an exit jump since else and elif or closeif will terminate
	this block, multiple then blocks are simply chained together
	naturally. */
	langL_tieloosejs(fs,s->j);
	langA_vardel(s->j);
	s->j = lnil;
}


void langL_closeif(FileState *fs, llineid line, Select *s) {
	/* collect missing else branch */
	if (s->jz != lnil) {
		langL_tieloosejs(fs,s->jz);
		langA_vardel(s->jz);
		s->jz = lnil;
	}
	/* collect missing then branch */
	if (s->j != lnil) {
		langL_tieloosejs(fs,s->j);
		langA_vardel(s->j);
		s->j = lnil;
	}
}


void langL_begindowhile(FileState *fs, llineid line, Loop *loop) {
	loop->e = langL_getlabel(fs);
	loop->x = NO_TREE;
	loop->f = lnil;
}


void langL_closedowhile(FileState *fs, llineid line, Loop *loop, ltreeid x) {
	ljlist js = {lnil};
	langL_jumpiftrue(fs,&js,NO_SLOT,x);

	langL_tieloosejsto(fs,js.t,loop->e);
	langA_vardel(js.t);
	js.t = lnil;
}


void langL_beginwhile(FileState *fs, llineid line, Loop *loop, ltreeid x) {
	loop->e = langL_getlabel(fs);
	loop->x = x;
	ljlist js = {lnil};
	loop->f = langL_jumpiffalse(fs,&js,NO_SLOT,x);
}


void langL_closewhile(FileState *fs, llineid line, Loop *loop) {
	langL_jump(fs,line,loop->e);
	langL_tieloosejs(fs,loop->f);
	langA_vardel(loop->f);
	loop->f = lnil;
}


void langL_beginrangedloop(FileState *fs, llineid line, Loop *loop, ltreeid x, ltreeid r) {
	LASSERT(fs->nodes[r].k == Y_RANGE);
	/* todo: */
	if (x == NO_TREE) {
		int v = lang_addglobal(fs->md,langS_new(fs->rt,"$temp"),(lValue){VALUE_NONE});
		x = langY_treeglobal(fs,line,v);
	}

	int lo = fs->nodes[r].x;
	int hi = fs->nodes[r].y;
	/* emit code to initialize x to lower range
	and then compare against upper range, if false
	emit code to jump, the jump is patched when the
	loop is closed */
	langL_loadinto(fs,line,x,lo);

	int c = langY_treexy(fs,NO_LINE,Y_LT,x,hi);

	ljlist js = {lnil};
	loop->f = langL_jumpiffalse(fs,&js,NO_SLOT,c);
	loop->e = langL_getlabel(fs);
	loop->x = x;
}


void langL_closerangedloop(FileState *fs, llineid line, Loop *loop) {
	int x = loop->x;

	int k = langY_treexy(fs,NO_LINE,Y_ADD,x,langY_treelongint(fs,NO_LINE,1));
	langL_loadinto(fs,line,x,k);

	langL_jump(fs,line,loop->e);

	langL_tieloosejs(fs,loop->f);
	langA_vardel(loop->f);
	loop->f = 0;
}


/* -- todo? we could merge with adjacent blocks, but this
would change the order of execution, should leave as is? */
void langL_begindelayedblock(FileState *fs, llineid line, CodeBlock *d) {
	d->j = langL_byte(fs,line,BC_DELAY,NO_JUMP);
}


void langL_closedelayedblock(FileState *fs, llineid line, CodeBlock *bl) {
	// langX_error(fs,line,"closed block, %i",langL_getlocallabel(fs));

	FileFunc *fn = fs->fn;

	langL_byte(fs,line,BC_LEAVE,0);
	langL_tieloosej(fs,bl->j);
}


lbyteop treetobyte(ltreetype tt) {
	switch (tt) {
		case Y_ADD:     return BC_ADD;
		case Y_SUB:     return BC_SUB;
		case Y_DIV:     return BC_DIV;
		case Y_MUL:     return BC_MUL;
		case Y_MOD:     return BC_MOD;
		case Y_NEQ:     return BC_NEQ;
		case Y_EQ:      return BC_EQ;
		case Y_LT:      return BC_LT;
		case Y_LTEQ:    return BC_LTEQ;
		case Y_BSHL:    return BC_SHL;
		case Y_BSHR:    return BC_SHR;
		case Y_BXOR:    return BC_XOR;
	}
	/* for intended use cases, this is an error */
	LNOBRANCH;
	return Y_NONE;
}
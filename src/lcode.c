/*
** See Copyright Notice In lang.h
** (L) lcode.c
** Bytecode Generator (node -> bytecode)
*/


lbyteop treetobyte(lnodeop tt);


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
	if (0) {
		bytefpf(md,stdout,md->nbytes-fn->bytes,byte);
	}

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


/*
** Evaluates the given boolean or logical expression using short
** circuit evaluation, creating branches as it evaluates each
** expression, only one register is necessary, if no registers
** are given one is allocated and deallocated automatically.
*/
lbyteid langL_branchif(FileState *fs, ljlist *js, lbool z, llocalid x, lnodeid id) {
	lNode v = fs->nodes[id];
	lbyteid j = NO_BYTE;

	llocalid mem = fs->fn->xlocals;
	/* if not provided one, allocate temporary register here,
	notice how this is done we keep splitting, this will make
	is so subsequent recursive calls to this function won't keep
	allocating registers, we get away with using only one register
	due to the transitive nature of short circuiting, expressible
	in the bytecode. */
	if (x == NO_SLOT) x = langL_localalloc(fs,1);

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
			langL_localload(fs,NO_LINE,lfalse,x,1,id);

			if (z != 0) {
				j = langL_bytexy(fs,v.line,BC_JNZ,NO_JUMP,x);
				langA_varadd(js->t,j);
			} else {
				j = langL_bytexy(fs,v.line,BC_JZ,NO_JUMP,x);
				langA_varadd(js->f,j);
			}
		} break;
	}


	fs->fn->xlocals = mem;
	return j;
}


lbyteid langL_branchiffalse(FileState *fs, ljlist *js, llocalid x, lnodeid id) {
	return langL_branchif(fs,js,lfalse,x,id);
}


lbyteid langL_branchiftrue(FileState *fs, ljlist *js, llocalid x, lnodeid id) {
	return langL_branchif(fs,js,ltrue,x,id);
}


/*
** Similar to branch if true, but additionally this byte
** address becomes the false target, thus all false branches
** converge here. all true branches are returned.
*/
lbyteid *langL_jumpiftrue(FileState *fs, ljlist *js, llocalid x, lnodeid id) {
	langL_branchiftrue(fs,js,x,id);
	langL_tieloosejs(fs,js->f);
	langA_vardel(js->f);
	js->f = lnil;
	return js->t;
}


lbyteid *langL_jumpiffalse(FileState *fs, ljlist *js, llocalid x, lnodeid id) {
	langL_branchiffalse(fs,js,x,id);
	langL_tieloosejs(fs,js->t);
	langA_vardel(js->t);
	js->t = lnil;
	return js->f;
}


lbyteid *langL_jumpifnotnil(FileState *fs, llineid line, ljlist *js, lnodeid id) {
	return langL_jumpiffalse(fs,js,NO_SLOT,langN_xy(fs,line,Y_EQ,NT_BOL,id,langN_nil(fs,line)));
}


/* todo: add support for multiple results */
void langL_yield(FileState *fs, llineid line, lnodeid t) {
	int n = 0;
	llocalid x = 0;
	if (t != NO_NODE) {
		/* todo: determine the number of values in
		tree, and allocate that many registers? */
		x = langL_localalloc(fs,n=1);
		langL_localload(fs,line,ltrue,x,n,t);
		langL_localdealloc(fs,x);

		if (fs->fn->nyield < n) fs->fn->nyield = n;
		lbyteid j = langL_bytexyz(fs,line,BC_YIELD,NO_JUMP,x,n);
		langA_varadd(fs->fn->yj,j);

		/* if there are no results then simply leave directly */
	} else langL_byte(fs,line,BC_LEAVE,0);
}


llocalid langL_loadadjoined(FileState *fs, llineid line, llocalid adj, lnodeid id) {
	llocalid x = langL_localalloc(fs,1);
	if ((x - adj) != 0) {
		if (line == lnil) line = fs->nodes[id].line;
		langX_error(fs,line,"internal error, invalid memory state");
		LNOBRANCH;
	}
	langL_localload(fs,line,ltrue,x,1,id);
	return x;
}


llocalid langL_localize(FileState *fs, llineid line, lnodeid id) {
	llocalid x = fs->nodes[id].x;
	if (fs->nodes[id].k != N_LOCAL) {
		x = langL_localalloc(fs,1);
		langL_localload(fs,line,lfalse,x,1,id);
	}
	return x;
}


void langL_emit(FileState *fs, llineid line, lnodeid id) {
	lNode v = fs->nodes[id];
	switch (v.k) {
		case NODE_LOAD: {
			lNode x = fs->nodes[v.x];
			if ((x.k == N_LOCAL)) {
				LNOBRANCH;
			} else
			if ((x.k == N_FIELD) || (x.k == N_INDEX)) {
				llocalid xx = langL_localize(fs,line,x.x);
				llocalid xy = langL_localize(fs,line,x.y);
				llocalid yy = langL_localize(fs,line,v.y);
				if (x.k == N_FIELD) {
					langL_bytexyz(fs,line,BC_SETFIELD,xx,xy,yy);
				} else langL_bytexyz(fs,line,BC_SETINDEX,xx,xy,yy);
			} else LNOBRANCH;
		} break;
		default: LNOBRANCH;
	}
}


/*
** emits bytecode to load id into local x,
** if y is 0 the instruction is omitted if
** no side effects.
*/
void langL_localload(FileState *fs, llineid line, lbool reload, llocalid x, llocalid y, lnodeid id) {
	LASSERT(x > NO_SLOT);
	lNode v = fs->nodes[id];
	LASSERT(v.level <= fs->level);
	if (line == 0) line = v.line;

	lModule *md = fs->md;

	/* finally restore memory state */
	llocalid mem = fs->fn->xlocals;

	if ((v.r != NO_SLOT) && (reload != ltrue)) {
		goto leave;
	}
	LASSERT((v.k != N_LOCAL) || (v.r == v.x));
	LASSERT((v.r < mem));

	fs->nodes[id].r = x;

	/* some instructions have no side effects,
	in which case, if the yield count is 0, the
	instruction is not emitted */
	switch (v.k) {
		/* todo: remove this? */
		case N_LOCAL: {
			if (reload) {
				langL_bytexy(fs,line,BC_RELOAD,x,v.x);
			} else goto leave;
		} break;
		case Y_GLOBAL: {
			if (y == 0) goto leave;
			langL_bytexy(fs,line,BC_LOADGLOBAL,x,v.x);
		} break;
		case N_CACHE: {
			if (y == 0) goto leave;
			langL_bytexy(fs,line,BC_LOADCACHED,x,v.x);
		} break;
		case N_NIL: {
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
			/* todo: interning */
			int yy = langA_variadd(fs->md->kn,1);
			fs->md->kn[yy] = v.lit.n;
			langL_bytexy(fs,line,BC_LOADNUM,x,yy);
		} break;
		case Y_STRING: {
			if (y == 0) goto leave;
			/* -- todo: allocate this in constant pool */
			int g = lang_addglobal(fs->md,0,lang_S(langS_new(fs->rt,v.lit.s)));
			langL_bytexy(fs,line,BC_LOADGLOBAL,x,g);
		} break;
		case N_TABLE: {
			if (y == 0) goto leave;
			LASSERT(v.line != 0);
			langL_bytexy(fs,line,BC_TABLE,x,0);

			/* -- todo: this is temporary work around,
			otherwise any other instruction that circularly
			references this one will continue allocating
			a register for this instruction, this will
			be replaced with a more robust system in
			the near future */
			fs->nodes[id].k = N_LOCAL;
			fs->nodes[id].x = x;

			langA_varifor(v.z) langL_emit(fs,line,v.z[i]);
		} break;
		case N_FIELD: case N_INDEX: {
			if (y == 0) goto leave;
			llocalid xx = langL_localize(fs,line,v.x);
			llocalid yy = langL_localize(fs,line,v.y);
			langL_bytexyz(fs,line,treetobyte(v.k),x,xx,yy);
		} break;
		case N_FUN: {
			if (y == 0) goto leave;
			llocalid xx = x;
			langA_varifor(v.z) {
				if (i != 0) xx = langL_localalloc(fs,1);
				if ((xx - x) != i) LNOBRANCH;
				langL_localload(fs,line,ltrue,xx++,1,v.z[i]);
			}
			langL_bytexy(fs,line,BC_CLOSURE,x,v.x);
		} break;
		case Y_GROUP: {
			if (y == 0) goto leave;
			langL_localload(fs,line,reload,x,y,v.x);
		} break;
		/* -- todo: could do through libfn? */
		case Y_LOADFILE: {
			langL_localload(fs,line,ltrue,x,1,v.x);
			langL_bytexy(fs,line,BC_LOADFILE,x,y);
		} break;
		case Y_BUILTIN: { LNOBRANCH;
			llocalid xx = x;
			int n = langA_varlen(v.z);
			langA_varifor(v.z) {
				langL_loadadjoined(fs,NO_LINE,xx ++,v.z[i]);
			}
			if (v.x == TK_STKLEN) {
				langL_bytexy(fs,line,x,BC_STKLEN,MAX(n,y));
			} else
			if (v.x == TK_STKGET) {
				langL_bytexy(fs,line,x,BC_STKGET,MAX(n,y));
			} else LNOBRANCH;
		} break;
		case Y_CALL: case Y_MCALL: {
			llocalid xx = x;
			langL_localload(fs,NO_LINE,ltrue,xx ++,1,v.x);
			if (v.k == Y_MCALL) {
				langL_loadadjoined(fs,NO_LINE,xx ++,v.y);
			}
			langA_varifor(v.z) {
				langL_loadadjoined(fs,NO_LINE,xx ++,v.z[i]);
			}

			/* todo: should this be done here or not? */
			/* allocate remaining space for the return registers */
			llocalid rem = y-xx-x;
			if (rem > 0) {
				if ((xx - langL_localalloc(fs,rem)) != 0) {
					LNOBRANCH;
				}
			}

			int n = langA_varlen(v.z);
			langL_bytexyz(fs,line,treetobyte(v.k),x,n,y);
		} break;
		case Y_LOG_AND: case Y_LOG_OR: {
			if (y == 0) goto leave;
			/* todo: we need to optimize this, better support for
			boolean expressions */
			ljlist js = {0};
			langL_jumpiffalse(fs,&js,x,id);
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
			if (y == 0) goto leave;
			if ((v.k == Y_GT) || (v.k == Y_GTEQ)) {
				llocalid xx = langL_localize(fs,line,v.y);
				llocalid yy = langL_localize(fs,line,v.x);
				/* -- see lbyte.h for (v.k^1) */
				langL_bytexyz(fs,line,treetobyte(v.k^1),x,xx,yy);
			} else
			if (v.k == Y_EQ) {
				if(1) goto _else;
				if (fs->nodes[v.y].k == N_NIL) {
					llocalid xx = langL_localize(fs,line,v.y);
					langL_bytexy(fs,line,BC_ISNIL,x,xx);
				} else goto _else;
			} else { _else:
				llocalid xx = langL_localize(fs,line,v.x);
				llocalid yy = langL_localize(fs,line,v.y);
				langL_bytexyz(fs,line,treetobyte(v.k),x,xx,yy);
			}
		} break;
		default: LNOBRANCH;
	}

	leave:
	fs->fn->xlocals = mem;
}


void langL_moveto(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	lNode v = fs->nodes[x];
	LASSERT(v.level <= fs->level);
	LASSERT(x >= 0);
	LASSERT(y >= 0);
	if (line == 0) line = v.line;

	/* keep local state, finally free any temporary locals */
	llocalid mem = fs->fn->xlocals;
	switch (v.k) {
		case Y_GLOBAL: {
			llocalid yy = langL_localize(fs,line,y);
			langL_bytexy(fs,line,BC_SETGLOBAL,v.x,yy);
		} break;
		case N_LOCAL: {
			/* -- todo: account for {x = x} opt ?  */
			langL_localload(fs,line,ltrue,v.x,1,y);
		} break;
		case N_INDEX: case N_FIELD: {
			llocalid xx = langL_localize(fs,line,v.x);
			llocalid ii = langL_localize(fs,line,v.y);
			llocalid yy = langL_localize(fs,line,y);
			lnodeop op = v.k == N_INDEX ? BC_SETINDEX : BC_SETFIELD;
			langL_bytexyz(fs,line,op,xx,ii,yy);
		} break;
		// {x}[{x}..{x}] = {y}
		case Y_RANGE_INDEX: {
			lnodeid lo = fs->nodes[v.y].x;
			lnodeid hi = fs->nodes[v.y].y;
			llocalid xx = langL_localize(fs,line,v.x);
			llocalid yy = langL_localize(fs,line,y);
			Loop loop = {0};
			langL_beginrangedloop(fs,line,&loop,NO_NODE,lo,hi);
			langL_bytexyz(fs,line,BC_SETINDEX,xx,loop.r,yy);
			langL_closerangedloop(fs,line,&loop);
		} break;
		default: {
			LNOBRANCH;
		} break;
	}


	fs->fn->xlocals = mem;
}


void langL_beginif(FileState *fs, llineid line, Select *s, lnodeid x, int z) {
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
	loop->x = NO_NODE;
	loop->f = lnil;
}


void langL_closedowhile(FileState *fs, llineid line, Loop *loop, lnodeid x) {
	ljlist js = {lnil};
	langL_jumpiftrue(fs,&js,NO_SLOT,x);

	langL_tieloosejsto(fs,js.t,loop->e);
	langA_vardel(js.t);
	js.t = lnil;
}


void langL_beginwhile(FileState *fs, llineid line, Loop *loop, lnodeid x) {
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


void langL_beginrangedloop(FileState *fs, llineid line, Loop *loop, lnodeid x, lnodeid lo, lnodeid hi) {
	/*  todo: this is temporary */
	if (x == NO_NODE) loop->r = langL_localalloc(fs,1);
	else loop->r = langL_localize(fs,line,x);
	loop->x = langN_local(fs,line,loop->r);

	langL_localload(fs,line,ltrue,loop->r,1,lo);

	int c = langN_xy(fs,line,Y_LT,NT_BOL,loop->x,hi);
	loop->e = langL_getlabel(fs);

	ljlist js = {lnil};
	loop->f = langL_jumpiffalse(fs,&js,NO_SLOT,c);
}


void langL_closerangedloop(FileState *fs, llineid line, Loop *loop) {
	int x = loop->x;
	int k = langN_xy(fs,NO_LINE,Y_ADD,NT_INT,x,langN_longint(fs,NO_LINE,1));
	langL_moveto(fs,line,x,k);
	langL_jump(fs,line,loop->e);
	langL_tieloosejs(fs,loop->f);
	langA_vardel(loop->f);
	loop->f = 0;
	if (loop->y) langL_localdealloc(fs,loop->r);
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


lbyteop treetobyte(lnodeop tt) {
	switch (tt) {
		case N_FIELD: 	 return BC_FIELD;
		case N_INDEX: 	 return BC_INDEX;
		case Y_CALL:    return BC_CALL;
		case Y_MCALL: 	 return BC_METACALL;
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


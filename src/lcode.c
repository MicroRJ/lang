/*
** See Copyright Notice In elf.h
** (L) lcode.c
** Bytecode Generator (node -> bytecode)
*/


lbyteop nodetobyte(lnodeop tt);


lbyteid langL_getlabel(elf_FileState *fs) {
	return fs->md->nbytes;
}


llocalid langL_localalloc(elf_FileState *fs, llocalid n) {
	elf_assert(n > NO_SLOT);

	elf_FileFunc *fn = fs->fn;

	llocalid id = fn->xmemory;
	fn->xmemory += n;

	if (fn->nlocals < fn->xmemory) {
		fn->nlocals = fn->xmemory;
	}
	return id;
}


void langL_localdealloc(elf_FileState *fs, llocalid x) {
	elf_assert(x > NO_SLOT);

	elf_FileFunc *fn = fs->fn;

	elf_assert(x == fn->xmemory-1);
	fn->xmemory = x;
}


lbyteid langL_addbyte(elf_FileState *fs, llineid line, lBytecode byte) {
	elf_FileFunc *fn = fs->fn;
	elf_Module *md = fs->md;
	if (0) bytefpf(md,stdout,md->nbytes-fn->bytes,byte);

	elf_arradd(md->lines,line);
	elf_arradd(md->bytes,byte);
	return md->nbytes ++;
}


lbyteid langL_byte(elf_FileState *fs, llineid line, lbyteop k, elf_int i) {
	return langL_addbyte(fs,line,(lBytecode){k,i});
}


lbyteid elf_emitbytexy(elf_FileState *fs, llineid line, lbyteop k, int x, int y) {
	lBytecode b = (lBytecode){k};
	b.x = x;
	b.y = y;
	return langL_addbyte(fs,line,b);
}


lbyteid elf_emitbytexyz(elf_FileState *fs, llineid line, lbyteop k, int x, int y, int z) {
	lBytecode b = (lBytecode){k};
	b.x = x;
	b.y = y;
	b.z = z;
	return langL_addbyte(fs,line,b);
}


void langL_tieloosejto(elf_FileState *fs, lbyteid id, lbyteid j) {
	elf_Module *md = fs->md;
	lBytecode *bytes = md->bytes;
	lBytecode b = bytes[id];

	lbyteid l = j - id;
	// if (l == NO_JUMP) {
	// 	elf_lineerror(fs,md->lines[id],"opt, no jump");
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


void langL_tieloosej(elf_FileState *fs, lbyteid i) {
	langL_tieloosejto(fs,i,langL_getlabel(fs));
}


void langL_tieloosejs(elf_FileState *fs, lbyteid *js) {
	elf_arrfori(js) {
		langL_tieloosej(fs,js[i]);
	}
}


void langL_tieloosejsto(elf_FileState *fs, lbyteid *js, lbyteid j) {
	elf_arrfori(js) {
		langL_tieloosejto(fs,js[i],j);
	}
}


lbyteid langL_jump(elf_FileState *fs, llineid line, lbyteid j) {
	return langL_byte(fs,line,BC_J,j-fs->md->nbytes);
}


void langL_fnepiloge(elf_FileState *fs, llineid line) {
	elf_Module *md = fs->md;
	elf_FileFunc *fn = fs->fn;

	/* todo: this is unncessary, we know were
	the return instruction is at. */
	langL_tieloosejs(fs,fn->yj);
	elf_delvar(fn->yj);
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
lbyteid langL_branchif(elf_FileState *fs, ljlist *js, elf_bool z, llocalid x, lnodeid id) {
	lNode v = fs->nodes[id];
	lbyteid j = NO_BYTE;

	llocalid mem = fs->fn->xmemory;
	/* -------------------------------
	if not provided one, allocate temporary
	register here, notice how this is done
	before we keep splitting, this will make
	is so subsequent recursive calls to this
	function won't keep allocating registers,
	we get away with using only one register
	due to the transitive nature of short
	circuiting, expressible in the bytecode. */
	if (x == NO_SLOT) x = langL_localalloc(fs,1);

	switch (v.k) {
		case NODE_GROUP: {
			j = langL_branchif(fs,js,z,x,v.x);
		} break;
		/* '&&' expressions short-circuits to a false
		jump as early as possible and the opposite
		is true for the '||' expressions, last jump is
		whatever the user specified */
		case NODE_AND: {
			langL_jumpiffalse(fs,js,x,v.x);
			j = langL_branchif(fs,js,z,x,v.y);
		} break;
		case NODE_OR: {
			langL_jumpiftrue(fs,js,x,v.x);
			j = langL_branchif(fs,js,z,x,v.y);
		} break;
		default: {
			langL_localload(fs,NO_LINE,lfalse,x,1,id);

			if (z != 0) {
				j = elf_emitbytexy(fs,v.line,BC_JNZ,NO_JUMP,x);
				elf_arradd(js->t,j);
			} else {
				j = elf_emitbytexy(fs,v.line,BC_JZ,NO_JUMP,x);
				elf_arradd(js->f,j);
			}
		} break;
	}


	fs->fn->xmemory = mem;
	return j;
}


lbyteid langL_branchiffalse(elf_FileState *fs, ljlist *js, llocalid x, lnodeid id) {
	return langL_branchif(fs,js,lfalse,x,id);
}


lbyteid langL_branchiftrue(elf_FileState *fs, ljlist *js, llocalid x, lnodeid id) {
	return langL_branchif(fs,js,ltrue,x,id);
}


/*
** Similar to branch if true, but additionally this byte
** address becomes the false target, thus all false
** branches converge here. all true branches are returned.
*/
lbyteid *langL_jumpiftrue(elf_FileState *fs, ljlist *js, llocalid x, lnodeid id) {
	langL_branchiftrue(fs,js,x,id);
	langL_tieloosejs(fs,js->f);
	elf_delvar(js->f);
	js->f = lnil;
	return js->t;
}


lbyteid *langL_jumpiffalse(elf_FileState *fs, ljlist *js, llocalid x, lnodeid id) {
	langL_branchiffalse(fs,js,x,id);
	langL_tieloosejs(fs,js->t);
	elf_delvar(js->t);
	js->t = lnil;
	return js->f;
}


lbyteid *langL_jumpifnotnil(elf_FileState *fs, llineid line, ljlist *js, lnodeid id) {
	return langL_jumpiffalse(fs,js,NO_SLOT,elf_nodebinary(fs,line,NODE_EQ,NT_BOL,id,elf_nodenil(fs,line)));
}


/* todo: add support for multiple results */
void langL_yield(elf_FileState *fs, llineid line, lnodeid id) {
	int n = 0;
	llocalid x = 0;
	if (id != NO_NODE) {
		/* todo: determine the number of values in
		tree, and allocate that many registers? */
		x = langL_localalloc(fs,n=1);
		langL_localload(fs,line,ltrue,x,n,id);
		langL_localdealloc(fs,x);

		if (fs->fn->nyield < n) fs->fn->nyield = n;
		lbyteid j = elf_emitbytexyz(fs,line,BC_YIELD,NO_JUMP,x,n);
		elf_arradd(fs->fn->yj,j);

		/* if there are no results then simply leave directly */
	} else langL_byte(fs,line,BC_LEAVE,0);
}


/*
*/
void elf_Ltestmemory(elf_FileState *fs, llineid line, llocalid r) {
	if ((fs->fn->xmemory - r) != 1) {
		elf_lineerror(fs,line,"invalid memory state");
	}
}


/*
** Ensures that the next free local is the
** given one and loads the node to that
** local.
*/
void langL_localloadin(elf_FileState *fs, llineid line, llocalid r, lnodeid id) {
	/* the user can provide exactly the next free register,
	or exactly the last allocated register  */
	if ((fs->fn->xmemory - r) == 0) {
		langL_localalloc(fs,1);
	}
	if ((fs->fn->xmemory - r) != 1) {
		elf_lineerror(fs,line,"invalid memory state");
	}
	langL_localload(fs,line,ltrue,r,1,id);
}


llocalid langL_localize(elf_FileState *fs, llineid line, lnodeid id) {
	lNode v = fs->nodes[id];
	/* todo: this literally contradicts SSA, this system is
	obsolete, replace with something else... */
	llocalid r = v.r;
	/* the node is currently allocated */
	if ((r != NO_SLOT) && (r < fs->fn->xmemory)) {
		goto leave;
	} else r = langL_localalloc(fs,1);
	langL_localload(fs,line,lfalse,r,1,id);
	leave: return r;
}


void langL_emit(elf_FileState *fs, llineid line, lnodeid id) {
	llocalid mem = fs->fn->xmemory;
	lNode v = fs->nodes[id];
	switch (v.k) {
		case NODE_LOAD: {
			lNode x = fs->nodes[v.x];
			if ((x.k == NODE_LOCAL)) {
				LNOBRANCH;
			} else
			if ((x.k == NODE_FIELD) || (x.k == NODE_INDEX)) {
				llocalid xx = langL_localize(fs,line,x.x);
				llocalid xy = langL_localize(fs,line,x.y);
				llocalid yy = langL_localize(fs,line,v.y);
				if (x.k == NODE_FIELD) {
					elf_emitbytexyz(fs,line,BC_SETFIELD,xx,xy,yy);
				} else elf_emitbytexyz(fs,line,BC_SETINDEX,xx,xy,yy);
			} else LNOBRANCH;
		} break;
		default: LNOBRANCH;
	}
	fs->fn->xmemory = mem;
}


/*
** emits bytecode to load id into local x,
** if y is 0 the instruction is omitted if
** no side effects.
*/
void langL_localload(elf_FileState *fs, llineid line, elf_bool reload, llocalid x, llocalid y, lnodeid id) {
	elf_assert(x > NO_SLOT);
	elf_assert(x < fs->fn->xmemory);

	lNode v = fs->nodes[id];
	elf_assert(v.level <= fs->level);

	if (line == 0) line = v.line;

	elf_Module *md = fs->md;
	elf_FileFunc *fn = fs->fn;

	/* finally restore memory state */
	llocalid mem = fs->fn->xmemory;

	if ((v.r != NO_SLOT) && (reload != ltrue)) {
		/* node is already allocated, and we're
		not asked to reload it */
		if (v.r <= mem) goto leave;
		v.r = NO_SLOT;
	}
	elf_assert((v.r < mem));
	elf_assert((v.k != NODE_LOCAL) || (v.r == v.x));


	fs->nodes[id].r = x;

	/* some instructions have no side effects,
	in which case, if the yield count is 0, the
	instruction is not emitted */
	switch (v.k) {
		/* todo: remove this? */
		case NODE_LOCAL: {
			if (y == 0) goto leave;
			if (reload) {
				elf_emitbytexy(fs,line,BC_RELOAD,x,v.x);
			} else goto leave;
		} break;
		case NODE_THIS: {
			if (y == 0) goto leave;
			langL_byte(fs,line,BC_LOADTHIS,x);
		} break;
		case NODE_CACHE: {
			if (y == 0) goto leave;
			elf_emitbytexy(fs,line,BC_LOADCACHED,x,v.x);
		} break;
		case NODE_GLOBAL: {
			if (y == 0) goto leave;
			elf_emitbytexy(fs,line,BC_LOADGLOBAL,x,v.x);
		} break;
		case NODE_NIL: {
			if (y == 0) goto leave;
			/* -- todo: coalesce */
			elf_emitbytexy(fs,line,BC_LOADNIL,x,y);
		} break;
		case NODE_INTEGER: {
			if (y == 0) goto leave;
			/* todo: interning */
			int yy = langA_variadd(fs->md->ki,1);
			fs->md->ki[yy] = v.lit.i;
			elf_emitbytexy(fs,line,BC_LOADINT,x,yy);
		} break;
		case NODE_NUMBER: {
			if (y == 0) goto leave;
			/* todo: interning */
			int yy = langA_variadd(fs->md->kn,1);
			fs->md->kn[yy] = v.lit.n;
			elf_emitbytexy(fs,line,BC_LOADNUM,x,yy);
		} break;
		case NODE_STRING: {
			if (y == 0) goto leave;
			/* -- todo: allocate this in constant pool */
			int g = lang_addglobal(fs->md,0,lang_S(elf_newstr(fs->rt,v.lit.s)));
			elf_emitbytexy(fs,line,BC_LOADGLOBAL,x,g);
		} break;
		case NODE_TABLE: {
			if (y == 0) goto leave;
			elf_assert(v.line != 0);
			elf_emitbytexy(fs,line,BC_TABLE,x,0);
			elf_arrfori(v.z) langL_emit(fs,line,v.z[i]);
		} break;
		case NODE_FIELD: case NODE_INDEX: {
			if (y == 0) goto leave;
			llocalid xx = langL_localize(fs,line,v.x);
			llocalid yy = langL_localize(fs,line,v.y);
			elf_emitbytexyz(fs,line,nodetobyte(v.k),x,xx,yy);
		} break;
		case NODE_CLOSURE: {
			if (y == 0) goto leave;
			llocalid xx = x;
			elf_arrfori(v.z) {
				if (i != 0) xx = langL_localalloc(fs,1);
				if ((xx - x) != i) LNOBRANCH;
				langL_localload(fs,line,ltrue,xx++,1,v.z[i]);
			}
			elf_emitbytexy(fs,line,BC_CLOSURE,x,v.x);
		} break;
		case NODE_TYPEGUARD: {
			if (y == 0) goto leave;
			langL_localload(fs,line,reload,x,y,v.x);
			fs->nodes[id].r = x = fs->nodes[v.x].r;
			elf_emitbytexy(fs,v.line,BC_TYPEGUARD,x,elf_nodettotag(v.y));
		} break;
		case NODE_GROUP: {
			if (y == 0) goto leave;
			langL_localload(fs,line,reload,x,y,v.x);
		} break;
		/* -- todo: could do through libfn? */
		case NODE_FILE: {
			langL_localload(fs,line,ltrue,x,1,v.x);
			elf_emitbytexy(fs,line,BC_LOADFILE,x,y);
		} break;
		case NODE_BUILTIN: {
			llocalid xx = x;
			int n = elf_arrlen(v.z);
			elf_arrfori(v.z) {
				langL_localloadin(fs,NO_LINE,xx ++,v.z[i]);
			}
			if (v.x == TK_STKLEN) {
				langL_byte(fs,v.line,BC_STKLEN,x);
			} else
			if (v.x == TK_STKGET) {
				elf_emitbytexy(fs,v.line,BC_STKGET,x,x);
			} else LNOBRANCH;
		} break;
		case NODE_METAFIELD: {
			if (y == 0) goto leave;
			llocalid rx = langL_localize(fs,line,v.x);
			llocalid ry = langL_localize(fs,line,v.y);
			elf_emitbytexyz(fs,line,nodetobyte(v.k),x,rx,ry);
		} break;
		case NODE_CALL: {
			/* [FIX] 4.25.2024: fixed bug where I would
			issue a call instruction at x, when x was
			reserved for locals, call instructions should
			always be done with free registers. */
			/* is the register given to us the last free
			register? otherwise allocate a new register.
			note that so far this only happens when you
			do stuff like this: a = a:add(b), where a
			was defined way before and the caller expects
			us to put the result there at a, we can't do
			so directly because if there's another variable
			say b, defined right after a we could overwrite
			it, so ensure that we allocate a free register
			then. */
			llocalid head = x;
			if ((fs->fn->xmemory - head) != 1) {
				head = langL_localalloc(fs,1);
			}
			elf_Ltestmemory(fs,line,head);
			lNode xx = fs->nodes[v.x];
			/* allocate v.x.x (the object) here because
			obviously if we just load v.x it'll reset
			its memory state and we'll loose v.x, so
			just do it here beforehand for convenience.
			once we load v.x, since v.x.x was already
			allocated and we didn't free it because
			we're still within the subexpression, it'll
			reuse that register. */
			llocalid tail = head;
			if (xx.k == NODE_METAFIELD) {
				langL_localloadin(fs,line,tail ++,xx.x);
			}
			langL_localloadin(fs,line,tail ++,v.x);
			elf_arrfori(v.z) {
				langL_localloadin(fs,line,tail ++,v.z[i]);
			}
			int n = elf_arrlen(v.z);
			elf_emitbytexyz(fs,line,xx.k == NODE_METAFIELD ? BC_METACALL : BC_CALL,head,n,y);
			/* and finally, if the registers don't match,
			meaning the caller expects the result in a
			different register, move it there */
			if (head != x) {
				/* also, ensure that we don't expect more than
				one result if this is case, because we don't
				support that */
				if (y != 1) elf_lineerror(fs,line,"unsupported");

				elf_emitbytexy(fs,line,BC_RELOAD,x,head);
			}
		} break;
		case NODE_AND: case NODE_OR: {
			if (y == 0) goto leave;
			/* todo: we need to optimize this, better support for
			boolean expressions */
			ljlist js = {0};
			langL_jumpiffalse(fs,&js,NO_SLOT,id);
			langL_localload(fs,line,ltrue,x,1,elf_nodeint(fs,line,ltrue));
			int j = langL_jump(fs,line,-1);
			langL_tieloosejs(fs,js.f);
			elf_delvar(js.f);
			js.f = lnil;
			langL_localload(fs,line,ltrue,x,1,elf_nodeint(fs,line,lfalse));
			langL_tieloosej(fs,j);
		} break;
		case NODE_NEQ: case NODE_EQ:
		case NODE_GT: case NODE_LT:
		case NODE_GTEQ: case NODE_LTEQ:
		case NODE_DIV: case NODE_MUL: case NODE_MOD:
		case NODE_SUB: case NODE_ADD:
		case NODE_BITSHL: case NODE_BITSHR: case NODE_BITXOR: {
			if (y == 0) goto leave;
			if ((v.k == NODE_GT) || (v.k == NODE_GTEQ)) {
				llocalid xx = langL_localize(fs,line,v.y);
				llocalid yy = langL_localize(fs,line,v.x);
				elf_emitbytexyz(fs,line,nodetobyte(v.k^1),x,xx,yy);
			} else
			if (v.k == NODE_EQ) {
				/* todo: enable this */
				if(1) goto _else;
				if (fs->nodes[v.y].k == NODE_NIL) {
					llocalid xx = langL_localize(fs,line,v.y);
					elf_emitbytexy(fs,line,BC_ISNIL,x,xx);
				} else goto _else;
			} else { _else:
				llocalid xx = langL_localize(fs,line,v.x);
				llocalid yy = langL_localize(fs,line,v.y);
				elf_emitbytexyz(fs,line,nodetobyte(v.k),x,xx,yy);
			}
		} break;
		default: LNOBRANCH;
	}

	leave:
	fs->fn->xmemory = mem;
}


void langL_moveto(elf_FileState *fs, llineid line, lnodeid x, lnodeid y) {
	lNode v = fs->nodes[x];
	elf_assert(v.level <= fs->level);
	elf_assert(x >= 0);
	elf_assert(y >= 0);
	if (line == 0) line = v.line;

	/* keep local state, finally free any temporary locals */
	llocalid mem = fs->fn->xmemory;
	switch (v.k) {
		case NODE_GLOBAL: {
			llocalid yy = langL_localize(fs,line,y);
			elf_emitbytexy(fs,line,BC_SETGLOBAL,v.x,yy);
		} break;
		case NODE_CACHE: {
			elf_lineerror(fs,line,"assignment to cache value is not supported yet");
		} break;
		case NODE_LOCAL: {
			/* -- todo: account for {x = x} opt ?  */
			langL_localload(fs,line,ltrue,v.x,1,y);
		} break;
		case NODE_INDEX: case NODE_FIELD: {
			llocalid xx = langL_localize(fs,line,v.x);
			llocalid ii = langL_localize(fs,line,v.y);
			llocalid yy = langL_localize(fs,line,y);
			lnodeop op = v.k == NODE_INDEX ? BC_SETINDEX : BC_SETFIELD;
			elf_emitbytexyz(fs,line,op,xx,ii,yy);
		} break;
		case NODE_METAFIELD: {
			llocalid xx = langL_localize(fs,line,v.x);
			llocalid ii = langL_localize(fs,line,v.y);
			llocalid yy = langL_localize(fs,line,y);
			elf_emitbytexyz(fs,line,BC_SETMETAFIELD,xx,ii,yy);
		} break;
		// {x}[{x}..{x}] = {y}
		case NODE_RANGE_INDEX: {
			lnodeid lo = fs->nodes[v.y].x;
			lnodeid hi = fs->nodes[v.y].y;
			lnodeid ii = elf_nodelocal(fs,line,langL_localalloc(fs,1));
			llocalid xx = langL_localize(fs,line,v.x);
			llocalid yy = langL_localize(fs,line,y);
			Loop loop = {0};
			langL_beginrangedloop(fs,line,&loop,ii,lo,hi);
			elf_emitbytexyz(fs,line,BC_SETINDEX,xx,loop.r,yy);
			langL_closerangedloop(fs,line,&loop);
		} break;
		default: {
			LNOBRANCH;
		} break;
	}


	fs->fn->xmemory = mem;
}


void langL_beginif(elf_FileState *fs, llineid line, Select *s, lnodeid x, int z) {
	ljlist js = {0};
	langL_branchif(fs,&js,z,NO_SLOT,x);
	// if  0 = jz
	// iff 1 = jnz
	if (z == L_IF) {
		elf_assert(js.f != 0);
		langL_tieloosejs(fs,js.t);
		elf_delvar(js.t);
		js.t = 0;
		s->jz = js.f;
	} else {
		elf_assert(js.t != 0);
		langL_tieloosejs(fs,js.f);
		elf_delvar(js.f);
		js.f = 0;
		s->jz = js.t;
	}
}


/*
** Closes previous conditional block by emitting
** escape jump, patches previous jz (jump if false)
** list to enter this block.
*/
void langL_addelse(elf_FileState *fs, llineid line, Select *s) {
	elf_assert(s->jz != 0);
	int j = langL_jump(fs,line,-1);
	elf_arradd(s->j,j);

	langL_tieloosejs(fs,s->jz);
	elf_delvar(s->jz);
	s->jz = lnil;
}


void langL_addelif(elf_FileState *fs, llineid line, Select *s, int x) {
	langL_addelse(fs,line,s);
	langL_beginif(fs,line,s,x,L_IF);
}


void langL_addthen(elf_FileState *fs, llineid line, Select *s) {
	/* we don't need to close the previous block, it can just fall
	through to our branch, do collect all the other exit jumps and
	tie them to this branch block, naturally we don't need to add
	an exit jump since else and elif or closeif will terminate
	this block, multiple then blocks are simply chained together
	naturally. */
	langL_tieloosejs(fs,s->j);
	elf_delvar(s->j);
	s->j = lnil;
}


void langL_closeif(elf_FileState *fs, llineid line, Select *s) {
	/* collect missing else branch */
	if (s->jz != lnil) {
		langL_tieloosejs(fs,s->jz);
		elf_delvar(s->jz);
		s->jz = lnil;
	}
	/* collect missing then branch */
	if (s->j != lnil) {
		langL_tieloosejs(fs,s->j);
		elf_delvar(s->j);
		s->j = lnil;
	}
}


void langL_begindowhile(elf_FileState *fs, llineid line, Loop *loop) {
	loop->e = langL_getlabel(fs);
	loop->x = NO_NODE;
	loop->f = lnil;
}


void langL_closedowhile(elf_FileState *fs, llineid line, Loop *loop, lnodeid x) {
	ljlist js = {lnil};
	langL_jumpiftrue(fs,&js,NO_SLOT,x);

	langL_tieloosejsto(fs,js.t,loop->e);
	elf_delvar(js.t);
	js.t = lnil;
}


void langL_beginwhile(elf_FileState *fs, llineid line, Loop *loop, lnodeid x) {
	loop->e = langL_getlabel(fs);
	loop->x = x;
	ljlist js = {lnil};
	loop->f = langL_jumpiffalse(fs,&js,NO_SLOT,x);
}


void langL_closewhile(elf_FileState *fs, llineid line, Loop *loop) {
	langL_jump(fs,line,loop->e);
	langL_tieloosejs(fs,loop->f);
	elf_delvar(loop->f);
	loop->f = lnil;
}


lnodeid elf_nodeilessthan(elf_FileState *fs, llineid line, lnodeid x, lnodeid y) {
	x = elf_nodetypeguard(fs,fs->nodes[x].line,x,NT_INT);
	y = elf_nodetypeguard(fs,fs->nodes[y].line,y,NT_INT);
	return elf_nodebinary(fs,line,NODE_LT,NT_BOL,x,y);
}


void langL_beginrangedloop(elf_FileState *fs, llineid line, Loop *loop, lnodeid x, lnodeid lo, lnodeid hi) {
	elf_assert(x != NO_NODE);
	loop->x = x;
	loop->r = langL_localize(fs,line,x);
	langL_localload(fs,line,ltrue,loop->r,1,lo);

	loop->e = langL_getlabel(fs);

	langL_localize(fs,line,hi);
	lnodeid c = elf_nodeilessthan(fs,line,loop->x,hi);

	ljlist js = {lnil};
	loop->f = langL_jumpiffalse(fs,&js,NO_SLOT,c);
}


void langL_closerangedloop(elf_FileState *fs, llineid line, Loop *loop) {
	elf_assert(loop->r == fs->nodes[loop->x].r);
	int x = loop->x;
	int k = elf_nodebinary(fs,NO_LINE,NODE_ADD,NT_INT,x,elf_nodeint(fs,NO_LINE,1));
	langL_moveto(fs,line,x,k);
	langL_jump(fs,line,loop->e);
	langL_tieloosejs(fs,loop->f);
	elf_delvar(loop->f);
	loop->f = 0;
}


/* -- todo? we could merge with adjacent blocks, but this
would change the order of execution, should leave as is? */
void langL_begindelayedblock(elf_FileState *fs, llineid line, FileBlock *d) {
	d->j = langL_byte(fs,line,BC_DELAY,NO_JUMP);
}


void langL_closedelayedblock(elf_FileState *fs, llineid line, FileBlock *bl) {
	// elf_lineerror(fs,line,"closed block, %i",langL_getlocallabel(fs));

	elf_FileFunc *fn = fs->fn;

	langL_byte(fs,line,BC_LEAVE,0);
	langL_tieloosej(fs,bl->j);
}


lbyteop nodetobyte(lnodeop tt) {
	switch (tt) {
		case NODE_FIELD: 	  	return BC_FIELD;
		case NODE_INDEX: 	  	return BC_INDEX;
		case NODE_CALL:     	return BC_CALL;
		case NODE_METAFIELD:	return BC_METAFIELD;
		case NODE_ADD:     	return BC_ADD;
		case NODE_SUB:     	return BC_SUB;
		case NODE_DIV:     	return BC_DIV;
		case NODE_MUL:     	return BC_MUL;
		case NODE_MOD:     	return BC_MOD;
		case NODE_NEQ:     	return BC_NEQ;
		case NODE_EQ:      	return BC_EQ;
		case NODE_LT:      	return BC_LT;
		case NODE_LTEQ:    	return BC_LTEQ;
		case NODE_BITSHL:    	return BC_SHL;
		case NODE_BITSHR:    	return BC_SHR;
		case NODE_BITXOR:    	return BC_XOR;
	}
	/* for intended use cases, this is an error */
	LNOBRANCH;
	return NODE_NONE;
}


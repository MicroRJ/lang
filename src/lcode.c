/*
** See Copyright Notice In lang.h
** (L) lcode.c
** Bytecode Generator
*/


int byteeffect(lModule *md, lBytecode b);
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
	// langL_localalloc(fs,byteeffect(md,byte));
	// lang_loginfo("%s %lli: byte: %s, %lli"
	// , fs->filename,md->nbytes,lang_bytename(byte.k),byte.i);
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
		case BC_JZ:
		case BC_JNZ:
		case BC_DELAY: {
			bytes[id].i = l;
		} break;
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


void langL_loadinto(FileState *fs, llineid line, ltreeid x, ltreeid y);


int langL_localloadadj(FileState *fs, llocalid y, ltreeid *z);


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
**
**
*/
lbyteid langL_jumpif(FileState *fs, ljlist *js, ltreeid x, lbool z) {
	Tree v = fs->nodes[x];
	lbyteid jid = NO_BYTE;
	switch (v.k) {
		case Y_LOG_AND: {
			langL_jumpif(fs,js,v.x,0);
			langL_tieloosejs(fs,js->t);
			langA_vardel(js->t);
			js->t = 0;
			jid = langL_jumpif(fs,js,v.y,z);
		} break;
		case Y_LOG_OR: {
			langL_jumpif(fs,js,v.x,1);
			langL_tieloosejs(fs,js->f);
			langA_vardel(js->f);
			js->f = 0;
			jid = langL_jumpif(fs,js,v.y,z);
		} break;
		default: {
			llocalid reg = langL_load2any(fs,v.line,1,x);
			if (z != 0) {
				jid = langL_byte(fs,v.line,BC_JNZ,NO_JUMP);
				langA_varadd(js->t,jid);
			} else {
				jid = langL_byte(fs,v.line,BC_JZ,NO_JUMP);
				langA_varadd(js->f,jid);
			}
		} break;
	}

	return jid;
}


lbyteid langL_jumpiffalse(FileState *fs, ljlist *js, ltreeid x) {
	return langL_jumpif(fs,js,x,lfalse);
}


lbyteid langL_jumpiftrue(FileState *fs, ljlist *js, ltreeid x) {
	return langL_jumpif(fs,js,x,ltrue);
}



/* todo: add support for multiple results */
void langL_yield(FileState *fs, llineid line, ltreeid t) {
	int n = 0;
	if (t != NO_TREE) {
		/* todo: determine the number of values in
		tree, and allocate that many registers?
		what if we return a function that returns
		multiple values? */
		llocalid x = langL_localalloc(fs,n=1);
		langL_reload(fs,line,ltrue,x,n,t);
		langL_localdealloc(fs,x);
	}
	if (fs->fn->nyield < n) {
		fs->fn->nyield = n;
	}
	lbyteid j = langL_bytexy(fs,line,BC_YIELD,NO_JUMP,n);
	langA_varadd(fs->fn->yj,j);
}


llocalid langL_load2any(FileState *fs, llineid line, llocalid n, ltreeid id) {
	if (fs->nodes[id].k != Y_LOCAL) {
		return fs->nodes[id].x;
	}
	llocalid x = langL_localalloc(fs,n);
	langL_reload(fs,line,lfalse,x,n,id);
	return x;
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
	/**/langL_reload(fs,NO_LINE,ltrue,id+i,1,z[i]);
	fs->fn->xlocals = id + y;
	return n;
}


/*
** x is preallocated stack slot to put the result
** in, y the number of slots allocated after x.
** if y is NO_SLOT the instruction is ommitted unless
** it has side-effects, such as a function call,
** but no registers are allocated for it, if x.
** if reload is specified, when the value is already
** on the stack, then it is reloaded into x.
*/
/* -- Some instructions have no side effects,
- in which case, if the yield count is 0, the
- instruction is not emitted */
void langL_reload(FileState *fs, llineid line, lbool reload, llocalid x, llocalid y, ltreeid id) {
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
			LNOBRANCH;
			// langL_load2any(fs,NO_LINE,v.x,1);
			// langL_load2any(fs,NO_LINE,v.y,1);
			// langL_byte(fs,line,BC_FIELD,0);
		} break;
		case Y_INDEX: {
			LNOBRANCH;
			// langL_load2any(fs,NO_LINE,v.x,1);
			// langL_load2any(fs,NO_LINE,v.y,1);
			// langL_byte(fs,line,BC_INDEX,0);
		} break;
		case Y_GROUP: {
			langL_reload(fs,line,reload,x,v.x,y);
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
			// langL_reload(fs,line,x,v.x,1);
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
			int n = langL_localloadadj(fs,y,v.z);
			langL_bytexyz(fs,line,BC_CALL,x,v.x,MAX(n,y));
		} break;
		case Y_LOG_AND: case Y_LOG_OR: {
			ljlist js = {0};
			langL_jumpiffalse(fs,&js,x);
			langL_tieloosejs(fs,js.t);
			langA_vardel(js.t);
			js.t = lnil;
			langL_byte(fs,line,BC_INT,ltrue);
			int j = langL_jump(fs,line,-1);
			langL_tieloosejs(fs,js.f);
			langA_vardel(js.f);
			js.f = lnil;
			langL_byte(fs,line,BC_INT,lfalse);
			langL_tieloosej(fs,j);
		} break;
		case Y_GT: {
			llocalid r = langL_localalloc(fs,1);
			langL_reload(fs,line,NO_SLOT,x,v.y,1);
			langL_reload(fs,line,NO_SLOT,r,v.x,1);
			langL_localdealloc(fs,r);
			langL_byte(fs,line,BC_LT,0);
		} break;
		case Y_GTEQ: {
			llocalid r = langL_localalloc(fs,1);
			langL_reload(fs,line,lfalse,x,v.y,1);
			langL_reload(fs,line,lfalse,r,v.x,1);
			langL_localdealloc(fs,r);
			langL_bytexyz(fs,line,BC_LTEQ,x,x,r);
		} break;
		case Y_EQ: {
			if (fs->nodes[v.y].k == Y_NIL) {
				langL_reload(fs,line,lfalse,x,v.x,1);
				langL_bytexy(fs,line,BC_ISNIL,x,x);
			} else {
				llocalid r = langL_localalloc(fs,1);
				langL_reload(fs,line,lfalse,x,v.x,1);
				langL_reload(fs,line,lfalse,r,v.y,1);
				langL_localdealloc(fs,r);
				langL_bytexyz(fs,line,BC_EQ,x,x,r);
			}
		} break;
		case Y_LT: case Y_LTEQ: case Y_NEQ:
		case Y_DIV: case Y_MUL: case Y_MOD:
		case Y_SUB: case Y_ADD:
		case Y_BSHL: case Y_BSHR: case Y_BXOR: {
			llocalid r = langL_localalloc(fs,1);
			langL_reload(fs,line,lfalse,x,v.x,1);
			langL_reload(fs,line,lfalse,r,v.y,1);
			langL_localdealloc(fs,r);
			langL_bytexyz(fs,line,treetobyte(v.k),x,x,r);
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
			langL_reload(fs,line,lfalse,r,y,1);
			langL_localdealloc(fs,r);
			langL_bytexy(fs,line,BC_SETGLOBAL,v.x,r);
		} break;
		case Y_LOCAL: {
			langL_reload(fs,line,lfalse,v.x,1,y);
		} break;
		case Y_INDEX: {
			LNOBRANCH;
			// llocalid table = langL_load2any(fs,line,NO_SLOT,v.x,1);
			// llocalid index = langL_load2any(fs,line,NO_SLOT,v.y,1);
			// llocalid value = langL_load2any(fs,line,NO_SLOT,  y,1);
			// langL_bytexyz(fs,line,BC_SETINDEX,table,index,value);
		} break;
		case Y_FIELD: {
			LNOBRANCH;
			// langL_load2any(fs,line,v.x,1);/* table */
			// langL_load2any(fs,line,v.y,1);/* index */
			// langL_load2any(fs,line,  y,1);/* value */
			// langL_byte(fs,line,BC_SETFIELD,0);
		} break;
		// {x}[{x}..{x}] = {y}
		case Y_RANGE_INDEX: {
			LNOBRANCH;
			// Loop l = {0};
			// langL_load2any(fs,line,v.x,1);/* table */
			// langL_beginloop(fs,line,&l,-1,v.y);
			// langL_load2any(fs,line,l.x,1);/* index */
			// langL_load2any(fs,line,y,1);/* value */
			// langL_byte(fs,line,BC_SETINDEX,0);
			// langL_closeloop(fs,line,&l);
		} break;
		default: {
			LNOBRANCH;
		} break;
	}
}


void langL_beginif(FileState *fs, llineid line, Select *s, ltreeid x, int z) {
	ljlist js = {0};

	langL_jumpif(fs,&js,x,z);
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
	/* we don't need to close the previous
	block, it can just fall through to our
	branch, do collect all the other exit
	jumps and tie them to this branch block,
	naturally we don't need to add an exit
	jump since this is the last branch as
	defined in grammar, and even if it wasn't
	else and elif will close off this block */
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
	langL_jumpiftrue(fs,&js,x);
	LASSERT(js.t != 0);

	langL_tieloosejsto(fs,js.t,loop->e);
	langA_vardel(js.t);
	js.t = lnil;

	langL_tieloosejs(fs,js.f);
	langA_vardel(js.f);
	js.f = lnil;
}


void langL_beginwhile(FileState *fs, llineid line, Loop *loop, ltreeid x) {

	loop->e = langL_getlabel(fs);

	ljlist js = {lnil};
	langL_jumpiffalse(fs,&js,x);
	langL_tieloosejs(fs,js.t);
	langA_vardel(js.t);
	js.t = lnil;

	loop->x = x;
	loop->f = js.f;
}


void langL_closewhile(FileState *fs, llineid line, Loop *loop) {
	langL_jump(fs,line,loop->e);

	langL_tieloosejs(fs,loop->f);
	langA_vardel(loop->f);
	loop->f = lnil;
}


void langL_beginloop(FileState *fs, llineid line, Loop *loop, ltreeid x, ltreeid r) {
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

	loop->e = langL_getlabel(fs);

	ljlist js = {0};
	langL_jumpiffalse(fs,&js,c);
	LASSERT(js.f != 0);

	langL_tieloosejs(fs,js.t);
	langA_vardel(js.t);

	loop->x = x;
	loop->f = js.f;
}


void langL_closeloop(FileState *fs, llineid line, Loop *loop) {
	int x = loop->x;

	int k = langY_treexy(fs,NO_LINE,Y_ADD,x,langY_treelongint(fs,NO_LINE,1));
	langL_loadinto(fs,line,x,k);

	langL_jump(fs,line,loop->e);

	langL_tieloosejs(fs,loop->f);
	langA_vardel(loop->f);
	loop->f = 0;
}


/* todo: merge with aljacent blocks */
void langL_begindelayedblock(FileState *fs, llineid line, CodeBlock *d) {
	d->j = langL_byte(fs,line,BC_DELAY,NO_JUMP);
}


void langL_closedelayedblock(FileState *fs, llineid line, CodeBlock *bl) {
	// langX_error(fs,line,"closed block, %i",langL_getlocallabel(fs));

	FileFunc *fn = fs->fn;

	langL_byte(fs,line,BC_LEAVE,0);
	langL_tieloosej(fs,bl->j);
}


int byteeffect(lModule *md, lBytecode b) {
	switch (b.k) {
		/* -- should have already been allocated */
		case BC_LOADNUM:
		case BC_LOADINT:
		case BC_LOADNIL:
		/* -------------------- */
		case BC_HALT:
		case BC_J:
		case BC_JZ:
		case BC_JNZ:
		case BC_DELAY:
		case BC_LEAVE:
		case BC_YIELD: {
			return 0;
		}
		case BC_TABLE:
		case BC_NIL:
		case BC_NUM:
		case BC_INT: {
			return -0+1;
		}
		case BC_ISNIL: {
			return -1+1;
		}
		case BC_NEQ:
		case BC_EQ:
		case BC_LT:
		case BC_LTEQ:
		case BC_ADD:
		case BC_SUB:
		case BC_DIV:
		case BC_MUL:
		case BC_MOD:
		case BC_SHL:
		case BC_SHR:
		case BC_XOR: {
			return -2+1;
		}
		case BC_DUPL: {
			return b.i;
		}
		case BC_DROP: {
			return -1;
		}

		case BC_CALL: {
			return -b.x;
		}
		case BC_METACALL: {
			return -2 - b.x;
		}
		case BC_TABLECALL: {
			return -2 - b.x;
		}
		case BC_LOADFILE:
		case BC_LOADCLIB: {
			return -1;
		}

		case BC_STKGET: {
			return -1+1;
		}
		case BC_STKLEN: {
			return -0+1;
		}
		case BC_LOADGLOBAL:
		case BC_LOADCACHED:
		case BC_INDEX: {
			return 1;
		}
		case BC_FIELD: {
			return -2+1;
		}
		case BC_SETINDEX:
		case BC_SETFIELD: {
			return -3+0;
		}
		case BC_RELOAD:
		case BC_SETGLOBAL: {
			return -1+0;
		}
		case BC_CLOSURE: {
			/* todo?: could also encode this in the byte? */
			return - md->p[b.i].ncaches;
		}
	}

	LNOBRANCH;
	return 0;
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
	return Y_NONE;
}
/*
** See Copyright Notice In lang.h
** (L) lcode.c
** (L)ang Code Generation
*/


llocalid langL_localalloc(FileState *fs, char *line, llocalid nlocals) {
	FileFunc *ff = fs->fn;
	llocalid id = fs->nlocals;
	fs->nlocals += nlocals;
	LASSERT(fs->nlocals >= ff->locals);

	nlocals = fs->nlocals - ff->locals;
	if (ff->nlocals < nlocals) ff->nlocals = nlocals;

	/* this should work with negative numbers too */
	langA_variadd(fs->locals,nlocals);

	if (nlocals > 0) {
		for (llocalid i = id; i < fs->nlocals; ++i) {
			fs->locals[i].level = fs->level;
			fs->locals[i].line  = line;
			fs->locals[i].name  = 0;
			fs->locals[i].node  = NO_NODE;
			fs->locals[i].enm   = lfalse;
		}
	}

	// langX_error(fs,line,"%i, fs=%i, ff=%i",id,fs->nlocals,nlocals);
	return id - ff->locals;
}


int langL_isjump(ByteName n) {
	return n == BYTE_J || n == BYTE_JZ || n == BYTE_JNZ;
}


/*
** Turns an absolute jump target into
** a local one, relative to the current
** function.
*/
lbyteid langL_localizej(FileState *fs, lbyteid j) {
	if (j == NO_JUMP) return NO_JUMP;
	LASSERT(j >= fs->fn->bytes);
	return j - fs->fn->bytes;
}


/* todo: deprecated */
lbyteid langL_getlabel(FileState *fs) {
	return fs->md->nbytes;
}


lbyteid langL_getlocallabel(FileState *fs) {
	return langL_localizej(fs,langL_getlabel(fs));
}


lbyteid langL_addbyte(FileState *fs, char *line, Bytecode byte) {
	// lang_loginfo("%s %lli: byte: %s, %lli"
	// , fs->filename,fs->md->nbytes,lang_bytename(byte.k),byte.i);
	langA_varadd(fs->md->lines,line);
	langA_varadd(fs->md->bytes,byte);
	return fs->md->nbytes ++;
}


lbyteid langL_byte(FileState *fs, char *line, ByteName k, llong i) {
	return langL_addbyte(fs,line,(Bytecode){k,i});
}


lbyteid langL_bytexy(FileState *fs, char *line, ByteName k, int x, int y) {
	Bytecode b = (Bytecode){k};
	b.x = x;
	b.y = y;
	return langL_addbyte(fs,line,b);
}


lbyteid langL_jump(FileState *fs, char *line, lbyteid j) {
	return langL_byte(fs,line,BYTE_J,langL_localizej(fs,j));
}


void langL_tieloosejto(FileState *fs, lbyteid i, lbyteid j) {
	LASSERT (i != NO_JUMP);
	lModule *md = fs->md;
	Bytecode *bytes = md->bytes;
	Bytecode b = bytes[i];

	lbyteid l = langL_localizej(fs,j);
	switch (b.k) {
		case BYTE_J:
		case BYTE_JZ:
		case BYTE_JNZ:
		case BYTE_DELAY: {
			bytes[i].i = l;
		} break;
		case BYTE_YIELD: {
			bytes[i].x = l;
		} break;
		default: LNOCHANCE;
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


void langL_yield(FileState *fs, char *line, lbyteid x) {
	/* todo: add support for multiple results */
	int y = 0;
	if (x != NO_NODE) y = langL_loadall(fs,line,x);
	if (fs->fn->nyield < y) fs->fn->nyield = y;
	lbyteid j = langL_bytexy(fs,line,BYTE_YIELD,NO_JUMP,y);
	langA_varadd(fs->fn->yj,j);
}


void langL_return(FileState *fs, char *line) {
	lModule *md = fs->md;
	FileFunc *fn = fs->fn;

	/* Todo: this is unncessary, we know were
	the return instruction is at.
	collect all yields and tie them here */
	langL_tieloosejs(fs,fn->yj);
	langA_vardel(fn->yj);
	fn->yj = lnil;

	/* finally, return control flow */
	langL_byte(fs,line,BYTE_LEAVE,0);
}


/*
**
**
*/
lbyteid langL_jumpif(FileState *fs, ljlist *js, lnodeid x, int z) {
	Node v = fs->nodes[x];
	lbyteid j = NO_JUMP;
	switch (v.k) {
		case NODE_LOG_AND: {
			langL_jumpif(fs,js,v.x,0);
			langL_tieloosejs(fs,js->t);
			langA_vardel(js->t);
			js->t = 0;
			j = langL_jumpif(fs,js,v.y,z);
		} break;
		case NODE_LOG_OR: {
			langL_jumpif(fs,js,v.x,1);
			langL_tieloosejs(fs,js->f);
			langA_vardel(js->f);
			js->f = 0;
			j = langL_jumpif(fs,js,v.y,z);
		} break;
		default: {
			langL_load2(fs,v.line,x,1);

			if (z != 0) {
				j = langL_byte(fs,v.line,BYTE_JNZ,NO_JUMP);
				langA_varadd(js->t,j);
			} else {
				j = langL_byte(fs,v.line,BYTE_JZ,NO_JUMP);
				langA_varadd(js->f,j);
			}
		} break;
	}

	return j;
}


lbyteid langL_jumpiffalse(FileState *fs, ljlist *js, lnodeid x) {
	return langL_jumpif(fs,js,x,0);
}


lbyteid langL_jumpiftrue(FileState *fs, ljlist *js, lnodeid x) {
	return langL_jumpif(fs,js,x,1);
}


lbyteid langL_push(FileState *fs, char *line, lbyteid n) {
	if (n == 0) return NO_BYTE;
	return langL_byte(fs,line,BYTE_NIL,n);
}


lbyteid langL_drop(FileState *fs, char *line, lbyteid n) {
	return langL_byte(fs,line,BYTE_DROP,n);
}


lbyteid langL_dupl(FileState *fs, char *line, lbyteid n) {
	return langL_byte(fs,line,BYTE_DUPL,n);
}


void langL_loaddrop(FileState *fs, char *line, lnodeid x) {
	langL_load2(fs,line,x,0);
}


int langL_loadall(FileState *fs, char *line, lnodeid x) {
	langL_load2(fs,line,x,1);
	/* todo: get this from the node */
	return 1;
}


void langL_load2(FileState *fs, char *line, lnodeid x, llocalid y) {
	Node v = fs->nodes[x];
	LASSERT(x >= 0);
	LASSERT(v.level <= fs->level);
	if (line == 0) line = v.line;

	switch (v.k) {
		case NODE_GLOBAL: {
			if (y == 0) return;
			langL_byte(fs,line,BYTE_GLOBAL,v.x);
		} break;
		case NODE_CACHE: {
			if (y == 0) return;
			langL_byte(fs,line,BYTE_CACHE,v.x);
		} break;
		case NODE_LOCAL: {
			if (y == 0) return;
			langL_byte(fs,line,BYTE_LOCAL,v.x);
		} break;
		case NODE_GROUP: {
			langL_load2(fs,line,v.x,y);
		} break;
		case NODE_NIL: {
			langL_byte(fs,line,BYTE_NIL,1);
		} break;
		case NODE_INTEGER: {
			langL_byte(fs,line,BYTE_INT,v.lit.i);
		} break;
		case NODE_NUMBER: {
			langL_byte(fs,line,BYTE_NUM,v.lit.i);
		} break;
		case NODE_STRING: {
			/* todo: allocate this in constant pool */
			int g = lang_addglobal(fs->md,0,lang_S(langS_new(fs->rt,v.lit.s)));

			langL_byte(fs,line,BYTE_GLOBAL,g);
		} break;
		case NODE_FUNCTION: {
			langA_varifor(v.z) {
				langL_load2(fs,NO_LINE,v.z[i],1);
			}
			langL_byte(fs,line,BYTE_CLOSURE,v.x);
		} break;
		case NODE_TABLE: {
			LASSERT(v.line != 0);
			/* table is left on the stack */
			langL_byte(fs,line,BYTE_TABLE,0);
			langA_varifor(v.z) {
				lnodeid z = v.z[i];
				LASSERT(z != x);
				Node q = fs->nodes[z];

				/* todo: this is temporary */
				if (q.k == NODE_DESIG) {
					langL_load2(fs,q.line,q.x,1);
					langL_load2(fs,q.line,q.y,1);
					langL_byte(fs,q.line,BYTE_SETFIELD,0);
				} else {
					langL_byte(fs,q.line,BYTE_INT,i);
					langL_load2(fs,q.line,z,1);
					langL_byte(fs,q.line,BYTE_SETINDEX,0);
				}
			}
		} break;
		case NODE_FIELD: {
			langL_load2(fs,NO_LINE,v.x,1);
			langL_load2(fs,NO_LINE,v.y,1);
			langL_byte(fs,line,BYTE_FIELD,0);
		} break;
		case NODE_INDEX: {
			langL_load2(fs,NO_LINE,v.x,1);
			langL_load2(fs,NO_LINE,v.y,1);
			langL_byte(fs,line,BYTE_INDEX,0);
		} break;
		case NODE_BUILTIN: {
			/* host the result registers */
			langL_push(fs,line,y);
			int n = langA_varlen(v.z);
			langA_varifor(v.z) {
				langL_load2(fs,NO_LINE,v.z[i],1);
			}
			if (v.x == TK_STKLEN) {
				langL_bytexy(fs,line,BYTE_STKLEN,n,y);
			} else
			if (v.x == TK_STKGET) {
				langL_bytexy(fs,line,BYTE_STKGET,n,y);
			} else LNOCHANCE;
		} break;
		case NODE_LOADFILE: {
			/* host the result registers */
			langL_push(fs,line,y);
			langL_load2(fs,line,v.x,1);
			langL_bytexy(fs,line,BYTE_LOADFILE,0,y);
		} break;
		case NODE_MCALL: {
			/* host the result registers */
			langL_push(fs,line,y);
			int n = langA_varlen(v.z);
			langA_varifor(v.z) {
				langL_load2(fs,NO_LINE,v.z[i],1);
			}
			langL_load2(fs,NO_LINE,v.x,1);
			langL_load2(fs,NO_LINE,v.y,1);
			langL_bytexy(fs,line,BYTE_METACALL,n,y);
		} break;
		case NODE_CALL: {
			/* host the result registers */
			langL_push(fs,line,y);
			llocalid n = langA_varlen(v.z);
			langA_varifor(v.z) {
				langL_load2(fs,NO_LINE,v.z[i],1);
			}
			langL_load2(fs,line,v.x,1);
			langL_bytexy(fs,line,BYTE_CALL,n,y);
		} break;
		case NODE_LOG_AND:
		case NODE_LOG_OR: {
			ljlist js = {0};
			langL_jumpiffalse(fs,&js,x);
			langL_tieloosejs(fs,js.t);
			langA_vardel(js.t);
			js.t = lnil;
			langL_byte(fs,line,BYTE_INT,ltrue);
			int j = langL_jump(fs,line,-1);
			langL_tieloosejs(fs,js.f);
			langA_vardel(js.f);
			js.f = lnil;
			langL_byte(fs,line,BYTE_INT,lfalse);
			langL_tieloosej(fs,j);
		} break;
		/* todo: make these more compact! */
		case NODE_GT: {
			langL_load2(fs,line,v.y,1);
			langL_load2(fs,line,v.x,1);
			langL_byte(fs,line,BYTE_LT,0);
		} break;
		case NODE_GTEQ: {
			langL_load2(fs,line,v.y,1);
			langL_load2(fs,line,v.x,1);
			langL_byte(fs,line,BYTE_LTEQ,0);
		} break;
		case NODE_LT: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_LT,0);
		} break;
		case NODE_LTEQ: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_LTEQ,0);
		} break;
		case NODE_EQ: {
			if (fs->nodes[v.y].k == NODE_NIL) {
				langL_load2(fs,line,v.x,1);
				langL_byte(fs,line,BYTE_ISNIL,0);
			} else {
				langL_load2(fs,line,v.x,1);
				langL_load2(fs,line,v.y,1);
				langL_byte(fs,line,BYTE_EQ,0);
			}
		} break;
		case NODE_NEQ: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_NEQ,0);
		} break;
		case NODE_DIV: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_DIV,0);
		} break;
		case NODE_MUL: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_MUL,0);
		} break;
		case NODE_MOD: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_MOD,0);
		} break;
		case NODE_SUB: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_SUB,0);
		} break;
		case NODE_ADD: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_ADD,0);
		} break;
		case NODE_BSHL: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_SHL,0);
		} break;
		case NODE_BSHR: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_SHR,0);
		} break;
		case NODE_BXOR: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_XOR,0);
		} break;
		default: {
			langX_error(fs,v.line,"unsupported node");
			LNOCHANCE;
		} break;
	}
}


void langL_loadinto(FileState *fs, char *line, lnodeid x, lnodeid y) {
	Node v = fs->nodes[x];
	LASSERT(v.level <= fs->level);
	LASSERT(x >= 0);
	LASSERT(y >= 0);
	if (line == 0) line = v.line;

	switch (v.k) {
		case NODE_GLOBAL: {
			langL_load2(fs,line,y,1);
			langL_byte(fs,line,BYTE_SETGLOBAL,v.x);
		} break;
		case NODE_LOCAL: {
			langL_load2(fs,line,y,1);
			langL_byte(fs,line,BYTE_SETLOCAL,v.x);
		} break;
		case NODE_INDEX: {
			langL_load2(fs,line,v.x,1);/* table */
			langL_load2(fs,line,v.y,1);/* index */
			langL_load2(fs,line,y,1);/* value */
			langL_byte(fs,line,BYTE_SETINDEX,0);
			langL_drop(fs,line,1);/* drop table */
		} break;
		case NODE_FIELD: {
			langL_load2(fs,line,v.x,1);/* table */
			langL_load2(fs,line,v.y,1);/* index */
			langL_load2(fs,line,y,1);/* value */
			langL_byte(fs,line,BYTE_SETFIELD,0);
			langL_drop(fs,line,1);/* drop table */
		} break;
		// {x}[{x}..{x}] = {y}
		case NODE_RANGE_INDEX: {
			Loop l = {0};
			langL_load2(fs,line,v.x,1);/* table */
			langL_beginloop(fs,line,&l,-1,v.y);
			langL_load2(fs,line,l.x,1);/* index */
			langL_load2(fs,line,y,1);/* value */
			langL_byte(fs,line,BYTE_SETINDEX,0);
			langL_closeloop(fs,line,&l);
			langL_drop(fs,line,1);/* drop table */
		} break;
		default: {
			LNOCHANCE;
		} break;
	}
}


void langL_beginif(FileState *fs, char *line, Select *s, lnodeid x, int z) {
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
void langL_addelse(FileState *fs, char *line, Select *s) {
	LASSERT(s->jz != 0);
	int j = langL_jump(fs,line,-1);
	langA_varadd(s->j,j);

	langL_tieloosejs(fs,s->jz);
	langA_vardel(s->jz);
	s->jz = lnil;
}


void langL_addelif(FileState *fs, char *line, Select *s, int x) {
	langL_addelse(fs,line,s);
	langL_beginif(fs,line,s,x,L_IF);
}


void langL_addthen(FileState *fs, char *line, Select *s) {
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


void langL_closeif(FileState *fs, char *line, Select *s) {
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


void langL_begindowhile(FileState *fs, char *line, Loop *loop) {
	loop->e = langL_getlabel(fs);
	loop->x = NO_NODE;
	loop->f = lnil;
}


void langL_closedowhile(FileState *fs, char *line, Loop *loop, lnodeid x) {

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


void langL_beginwhile(FileState *fs, char *line, Loop *loop, lnodeid x) {

	loop->e = langL_getlabel(fs);

	ljlist js = {lnil};
	langL_jumpiffalse(fs,&js,x);
	langL_tieloosejs(fs,js.t);
	langA_vardel(js.t);
	js.t = lnil;

	loop->x = x;
	loop->f = js.f;
}


void langL_closewhile(FileState *fs, char *line, Loop *loop) {
	langL_jump(fs,line,loop->e);

	langL_tieloosejs(fs,loop->f);
	langA_vardel(loop->f);
	loop->f = lnil;
}


void langL_beginloop(FileState *fs, char *line, Loop *loop, int x, int r) {
	LASSERT(fs->nodes[r].k == NODE_RANGE);
	/* todo: */
	if (x == -1) {
		int v = lang_addglobal(fs->md,langS_new(fs->rt,"$temp"),(lValue){VALUE_NONE});
		x = langY_globalnode(fs,line,v);
	}

	int lo = fs->nodes[r].x;
	int hi = fs->nodes[r].y;
	/* emit code to initialize x to lower range
	and then compare against upper range, if false
	emit code to jump, the jump is patched when the
	loop is closed */
	langL_loadinto(fs,line,x,lo);

	int c = langY_nodexy(fs,NO_LINE,NODE_LT,x,hi);

	loop->e = langL_getlabel(fs);

	ljlist js = {0};
	langL_jumpiffalse(fs,&js,c);
	LASSERT(js.f != 0);

	langL_tieloosejs(fs,js.t);
	langA_vardel(js.t);

	loop->x = x;
	loop->f = js.f;
}


void langL_closeloop(FileState *fs, char *line, Loop *loop) {
	int x = loop->x;

	int k = langY_nodexy(fs,NO_LINE,NODE_ADD,x,langY_nodeI(fs,NO_LINE,1));
	langL_loadinto(fs,line,x,k);

	langL_jump(fs,line,loop->e);

	langL_tieloosejs(fs,loop->f);
	langA_vardel(loop->f);
	loop->f = 0;
}


/* todo: merge with aljacent blocks */
void langL_begindelayedblock(FileState *fs, char *line, CodeBlock *d) {
	d->j = langL_byte(fs,line,BYTE_DELAY,NO_JUMP);
}


void langL_closedelayedblock(FileState *fs, char *line, CodeBlock *bl) {
	// langX_error(fs,line,"closed block, %i",langL_getlocallabel(fs));

	FileFunc *fn = fs->fn;

	langL_byte(fs,line,BYTE_LEAVE,0);
	langL_tieloosej(fs,bl->j);
}

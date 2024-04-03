/*
** See Copyright Notice In lang.h
** (L) lcode.c
** Bytecode Generator
*/


int byteeffect(lModule *md, lBytecode b);


/* todo: deprecated */
lbyteid langL_getlabel(FileState *fs) {
	return fs->md->nbytes;
}


llocalid langL_stkinctop(FileState *fs, llocalid inc) {
	FileFunc *fn = fs->fn;
	llocalid id = fn->stktop;
	fn->stktop += inc;
	if (fn->stklen < fn->stktop) fn->stklen = fn->stktop;
	return id;
}


lbyteid langL_addbyte(FileState *fs, llineid line, lBytecode byte) {
	FileFunc *fn = fs->fn;
	lModule *md = fs->md;
	langL_stkinctop(fs,byteeffect(md,byte));

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


void langL_tieloosejto(FileState *fs, lbyteid id, lbyteid j) {
	lModule *md = fs->md;
	lBytecode *bytes = md->bytes;
	lBytecode b = bytes[id];

	lbyteid l = j - id;
	// if (l == NO_JUMP) {
	// 	langX_error(fs,md->lines[id],"opt, no jump");
	// }
	switch (b.k) {
		case BYTE_J:
		case BYTE_JZ:
		case BYTE_JNZ:
		case BYTE_DELAY: {
			bytes[id].i = l;
		} break;
		case BYTE_YIELD: {
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
	return langL_byte(fs,line,BYTE_J,j-fs->md->nbytes);
}


void langL_yield(FileState *fs, llineid line, lbyteid x) {
	/* todo: add support for multiple results */
	int y = 0;
	if (x != NO_TREE) y = langL_loadall(fs,line,x);
	if (fs->fn->nyield < y) fs->fn->nyield = y;
	lbyteid j = langL_bytexy(fs,line,BYTE_YIELD,NO_JUMP,y);
	langA_varadd(fs->fn->yj,j);
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
	langL_byte(fs,line,BYTE_LEAVE,0);
}


/*
**
**
*/
lbyteid langL_jumpif(FileState *fs, ljlist *js, ltreeid x, lbool z) {
	Tree v = fs->nodes[x];
	lbyteid jid = NO_BYTE;
	switch (v.k) {
		case TREE_LOG_AND: {
			langL_jumpif(fs,js,v.x,0);
			langL_tieloosejs(fs,js->t);
			langA_vardel(js->t);
			js->t = 0;
			jid = langL_jumpif(fs,js,v.y,z);
		} break;
		case TREE_LOG_OR: {
			langL_jumpif(fs,js,v.x,1);
			langL_tieloosejs(fs,js->f);
			langA_vardel(js->f);
			js->f = 0;
			jid = langL_jumpif(fs,js,v.y,z);
		} break;
		default: {
			langL_load2(fs,v.line,x,1);
			if (z != 0) {
				jid = langL_byte(fs,v.line,BYTE_JNZ,NO_JUMP);
				langA_varadd(js->t,jid);
			} else {
				jid = langL_byte(fs,v.line,BYTE_JZ,NO_JUMP);
				langA_varadd(js->f,jid);
			}
		} break;
	}

	return jid;
}


lbyteid langL_jumpiffalse(FileState *fs, ljlist *js, ltreeid x) {
	return langL_jumpif(fs,js,x,0);
}


lbyteid langL_jumpiftrue(FileState *fs, ljlist *js, ltreeid x) {
	return langL_jumpif(fs,js,x,1);
}


lbyteid langL_push(FileState *fs, llineid line, lbyteid n) {
	if (n == 0) return NO_BYTE;
	return langL_byte(fs,line,BYTE_NIL,n);
}


lbyteid langL_drop(FileState *fs, llineid line, lbyteid n) {
	return langL_byte(fs,line,BYTE_DROP,n);
}


lbyteid langL_dupl(FileState *fs, llineid line, lbyteid n) {
	return langL_byte(fs,line,BYTE_DUPL,n);
}


void langL_loaddrop(FileState *fs, llineid line, ltreeid x) {
	langL_load2(fs,line,x,0);
}


int langL_loadall(FileState *fs, llineid line, ltreeid x) {
	langL_load2(fs,line,x,1);
	/* todo: get this from the node */
	return 1;
}


void langL_load2(FileState *fs, llineid line, ltreeid x, llocalid y) {
	Tree v = fs->nodes[x];
	LASSERT(x >= 0);
	LASSERT(v.level <= fs->level);
	if (line == 0) line = v.line;
	lModule *md = fs->md;

	switch (v.k) {
		case TREE_GLOBAL: {
			if (y == 0) return;
			langL_byte(fs,line,BYTE_GLOBAL,v.x);
		} break;
		case TREE_CACHE: {
			if (y == 0) return;
			langL_byte(fs,line,BYTE_CACHE,v.x);
		} break;
		case TREE_LOCAL: {
			if (y == 0) return;
			lBytecode *bytes = md->bytes;
			lbyteid nbytes = md->nbytes;
			lBytecode last = bytes[nbytes-1];
			/* silly opt experiment */
			if (last.k == BYTE_LOCAL) {
				if (last.x+last.y == v.x) {
					++ bytes[nbytes-1].y;
				} else goto _else;
			} else { _else:
				langL_bytexy(fs,line,BYTE_LOCAL,v.x,1);
			}
		} break;
		case TREE_GROUP: {
			langL_load2(fs,line,v.x,y);
		} break;
		case TREE_NIL: {
			langL_byte(fs,line,BYTE_NIL,1);
		} break;
		case TREE_INTEGER: {
			// llocalid loc = langL_stkinctop(fs,1);
			// langL_bytexy(fs,line,BYTE_LOADINT,loc,v.lit.i);

			langL_byte(fs,line,BYTE_INT,v.lit.i);
		} break;
		case TREE_NUMBER: {
			langL_byte(fs,line,BYTE_NUM,v.lit.i);
		} break;
		case TREE_STRING: {
			/* todo: allocate this in constant pool */
			int g = lang_addglobal(fs->md,0,lang_S(langS_new(fs->rt,v.lit.s)));

			langL_byte(fs,line,BYTE_GLOBAL,g);
		} break;
		case TREE_FUNCTION: {
			langA_varifor(v.z) {
				langL_load2(fs,NO_LINE,v.z[i],1);
			}
			langL_byte(fs,line,BYTE_CLOSURE,v.x);
		} break;
		case TREE_TABLE: {
			LASSERT(v.line != 0);
			/* table is left on the stack */
			langL_byte(fs,line,BYTE_TABLE,0);
			langA_varifor(v.z) {
				ltreeid z = v.z[i];
				LASSERT(z != x);
				Tree q = fs->nodes[z];

				/* todo: this is temporary */
				if (q.k == TREE_DESIG) {
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
		case TREE_FIELD: {
			langL_load2(fs,NO_LINE,v.x,1);
			langL_load2(fs,NO_LINE,v.y,1);
			langL_byte(fs,line,BYTE_FIELD,0);
		} break;
		case TREE_INDEX: {
			langL_load2(fs,NO_LINE,v.x,1);
			langL_load2(fs,NO_LINE,v.y,1);
			langL_byte(fs,line,BYTE_INDEX,0);
		} break;
		case TREE_BUILTIN: {
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
			} else LNOBRANCH;
		} break;
		case TREE_LOADFILE: {
			/* host the result registers */
			langL_push(fs,line,y);
			langL_load2(fs,line,v.x,1);
			langL_bytexy(fs,line,BYTE_LOADFILE,0,y);
		} break;
		case TREE_MCALL: {
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
		case TREE_CALL: {
			/* host the result registers */
			langL_push(fs,line,y);
			llocalid n = langA_varlen(v.z);
			langA_varifor(v.z) {
				langL_load2(fs,NO_LINE,v.z[i],1);
			}
			langL_load2(fs,line,v.x,1);
			langL_bytexy(fs,line,BYTE_CALL,n,y);
		} break;
		case TREE_LOG_AND:
		case TREE_LOG_OR: {
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
		case TREE_GT: {
			langL_load2(fs,line,v.y,1);
			langL_load2(fs,line,v.x,1);
			langL_byte(fs,line,BYTE_LT,0);
		} break;
		case TREE_GTEQ: {
			langL_load2(fs,line,v.y,1);
			langL_load2(fs,line,v.x,1);
			langL_byte(fs,line,BYTE_LTEQ,0);
		} break;
		case TREE_LT: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_LT,0);
		} break;
		case TREE_LTEQ: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_LTEQ,0);
		} break;
		case TREE_EQ: {
			if (fs->nodes[v.y].k == TREE_NIL) {
				langL_load2(fs,line,v.x,1);
				langL_byte(fs,line,BYTE_ISNIL,0);
			} else {
				langL_load2(fs,line,v.x,1);
				langL_load2(fs,line,v.y,1);
				langL_byte(fs,line,BYTE_EQ,0);
			}
		} break;
		case TREE_NEQ: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_NEQ,0);
		} break;
		case TREE_DIV: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_DIV,0);
		} break;
		case TREE_MUL: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_MUL,0);
		} break;
		case TREE_MOD: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_MOD,0);
		} break;
		case TREE_SUB: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_SUB,0);
		} break;
		case TREE_ADD: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_ADD,0);
		} break;
		case TREE_BSHL: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_SHL,0);
		} break;
		case TREE_BSHR: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_SHR,0);
		} break;
		case TREE_BXOR: {
			langL_load2(fs,line,v.x,1);
			langL_load2(fs,line,v.y,1);
			langL_byte(fs,line,BYTE_XOR,0);
		} break;
		default: {
			langX_error(fs,v.line,"unsupported node");
			LNOBRANCH;
		} break;
	}
}


void langL_loadinto(FileState *fs, llineid line, ltreeid x, ltreeid y) {
	Tree v = fs->nodes[x];
	LASSERT(v.level <= fs->level);
	LASSERT(x >= 0);
	LASSERT(y >= 0);
	if (line == 0) line = v.line;

	switch (v.k) {
		case TREE_GLOBAL: {
			langL_load2(fs,line,y,1);
			langL_byte(fs,line,BYTE_SETGLOBAL,v.x);
		} break;
		case TREE_LOCAL: {
			langL_load2(fs,line,y,1);
			langL_byte(fs,line,BYTE_SETLOCAL,v.x);
		} break;
		case TREE_INDEX: {
			langL_load2(fs,line,v.x,1);/* table */
			langL_load2(fs,line,v.y,1);/* index */
			langL_load2(fs,line,y,1);/* value */
			langL_byte(fs,line,BYTE_SETINDEX,0);
			langL_drop(fs,line,1);/* drop table */
		} break;
		case TREE_FIELD: {
			langL_load2(fs,line,v.x,1);/* table */
			langL_load2(fs,line,v.y,1);/* index */
			langL_load2(fs,line,y,1);/* value */
			langL_byte(fs,line,BYTE_SETFIELD,0);
			langL_drop(fs,line,1);/* drop table */
		} break;
		// {x}[{x}..{x}] = {y}
		case TREE_RANGE_INDEX: {
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
	LASSERT(fs->nodes[r].k == TREE_RANGE);
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

	int c = langY_treexy(fs,NO_LINE,TREE_LT,x,hi);

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

	int k = langY_treexy(fs,NO_LINE,TREE_ADD,x,langY_treelongint(fs,NO_LINE,1));
	langL_loadinto(fs,line,x,k);

	langL_jump(fs,line,loop->e);

	langL_tieloosejs(fs,loop->f);
	langA_vardel(loop->f);
	loop->f = 0;
}


/* todo: merge with aljacent blocks */
void langL_begindelayedblock(FileState *fs, llineid line, CodeBlock *d) {
	d->j = langL_byte(fs,line,BYTE_DELAY,NO_JUMP);
}


void langL_closedelayedblock(FileState *fs, llineid line, CodeBlock *bl) {
	// langX_error(fs,line,"closed block, %i",langL_getlocallabel(fs));

	FileFunc *fn = fs->fn;

	langL_byte(fs,line,BYTE_LEAVE,0);
	langL_tieloosej(fs,bl->j);
}


int byteeffect(lModule *md, lBytecode b) {
	switch (b.k) {
		/* -- should have already been allocated */
		case BYTE_LOADNUM:
		case BYTE_LOADINT:
		case BYTE_LOADNIL:
		/* -------------------- */
		case BYTE_HALT:
		case BYTE_J:
		case BYTE_JZ:
		case BYTE_JNZ:
		case BYTE_DELAY:
		case BYTE_LEAVE:
		case BYTE_YIELD: {
			return 0;
		}
		case BYTE_TABLE:
		case BYTE_NIL:
		case BYTE_NUM:
		case BYTE_INT: {
			return -0+1;
		}
		case BYTE_ISNIL: {
			return -1+1;
		}
		case BYTE_NEQ:
		case BYTE_EQ:
		case BYTE_LT:
		case BYTE_LTEQ:
		case BYTE_ADD:
		case BYTE_SUB:
		case BYTE_DIV:
		case BYTE_MUL:
		case BYTE_MOD:
		case BYTE_SHL:
		case BYTE_SHR:
		case BYTE_XOR: {
			return -2+1;
		}
		case BYTE_DUPL: {
			return b.i;
		}
		case BYTE_DROP: {
			return -1;
		}

		case BYTE_CALL: {
			return -b.x;
		}
		case BYTE_METACALL: {
			return -2 - b.x;
		}
		case BYTE_TABLECALL: {
			return -2 - b.x;
		}
		case BYTE_LOADFILE:
		case BYTE_LOADCLIB: {
			return -1;
		}

		case BYTE_STKGET: {
			return -1+1;
		}
		case BYTE_STKLEN: {
			return -0+1;
		}
		case BYTE_LOCAL: {
			return b.y;
		}
		case BYTE_GLOBAL:
		case BYTE_CACHE:
		case BYTE_INDEX: {
			return 1;
		}
		case BYTE_FIELD: {
			return -2+1;
		}
		case BYTE_SETINDEX:
		case BYTE_SETFIELD: {
			return -3+0;
		}
		case BYTE_SETLOCAL:
		case BYTE_SETGLOBAL: {
			return -1+0;
		}
		case BYTE_CLOSURE: {
			/* todo?: could also encode this in the byte? */
			return - md->p[b.i].ncaches;
		}
	}

	LNOBRANCH;
	return 0;
}
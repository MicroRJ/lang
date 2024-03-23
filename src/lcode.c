/*
** See Copyright Notice In lang.h
** lcode.c
** (C) Code Generation
*/


int langC_isjump(ByteName n) {
	return n == BYTE_J || n == BYTE_JZ || n == BYTE_JNZ;
}


int langC_byte(FileState *fs, char *loc, ByteName k, Integer i) {
	// lang_loginfo("%s %lli: byte: %s, %lli"
	// , fs->filename,fs->md->nbytes,lang_bytename(k),i);
	Bytecode *b = langA_varnadd(fs->md->bytes,1);
	b->k = k;
	b->i = i;
	langA_varadd(fs->md->lineinfo,loc);
	LASSERT(sizeof(b->i) == sizeof(void*));
	return fs->md->nbytes ++;
}


int langC_fixjump(FileState *fs, int j) {
	/* jump instructions are relative to
	the current file and function.
	todo?: I guess we could just make the
	file a function? */
	if (j == NO_JUMP) return NO_JUMP;
	if (fs->fn != 0) j -= fs->fn->bytes;
	LASSERT(j >= 0);
	return j;
}


int langC_jump(FileState *fs, char *loc, int j) {
	return langC_byte(fs,loc,BYTE_J,langC_fixjump(fs,j));
}


int langC_pop(FileState *fs, char *loc) {
	return langC_byte(fs,loc,BYTE_POP,0);
}


int langC_dup(FileState *fs, char *loc) {
	return langC_byte(fs,loc,BYTE_DUP,0);
}


void langC_leave(FileState *fs, char *loc, int x) {
	/* todo: can we use this to figure out the number
	of returns of a function? */
	langC_load(fs,loc,x);
	langC_byte(fs,loc,BYTE_RET,1);
}


void langC_patchjtohere(FileState *fs, int i) {
	LASSERT (i != NO_JUMP);
	fs->md->bytes[i].i = langC_fixjump(fs,fs->md->nbytes);
}


void langC_patchjstohere(FileState *fs, int *js) {
	langA_varfor(int *, j, js) {
		langC_patchjtohere(fs,*j);
	}
}


int langC_jumpif(FileState *fs, Boolean *js, NodeId x, int z) {
	Node v = fs->nodes[x];
	int j = NO_JUMP;
	switch (v.k) {
		case NODE_LOG_AND: {
			langC_jumpif(fs,js,v.x,0);
			langC_patchjstohere(fs,js->t);
			langA_vardel(js->t);
			js->t = 0;
			j = langC_jumpif(fs,js,v.y,z);
		} break;
		case NODE_LOG_OR: {
			langC_jumpif(fs,js,v.x,1);
			langC_patchjstohere(fs,js->f);
			langA_vardel(js->f);
			js->f = 0;
			j = langC_jumpif(fs,js,v.y,z);
		} break;
		default: {
			langC_load(fs,v.line,x);

			if (z != 0) {
				j = langC_byte(fs,v.line,BYTE_JNZ,NO_JUMP);
				langA_varadd(js->t,j);
			} else {
				j = langC_byte(fs,v.line,BYTE_JZ,NO_JUMP);
				langA_varadd(js->f,j);
			}
		} break;
	}

	return j;
}


int langC_jumpiffalse(FileState *fs, Boolean *js, int x) {
	return langC_jumpif(fs,js,x,0);
}


int langC_jumpiftrue(FileState *fs, Boolean *js, int x) {
	return langC_jumpif(fs,js,x,1);
}


void langC_load(FileState *fs, char *line, NodeId x) {
	LASSERT(x != -1);

	Node v = fs->nodes[x];
	if (v.level > fs->level) {
		langX_error(fs,v.line,"node was freed");
	}
	LASSERT(v.level <= fs->level);

	if (line == 0) line = v.line;


	switch (v.k) {
		case NODE_GLOBAL: {
			langC_byte(fs,line,BYTE_GGET,v.x);
		} break;
		case NODE_CACHE: {
			langC_byte(fs,line,BYTE_UGET,v.x);
		} break;
		case NODE_LOCAL: {
			langC_byte(fs,line,BYTE_LGET,v.x);
		} break;
		case NODE_INTEGER: {
			langC_byte(fs,line,BYTE_INT,v.lit.i);
		} break;
		case NODE_LOADFILE: {
			langC_load(fs,0,v.x);
			langC_byte(fs,line,BYTE_LOADFILE,0);
		} break;
		case NODE_STRING: {
			/* todo: allocate this in constant pool */
			int g = lang_addglobal(fs->md,0,lang_S(langS_new(fs->rt,v.lit.s)));
			langC_byte(fs,line,BYTE_GGET,g);
		} break;
		case NODE_FUNCTION: {
			langA_varfor(int *, u, v.z) {
				langC_load(fs,v.line,*u);
			}
			langC_byte(fs,line,BYTE_FNEW,v.x);
		} break;
		case NODE_TABLE: {
			LASSERT(v.line != 0);
			langC_byte(fs,line,BYTE_TNEW,0);
			langA_varifor(v.z) {
				NodeId z = v.z[i];
				Node q = fs->nodes[z];
				langC_dup(fs,q.line);

				if (q.k == NODE_FIELD) {
					/* todo: check for fields */
					LNOCHANCE;
				} else {
					langC_byte(fs,q.line,BYTE_INT,i);
					langC_load(fs,q.line,z);
					langC_byte(fs,q.line,BYTE_TSET,0);
				}
			}
		} break;
		case NODE_FIELD: {
			langC_load(fs,0,v.x);
			langC_load(fs,0,v.y);
			langC_byte(fs,line,BYTE_TGET,0);
		} break;
		case NODE_MCALL: {
			langA_varfor (int *, h, v.z) {
				langC_load(fs,0,*h);
			}
			langC_load(fs,0,v.x);
			langC_load(fs,0,v.y);
			langC_byte(fs,line,BYTE_MCALL,langA_varlen(v.z));
		} break;
		case NODE_CALL: {
			langA_varfor (int *, h, v.z) {
				langC_load(fs,0,*h);
			}
			langC_load(fs,line,v.x);
			langC_byte(fs,line,BYTE_CALL,langA_varlen(v.z));
		} break;
		case NODE_LOG_AND:
		case NODE_LOG_OR: {
			Boolean js = {0};
			langC_jumpiffalse(fs,&js,x);
			langC_patchjstohere(fs,js.t);
			langA_vardel(js.t);
			js.t = Null;
			langC_byte(fs,line,BYTE_INT,True);
			int j = langC_jump(fs,line,-1);
			langC_patchjstohere(fs,js.f);
			langA_vardel(js.f);
			js.f = Null;
			langC_byte(fs,line,BYTE_INT,False);
			langC_patchjtohere(fs,j);
		} break;
		case NODE_LT:
		case NODE_EQ:
		case NODE_NEQ:
		case NODE_DIV:
		case NODE_MUL:
		case NODE_MOD:
		case NODE_SUB:
		case NODE_ADD: {
			langC_load(fs,line,v.x);
			langC_load(fs,line,v.y);
			langC_byte(fs,line,v.k,0);
		} break;
		default: {
			lang_logerror("unsupported node");
			__debugbreak();
		} break;
	}
}


void langC_store(FileState *fs, char *line, int x, int y) {
	Node v = fs->nodes[x];
	LASSERT(v.level <= fs->level);
	if (line == 0) {
		line = v.line;
	}
	switch (v.k) {
		case NODE_GLOBAL: {
			langC_load(fs,line,y);
			langC_byte(fs,line,BYTE_GSET,v.x);
		} break;
		case NODE_LOCAL: {
			langC_load(fs,line,y);
			langC_byte(fs,line,BYTE_LSET,v.x);
		} break;
		case NODE_FIELD: {
			langC_load(fs,line,v.x);
			langC_load(fs,line,v.y);
			langC_load(fs,line,y);
			langC_byte(fs,line,BYTE_TSET,0);
		} break;
		// {x}[{x}..{x}] = {y}
		case NODE_RANGE_INDEX: {
			/* todo: emit code to check number of rhs values */
			#if 0
			langC_load(fs,y);
			Loop l = {0};
			langC_beginloop(fs,&l,-1,v.y);
			langC_dup(fs);
			langC_load(fs,v.x);
			langC_load(fs,l.x);
			langC_byte(fs,BYTE_TSET,0);
			langC_closeloop(fs,&l);
			#endif
			LNOCHANCE;
		} break;
		default: {
			LNOCHANCE;
		} break;
	}
}


/*
**
**
*/
void langC_beginif(FileState *fs, char *line, Select *s, int x) {
	Boolean js = {0};
	langC_jumpiffalse(fs,&js,x);
	LASSERT(js.f != 0);

	langC_patchjstohere(fs,js.t);
	langA_vardel(js.t);
	js.t = 0;

	/* will be patched when the block is closed */
	s->jz = js.f;
}


/*
** Closes previous conditional block by emitting
** escape jump, patches previous jz list to
** enter this block.
*/
void langC_addelse(FileState *fs, char *line, Select *s) {
	LASSERT(s->jz != 0);
	int j = langC_jump(fs,line,-1);
	langA_varadd(s->j,j);
	langC_patchjstohere(fs,s->jz);
	langA_vardel(s->jz);
	s->jz = Null;
}


void langC_addelif(FileState *fs, char *line, Select *s, int x) {
	langC_addelse(fs,line,s);
	langC_beginif(fs,line,s,x);
}


void langC_closeif(FileState *fs, char *line, Select *s) {
	/* check this in case we didn't add an else
	statement */
	if (s->jz != 0) {
		langC_patchjstohere(fs,s->jz);
	}

	langC_patchjstohere(fs,s->j);

	langA_vardel(s->j);
	s->j = 0;
}


void langC_beginloop(FileState *fs, char *line, Loop *loop, int x, int r) {
	LASSERT(fs->nodes[r].k == NODE_RANGE);
	/* todo: */
	if (x == -1) {
		int v = lang_addglobal(fs->md,langS_new(fs->rt,"$temp"),(Value){VALUE_NONE});
		x = langY_globalnode(fs,line,v);
	}

	int lo = fs->nodes[r].x;
	int hi = fs->nodes[r].y;
	/* emit code to initialize x to lower range
	and then compare against upper range, if false
	emit code to jump, the jump is patched when the
	loop is closed */
	langC_store(fs,line,x,lo);

	int c = langY_node2(fs,NO_LOC,NODE_LT,x,hi);

	loop->e = fs->md->nbytes;

	Boolean js = {0};
	langC_jumpiffalse(fs,&js,c);
	LASSERT(js.f != 0);

	langC_patchjstohere(fs,js.t);
	langA_vardel(js.t);

	loop->x = x;
	loop->f = js.f;
}


void langC_closeloop(FileState *fs, char *line, Loop *loop) {
	int x = loop->x;

	int k = langY_node2(fs,NO_LOC,NODE_ADD,x,langY_nodeI(fs,NO_LOC,1));
	langC_store(fs,line,x,k);

	langC_jump(fs,line,loop->e);

	langC_patchjstohere(fs,loop->f);
	langA_vardel(loop->f);
	loop->f = 0;
}

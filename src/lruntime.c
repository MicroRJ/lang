/*
** See Copyright Notice In lang.h
** lruntime.c
** Runtime
*/


lnumber ltonumber(lValue v) {
	return v.tag == TAG_INTEGER ? (lnumber) v.i : v.n;
}


llongint ltolong(lValue v) {
	return v.tag == TAG_NUMBER ? (llongint) v.n : v.i;
}


/* imagine being fast */
lBinding lang_fndmetafield(lObject *j, lString *name) {
	for (int i = 0; i < j->_n; i ++) {
		if (S_eq(j->_m[i].name,name->c)) {
			return j->_m[i].c;
		}
	}
	return lnil;
}


int fndfile(lModule *md, llineid line) {
	lFile *files = md->files;
	int nfiles = langA_varlen(files);
	for (int x = 0; x < nfiles; ++ x) {
		if (line < files[x].lines) continue;
		if (line > files[x].lines+files[x].nlines-1) continue;
		return x;
	}
	return -1;
}


void langR_error(lRuntime *R, lbyteid id, char *error) {
	lModule *md = R->md;
	if (id == NO_BYTE) id = R->j;
	llineid line = md->lines[id];
	int fileid = fndfile(md,line);
	if (fileid != -1) {
		lFile *file = &md->files[fileid];
		langX_error2(file->name,file->lines,line,error);
	}
}


void langR_typecheck(lRuntime *R, lbyteid id, lvaluetag x, lvaluetag y) {
	if (x != y) {
		langR_error(R,id,S_tpf("expected %s, instead got %s",tag2s[x],tag2s[y]));
	}
}


/*
** Calls a function, takes an optional object for meta
** fields, nx and ny are the number of in and out values
** respectively, rx is the local slot where the function
** value (bytecode or binding) resides, ry is the first
** result register (used for metacalls).
*/
int lang_call(lRuntime *R, lObject *obj, llocalid rx, int nx, int ny) {
	LASSERT((R->top - R->frame->base) >= rx);
	/* clear the locals? */
	lContext *caller = R->frame;
	lValue func = caller->base[rx];
	lValue *base = caller->base + rx + 1;
	/* top always points to one past locals,
	so far we only have nx argument locals,
	top is later incremented to match nlocals */
	lValue *top  = base + nx;
	lContext frame = {0};
	frame.top = R->top;
	frame.obj = obj;
	frame.base = base;
	frame.x = nx;
	frame.y = ny;
	if (func.tag == TAG_CLOSURE) {
		frame.cl = func.f;
		/* increment top to fill the function's locals */
		/* todo: replace with faster memset? */
		for (; nx < frame.cl->fn.nlocals; ++ nx) {
			top->tag = TAG_NIL;
			top->i   = 0;
			top ++;
		}
	}
	R->top = top;
	R->frame = &frame;
	llocalid nyield = 0;
	if (func.tag == TAG_CLOSURE) {
		nyield = lang_resume(R);
	} else
	if (func.tag == TAG_BINDING) {
		if (func.c != lnil) {
			nyield = func.c(R);
			/* ensure that the results were pushed to the stack */
			LASSERT(nyield <= R->top - frame.base);
			/* move results to hosted registers */
			int m = nyield < ny ? nyield : ny;
			for (int p = 0; p < m; ++ p) {
				if (obj != lnil) {
					/* todo: */
					frame.base[p-2] = R->top[p-nyield];
				} else {
					frame.base[p-1] = R->top[p-nyield];
				}
			}
		}
	} else {
		nyield = -1;
		langR_error(R,NO_BYTE,"not a function");
	}
	/* finally restore stack */
	R->frame = caller;
	R->top = frame.top;
	return nyield;
}


/*
** Loads a file and calls its function,
** returns the number of results.
*/
int lang_loadfile(lRuntime *rt, FileState *fs, lString *filename, llocalid x, int y) {

	if (filename == lnil) filename = lang_checkString(rt,x);

	char *contents;
	Error error = sys_loadfilebytes(lHEAP,&contents,filename->string);
	if (LFAILED(error)) return -1;

	lModule *md = rt->md;

	fs->rt = rt;
	fs->md = md;
	fs->bytes = md->nbytes;
	fs->filename = filename->c;
	fs->contents = contents;
	fs->thischar = contents;
	fs->linechar = contents;
	fs->linenumber = 1;

	/* kick start by lexing the first two tokens */
	langX_yield(fs);
	langX_yield(fs);


	FileFunc fn = {0};
	langY_beginfn(fs,&fn,fs->tk.line);
	while (!langX_test(fs,0)) langY_loadstat(fs);
	langY_closefn(fs);

	/* todo: this is temporary, please remove this or make
	some sort of object out of it... */
	lFile fl = {0};
	fl.bytes = fn.bytes;
	fl.nbytes = md->nbytes - fn.bytes;
	fl.name = filename->c;
	fl.lines = contents;
	fl.nlines = strlen(contents);
	fl.pathondisk = filename->c;
	langA_varadd(md->files,fl);

	lProto p = {0};
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes = md->nbytes - fn.bytes;

	/* -- todo: can we do this better? convert string to closure */
	rt->frame->base[x].tag = TAG_CLOSURE;
	rt->frame->base[x].f   = langF_newclosure(rt,p);
	if (rt->top == rt->frame->base) ++ rt->top;

	int nyield = lang_call(rt,lnil,x,0,y);

	/* todo: lines? */
	// langM_dealloc(lHEAP,contents);
	return nyield;
}


int lang_resume(lRuntime *R) {
	lRuntime *cs = R;
	lContext *c = R->f;
	lContext *frame = R->frame;
	lModule *md = R->md;
	lClosure *cl = c->cl;
	lProto fn = cl->fn;

	while (c->j < fn.nbytes) {
		llongint jp = c->j ++;
		llongint bc = fn.bytes + jp;
		lBytecode b = md->bytes[bc];

		#if 1
		if (R->logging) bytefpf(md,stdout,jp,b);
		#endif

		switch (b.k) {
			case BC_LEAVE: {
				if (frame->dl != lnil) {
					frame-> j = frame->dl->j;
					frame->dl = frame->dl->n;
				} else goto leave;
			} break;
			case BC_DELAY: {
				/* todo: can we make this better */
				ldelaylist *dl = langM_alloc(lHEAP,sizeof(ldelaylist));
				dl->n = c->dl;
				dl->j = c->j;
				c->dl = dl;

				LASSERT(b.i >= 0);
				c->j = jp + b.i;
			} break;
			case BC_YIELD: {
				LASSERT(b.x >= 0);
				/* -- check that we don't exceed number of
				expected outputs */
				int z = b.z < frame->y ? b.z : frame->y;
				/* -- base points to the first local, -1 is where
				the function value is at and the first yield
				register */
				for (int p = 0; p < z; ++p) {
					frame->base[p-1] = frame->base[b.y+p];
				}
				frame->y = z;
				frame->j = jp + b.x;
			} break;
			case BC_STKGET: {
				langR_typecheck(R,bc,TAG_INTEGER,frame->base[b.y].tag);
				frame->base[b.x] = frame->base[frame->base[b.y].i];
			} break;
			case BC_STKLEN: {
				frame->base[b.x].tag = TAG_INTEGER;
				frame->base[b.x].i   = R->top - frame->base;
			} break;
			case BC_LOADFILE: {
				FileState fs = {0};
				lang_loadfile(R,&fs,lnil,b.x,b.y);
			} break;
			case BC_J: {
				c->j = jp + b.i;
			} break;
			case BC_JZ: {
				if (c->base[b.y].i == 0) c->j = jp + b.x;
			} break;
			case BC_JNZ: {
				if (c->base[b.y].i != 0) c->j = jp + b.x;
			} break;
			case BC_RELOAD: {
				frame->base[b.x] = frame->base[b.y];
			} break;
			case BC_LOADGLOBAL: {
				frame->base[b.x] = md->g->v[b.y];
			} break;
			case BC_SETGLOBAL: {
				md->g->v[b.x] = frame->base[b.y];
			} break;
			case BC_LOADNIL: {
				frame->base[b.x].tag = TAG_NIL;
				frame->base[b.x].i   = 0;
			} break;
			case BC_LOADINT: {
				frame->base[b.x].tag = TAG_INTEGER;
				frame->base[b.x].i   = md->ki[b.y];
			} break;
			case BC_LOADNUM: {
				frame->base[b.x].tag = TAG_NUMBER;
				frame->base[b.x].n   = md->kn[b.y];
			} break;
			case BC_LOADCACHED: {
				LASSERT(b.y >= 0 && b.y < fn.ncaches);
				frame->base[b.x] = cl->caches[b.y];
			} break;
			case BC_CLOSURE: {
				LASSERT(b.y >= 0 && b.y < langA_varlen(md->p));
				lProto p = md->p[b.y];
				lClosure *ncl = langF_newclosure(cs,p);
				for (int i = 0; i < p.ncaches; ++i) {
					ncl->caches[i] = frame->base[b.x+i];
				}
				frame->base[b.x].tag = TAG_CLOSURE;
				frame->base[b.x].f   = ncl;
			} break;
			case BC_TABLE: {
				frame->base[b.x].tag = TAG_TABLE;
				frame->base[b.x].t   = langH_new(cs);
			} break;
			case BC_INDEX: case BC_FIELD: {
				#define LNIL (lValue){TAG_NIL}
				if (frame->base[b.y].tag == TAG_TABLE) {
					frame->base[b.x] = langH_lookup(frame->base[b.y].t,frame->base[b.z]);
				} else frame->base[b.x] = LNIL;
			} break;
			case BC_SETFIELD: {
				if (frame->base[b.x].tag == TAG_TABLE) {
					langH_insert(frame->base[b.x].t,frame->base[b.y],frame->base[b.z]);
				} else LNOBRANCH;
			} break;
			case BC_SETINDEX: {
				if (frame->base[b.x].tag == TAG_TABLE) {
					langH_insert(frame->base[b.x].t,frame->base[b.y],frame->base[b.z]);
				} else LNOBRANCH;
			} break;
			case BC_METANAME: {
				frame->base[b.x].tag = TAG_BINDING;
				frame->base[b.x].c   = lang_fndmetafield(frame->base[b.y].j,frame->base[b.z].s);
			} break;
			case BC_METACALL: {
				cs->j = bc;
				lang_call(cs,frame->base[b.x].j,b.x+1,b.y,b.z);
			} break;
			case BC_CALL: {
				cs->j = bc;
				lang_call(cs,0,b.x,b.y,b.z);
			} break;
			case BC_ISNIL: {
				lValue x = frame->base[b.y];
				lbool nan = x.tag != TAG_INTEGER && x.tag != TAG_NUMBER;
				frame->base[b.x].tag = TAG_INTEGER;
				frame->base[b.x].i   = x.tag == TAG_NIL || (nan && x.i == 0);
			} break;
			case BC_EQ: case BC_NEQ: {
				lValue x = frame->base[b.y];
				lValue y = frame->base[b.z];
				lbool eq = lfalse;
				if (x.tag == TAG_STRING && y.tag == TAG_STRING) {
					eq = langS_eq(x.s,y.s);
				} else {
					eq = x.i == y.i;
				}
				if (b.k == BC_NEQ) eq = !eq;

				frame->base[b.x].tag = TAG_INTEGER;
				frame->base[b.x].i   = eq;
			} break;


			/* todo: make this better */
			#define CASE_IBOP(OPNAME,OP) \
			case OPNAME : {\
				c->l[b.x].tag = TAG_INTEGER;\
				c->l[b.x].i   = ltolong(c->l[b.y]) OP ltolong(c->l[b.z]);\
			} break

			/* todo: make this better */
			#define CASE_BOP(OPCODE,OP) \
			case OPCODE : {\
				if (c->l[b.y].tag == TAG_NUMBER || c->l[b.z].tag == TAG_NUMBER) {\
					c->l[b.x].tag = TAG_NUMBER;\
					c->l[b.x].n   = ltonumber(c->l[b.y]) OP ltonumber(c->l[b.z]);\
				} else {\
					c->l[b.x].tag = TAG_INTEGER;\
					c->l[b.x].i   = ltolong(c->l[b.y]) OP ltolong(c->l[b.z]);\
				}\
			} break

			case BC_LTEQ: {
				lValue x = c->l[b.y];
				lValue y = c->l[b.z];
				if (x.tag == TAG_NUMBER || y.tag == TAG_NUMBER) {
					c->l[b.x].tag = TAG_INTEGER;
					c->l[b.x].i   = ltonumber(x) <= ltonumber(y);
				} else {
					c->l[b.x].tag = TAG_INTEGER;
					c->l[b.x].i   = ltolong(x) <= ltolong(y);
				}
			} break;
			case BC_LT: {
				lValue x = c->l[b.y];
				lValue y = c->l[b.z];
				if (x.tag == TAG_NUMBER || y.tag == TAG_NUMBER) {
					c->l[b.x].tag = TAG_NUMBER;
					c->l[b.x].n   = ltonumber(x) < ltonumber(y);
				} else {
					c->l[b.x].tag = TAG_INTEGER;
					c->l[b.x].i   = ltolong(x) < ltolong(y);
				}
			} break;

			CASE_IBOP(BC_SHL,  <<);
			CASE_IBOP(BC_SHR,  >>);
			CASE_IBOP(BC_XOR,   ^);
			CASE_IBOP(BC_MOD,   %);

			CASE_BOP(BC_ADD,   +);
			CASE_BOP(BC_SUB,   -);
			CASE_BOP(BC_MUL,   *);
			CASE_BOP(BC_DIV,   /);
			#undef CASE_BOP

			default: {

				LNOBRANCH;
			} break;
		}
	}

	leave:
	return c->y;
}



lapi llongint lang_poplong(lRuntime *c) {
	lValue v = *(-- c->v);
	LASSERT(v.tag == TAG_INTEGER);
	return v.i;
}


lapi llongint lang_topop(lRuntime *R) {
	return R->top - R->frame->base;
}


lapi lValue lang_load(lRuntime *c, int x) {
	return c->f->l[x];
}


lapi lString *lang_loadS(lRuntime *c, int x) {
	lValue v = c->f->l[x];
	LASSERT(v.tag == TAG_STRING);
	return v.s;
}


lapi lClosure *lang_loadcl(lRuntime *c, int x) {
	lValue v = c->f->l[x];
	LASSERT(v.tag == TAG_CLOSURE);
	return v.f;
}


lapi void lang_checkcl(lRuntime *c, llocalid x) {
	LASSERT(c->f->l[x].tag == TAG_CLOSURE);
}


lapi lString *lang_checkString(lRuntime *c, llocalid x) {
	LASSERT(c->f->l[x].tag == TAG_STRING);
	return c->f->l[x].s;
}


lapi llongint lang_loadlong(lRuntime *c, int x) {
	lValue v = c->f->l[x];
	if (v.tag == TAG_NUMBER) {
		return (llongint) v.n;
	}
	LASSERT(v.tag == TAG_INTEGER);
	return v.i;
}


lapi lnumber lang_loadnum(lRuntime *c, llocalid x) {
	lValue v = c->f->l[x];
	if (v.tag == TAG_INTEGER) return (lnumber) v.i;
	LASSERT(v.tag == TAG_NUMBER);
	return v.n;
}


lapi Handle lang_loadhandle(lRuntime *c, llocalid x) {
	lValue v = c->f->l[x];
	LASSERT(v.tag == TAG_HANDLE);
	return v.h;
}


llocalid lang_stkalloc(lRuntime *R, int n) {
	llocalid stkptr = R->top - R->stk;
	if (stkptr <= R->stklen) {
		R->top += n;
	} else LNOBRANCH;
	return stkptr;
}


llocalid lang_pushvalue(lRuntime *R, lValue v) {
	*R->top = v;
	return lang_stkalloc(R,1);
}


void lang_pushnil(lRuntime *c) {
	c->v->tag = TAG_NIL;
	++ c->v;
}


void lang_pushlong(lRuntime *c, llongint i) {
	c->v->tag = TAG_INTEGER;
	c->v->i = i;
	++ c->v;
}


void lang_pushnum(lRuntime *c, lnumber n) {
	c->v->tag = TAG_NUMBER;
	c->v->n = n;
	++ c->v;
}


void lang_pushhandle(lRuntime *c, Handle h) {
	c->v->tag = TAG_HANDLE;
	c->v->h = h;
	++ c->v;
}


void lang_pushString(lRuntime *c, lString *s) {
	c->v->tag = TAG_STRING;
	c->v->s = s;
	++ c->v;
}


llocalid lang_pushclosure(lRuntime *R, lClosure *cl) {
	R->top->tag = TAG_CLOSURE;
	R->top->f   = cl;
	return R->top ++ - R->frame->base;
}


void lang_pushtable(lRuntime *c, lTable *t) {
	c->v->tag = TAG_TABLE;
	c->v->t = t;
	++ c->v;
}


void lang_pushbinding(lRuntime *c, lBinding b) {
	c->v->tag = TAG_BINDING;
	c->v->c = b;
	++ c->v;
}


lTable *lang_pushnewtable(lRuntime *c) {
	lTable *t = langH_new(c);
	lang_pushtable(c,t);
	return t;
}


lString *lang_pushnewS(lRuntime *c, char const *junk) {
	lString *s = langS_new(c,junk);
	lang_pushString(c,s);
	return s;
}


llocalid lang_pushnewclosure(lRuntime *R, lProto fn) {
	lClosure *cl = langF_newclosure(R,fn);
	LASSERT(lang_topop(R) >= fn.ncaches);
	R->top -= fn.ncaches;
	int i;
	for (i=0; i<fn.ncaches; ++i) {
		cl->caches[i] = R->top[i];
	}
	return lang_pushclosure(R,cl);
}

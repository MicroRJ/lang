/*
** See Copyright Notice In lang.h
** lruntime.c
** lRuntime
*/


lnumber ltonumber(lValue v) {
	return v.tag == TAG_INTEGER ? (lnumber) v.i : v.n;
}


llongint ltolong(lValue v) {
	return v.tag == TAG_NUMBER ? (llongint) v.n : v.i;
}


/* imagine being fast */
lBinding lfndmetafn(lObject *j, lString *name) {
	for (int i = 0; i < j->_n; i ++) {
		if (S_eq(j->_m[i].name,name->c)) {
			return j->_m[i].c;
		}
	}
	return lnil;
}


int lang_callargs(lRuntime *c, lClosure *cl, int x, int y, ...) {
	#if 0
	va_list v;
	va_start(v,y);
	for (int i = 0; i < x; ++i) {
		lang_pushvalue(c,va_arg(v,lValue));
	}
	va_end(v);
	return lang_call(c,0,cl,x,y);
	#endif
	return 0;
}


/*
** , takes an optional
** object for meta functions, x and y are the
** number of in and out values respectively.
*/
#if 0
int lang_bind(lRuntime *c, lObject *obj, lBinding b, int x, int y) {
	LASSERT(lang_leftover(c) >= x + y);
	lContext s = {0};
	s.obj = obj;
	s.l = c->v - x;
	s.x = x;
	s.y = y;
	lContext *f = c->f;
	c->f = &s;
	int r = b(c);
	/* move results to hosted registers */
	int m = r < y ? r : y;
	for (int p = 0; p < m; ++ p) {
		s.l[p-y] = c->v[p-r];
	}
	c->f = f;
	c->v = s.l;
	return r;
}
#endif


/*
** Calls a function, takes an optional object for meta
** methods, x and y are the number of in and out values
** respectively, r is the local slot where the function
** value (bytecode or binding) resides.
*/
int lang_call(lRuntime *R, lObject *obj, llocalid r, int x, int y) {
	LASSERT((R->top - R->frame->base) >= r);
	/* clear the locals? */
	lContext *caller = R->frame;
	lValue func = caller->base[r];
	lValue *base = caller->base + r + 1;
	lValue *top  = base + x;
	lContext frame = {0};
	frame.top  = R->top;
	frame.obj  = obj;
	frame.base = base;
	frame.x = x;
	frame.y = y;
	if (func.tag == TAG_CLOSURE) {
		frame.cl = func.f;
		/* todo: replace with faster memset? */
		for (; x < frame.cl->fn.nlocals; ++ x) {
			top->tag = VALUE_NONE;
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
	if (func.tag == VALUE_BINDING) {
		if (func.c != lnil) {
			nyield = func.c(R);
			/* ensure that the results where pushed to the stack */
			LASSERT(nyield <= R->top - frame.base);
			/* move results to hosted registers */
			int m = nyield < y ? nyield : y;
			for (int p = 0; p < m; ++ p) {
				frame.base[p-1] = R->top[p-nyield];
			}
		}
	} else lang_logerror("not a function!");
	/* finally restore stack */
	R->frame = caller;
	R->top = frame.top;
	return nyield;
}


/*
** Loads a file and calls its function,
** returns the number of results.
*/
int lang_loadfile(lRuntime *rt, FileState *fs, lString *filename, int y) {

	char *contents;
	Error error = sys_loadfilebytes(lHEAP,&contents,filename->string);
	if (LFAILED(error)) return -1;

	lModule *md = rt->md;

	fs->rt = rt;
	fs->md = md;
	fs->bytes = md->nbytes;
	fs->filename = filename->string;
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
	p.bytes = p.bytes;
	p.nbytes = md->nbytes - fn.bytes;

	llocalid cl = lang_pushnewclosure(rt,p);
	int nyield = lang_call(rt,lnil,cl,0,y);

	/* todo: lines? */
	// langM_dealloc(lHEAP,contents);
	return nyield;
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
				LASSERT(b.x == 1);
				LASSERT(b.y <= 1);
				llongint i = ltolong(*-- cs->v);
				if (b.y != 0) {
					cs->v[-b.y] = c->l[i];
				}
			} break;
			case BC_STKLEN: {
				LASSERT(b.x == 0);
				LASSERT(b.y <= 1);
				if (b.y != 0) {
					cs->v[-b.y].tag = TAG_INTEGER;
					cs->v[-b.y].i   = cs->v - c->l;
				}
			} break;
			case BC_LOADFILE: {
				LASSERT(lang_leftover(cs) >= 1);
				LASSERT(cs->v[-1].tag == VALUE_STRING);
				lString *name = (--cs->v)->s;
				langGC_markpink((lObject*)name);
				FileState fs = {0};
				lang_loadfile(cs,&fs,name,b.y);
				langGC_markwhite((lObject*)name);
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
			case BC_LOADINT: {
				frame->base[b.x].tag = TAG_INTEGER;
				frame->base[b.x].i   = b.y;
			} break;
			case BC_LOADCACHED: {
				LASSERT(b.i >= 0 && b.i < fn.ncaches);
				frame->base[b.x] = cl->caches[b.i];
			} break;
			case BC_CLOSURE: {
				LASSERT(b.y >= 0 && b.y < langA_varlen(md->p));
				frame->base[b.x].tag = TAG_CLOSURE;
				frame->base[b.x].f   = langF_newclosure(cs,md->p[b.y]);
			} break;
			case BC_TABLE: {
				frame->base[b.x].tag = TAG_TABLE;
				frame->base[b.x].t   = langH_new(cs);
			} break;
			case BC_FIELD: {
				LASSERT(frame->base[b.y].tag == TAG_TABLE);
				frame->base[b.x] = langH_lookup(frame->base[b.y].t,frame->base[b.z]);
			} break;
			case BC_SETFIELD: {
				LASSERT(frame->base[b.x].tag == TAG_TABLE);
				langH_insert(frame->base[b.x].t,frame->base[b.y],frame->base[b.z]);
			} break;
			case BC_SETINDEX: {
				lValue t = cs->v[-3];
				lValue k = cs->v[-2];
				lValue v = cs->v[-1];
				cs->v -= 2;

				if (t.tag == VALUE_STRING) {
					if (v.tag == TAG_INTEGER) {
						t.s->c[k.i] = v.i;
					} else {
						LNOBRANCH;
					}
				} else
				if (t.tag == TAG_TABLE) {
					langH_insert(t.t,k,v);
				} else {
					LNOBRANCH;
				}
			} break;
			case BC_INDEX: {
				cs->v -= 2;
				lValue t,k;
				t = cs->v[0];
				k = cs->v[1];
				if (t.tag == VALUE_STRING) {
					if (k.tag == TAG_INTEGER) {
						lang_pushlong(cs,t.s->c[k.i]);
					} else {
						LNOBRANCH;
					}
				} else
				if (t.tag == TAG_TABLE) {
					lang_pushvalue(cs,langH_lookup(t.t,k));
				} else {
					lang_pushnil(cs);
				}
			} break;

			case BC_METACALL: {
				LNOBRANCH;
				// cs->v -= 2;
				// lValue o = cs->v[0];
				// lValue m = cs->v[1];

				// LASSERT(m.tag == VALUE_STRING);
				// LASSERT(ttisobj(o.tag));
				// lBinding d = lfndmetafn(o.j,m.s);
				// if (d != lnil) {
				// 	lang_call(cs,o.j,d,b.x,b.y);
				// }
			} break;
			case BC_CALL: {
				if (R->top < frame->base) LNOBRANCH;
				lang_call(cs,0,b.x,b.y,b.z);
				if (R->top < frame->base) LNOBRANCH;
			} break;
			case BC_ISNIL: {
				LNOBRANCH;
				cs->v -= 1;
				lValue x = cs->v[0];
				if (x.tag == TAG_INTEGER || x.tag == TAG_NUMBER) {
					lang_pushlong(cs,0);
				} else {
					lang_pushlong(cs,x.i == 0);
				}
			} break;
			case BC_EQ: case BC_NEQ: {
				lValue x = frame->base[b.y];
				lValue y = frame->base[b.z];
				lbool eq = lfalse;
				if (x.tag == VALUE_STRING && y.tag == VALUE_STRING) {
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
					c->l[b.x].tag = TAG_NUMBER;
					c->l[b.x].n   = ltonumber(x) <= ltonumber(y);
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


lapi llongint lang_leftover(lRuntime *c) {
	return c->v - c->f->l;
}


lapi lValue lang_load(lRuntime *c, int x) {
	return c->f->l[x];
}


lapi lString *lang_loadS(lRuntime *c, int x) {
	lValue v = c->f->l[x];
	LASSERT(v.tag == VALUE_STRING);
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
	LASSERT(v.tag == VALUE_HANDLE);
	return v.h;
}


void lang_pushvalue(lRuntime *c, lValue v) {
	LASSERT((c->top - c->stk) < c->stklen);
	*(c->top ++) = v;
}


void lang_pushnil(lRuntime *c) {
	c->v->tag = VALUE_NONE;
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
	c->v->tag = VALUE_HANDLE;
	c->v->h = h;
	++ c->v;
}


void lang_pushString(lRuntime *c, lString *s) {
	c->v->tag = VALUE_STRING;
	c->v->s = s;
	++ c->v;
}


llocalid lang_pushclosure(lRuntime *R, lClosure *cl) {
	R->top->tag = TAG_CLOSURE;
	R->top->f   = cl;
	return R->top ++ - R->frame->base;
}


void lang_pushtable(lRuntime *c, Table *t) {
	c->v->tag = TAG_TABLE;
	c->v->t = t;
	++ c->v;
}


void lang_pushbinding(lRuntime *c, lBinding b) {
	c->v->tag = VALUE_BINDING;
	c->v->c = b;
	++ c->v;
}


Table *lang_pushnewtable(lRuntime *c) {
	Table *t = langH_new(c);
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
	LASSERT(lang_leftover(R) >= fn.ncaches);
	R->top -= fn.ncaches;
	int i;
	for (i=0; i<fn.ncaches; ++i) {
		cl->caches[i] = R->top[i];
	}
	return lang_pushclosure(R,cl);
}

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
	va_list v;
	va_start(v,y);
	for (int i = 0; i < x; ++i) {
		lang_pushvalue(c,va_arg(v,lValue));
	}
	va_end(v);
	return lang_call(c,0,cl,x,y);
}


/*
** Calls a binding function, takes an optional
** object for meta functions, x and y are the
** number of in and out values respectively.
*/
int lang_bind(lRuntime *c, lObject *obj, lBinding b, int x, int y) {
	LASSERT(lang_leftover(c) >= x + y);
	CallFrame s = {0};
	s.obj = obj;
	s.l = c->v - x;
	s.x = x;
	s.y = y;
	CallFrame *f = c->f;
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


/*
** Calls a bytecode function, takes an optional
** object for meta functions, x and y are the
** number of in and out values respectively.
*/
int lang_call(lRuntime *c, lObject *obj, lClosure *cl, int x, int y) {
	LASSERT((c->v-c->s) >= x + y);
	/* clear the locals? */
	for (; x < cl->fn.nlocals; ++ x) {
		c->v->tag = VALUE_NONE;
		c->v->i   = 0;
		c->v ++;
	}

	CallFrame s = {0};
	s.obj = obj;
	s.cl  = cl;
	s.l = c->v - x;
	s.x = x;
	s.y = y;
	CallFrame *f = c->f;
	c->f = &s;
	llocalid nleft = lang_resume(c);
	c->f = f;
	/* all the hosted return registers are above s.l */
	c->v = s.l;
	return nleft;
}


/*
** Loads a file and calls its function,
** returns the number of results.
*/
int lang_loadfile(lRuntime *rt, lString *filename, int y) {

	char *contents;
	Error error = sys_loadfilebytes(lHEAP,&contents,filename->string);
	if (LFAILED(error)) return -1;

	lModule *md = rt->md;

	FileState fs = {md};
	fs.rt = rt;
	fs.md = md;
	fs.bytes = md->nbytes;
	fs.filename = filename->string;
	fs.contents = contents;
	fs.thischar = contents;
	fs.linechar = contents;
	fs.linenumber = 1;

	/* kick start by lexing the first two tokens */
	langX_yield(&fs);
	langX_yield(&fs);


	FileFunc fn = {0};
	langY_beginfn(&fs,&fn,fs.tk.line);
	while (!langX_test(&fs,0)) langY_loadstat(&fs);
	langY_closefn(&fs);


	lFile fl = {0};
	fl.bytes = fn.bytes;
	fl.nbytes = md->nbytes - fn.bytes;
	fl.name = filename->c;
	fl.lines = contents;
	fl.nlines = strlen(contents);
	fl.pathondisk = filename->c;
	langA_varadd(md->files,fl);


	lClosure cl = {0};
	cl.fn.nlocals = fn.nlocals;
	cl.fn.bytes = fn.bytes;
	cl.fn.nbytes = md->nbytes - fn.bytes;

	int nleft = lang_call(rt,lnil,&cl,0,y);

	/* todo: lines? */
	// langM_dealloc(lHEAP,contents);

	return nleft;
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


int lang_resume(lRuntime *cs) {
	lModule *md = cs->md;
	CallFrame *c = cs->f;
	lClosure *cl = c->cl;
	lProto fn = cl->fn;

	while (c->j < fn.nbytes) {
		llongint jp = c->j ++;
		llongint bc = fn.bytes + jp;
		lBytecode b = md->bytes[bc];

		#if 1
		if (cs->logging) {
			printf(" | %-4lli: %-4lli, %s"
			,	jp,cs->v-c->l,lang_bytename(b.k));
			if (lang_byteclass(b.k) == BYTE_CLASS_XY) {
				printf("(x=%i,y=%i)",b.x,b.y);
			} else {
				printf("(i=%lli)",b.i);
			}
			if (b.k == BYTE_GLOBAL) {
				printf("  // ");
				syslib_fpfv_(stdout,md->g->v[b.i],ltrue);
			} else
			if (b.k == BYTE_LOCAL) {
				printf("  // ");
				syslib_fpfv_(stdout,c->l[b.i],ltrue);
			}
			printf("\n");
		}

		#endif
		switch (b.k) {
			case BYTE_LEAVE: {
				if (c->dl) {
					c-> j = c->dl->j;
					c->dl = c->dl->n;
				} else goto leave;
			} break;
			case BYTE_DELAY: {
				ldelaylist *dl = langM_alloc(lHEAP,sizeof(ldelaylist));
				dl->n = c->dl;
				dl->j = c->j;
				c->dl = dl;

				LASSERT(b.i >= 0);
				c->j = jp + b.i;
			} break;
			case BYTE_YIELD: {
				LASSERT(b.x >= 0);
				/* move return values to hosted
				return registers, discarding
				any excess returns. */
				int y = b.y < c->y ? b.y : c->y;
				for (int p = 0; p < y; ++p) {
					c->l[p-c->y] = cs->v[p-b.y];
				}
				c->y = y;
				/* todo: this is unnecessary */
				c->j = jp + b.x;
				cl->j = c->j;
			} break;
			case BYTE_STKGET: {
				LASSERT(b.x == 1);
				LASSERT(b.y <= 1);
				llongint i = ltolong(*-- cs->v);
				if (b.y != 0) {
					cs->v[-b.y] = c->l[i];
				}
			} break;
			case BYTE_STKLEN: {
				LASSERT(b.x == 0);
				LASSERT(b.y <= 1);
				if (b.y != 0) {
					cs->v[-b.y].tag = TAG_INTEGER;
					cs->v[-b.y].i   = cs->v - c->l;
				}
			} break;
			case BYTE_LOADFILE: {
				LASSERT(lang_leftover(cs) >= 1);
				LASSERT(cs->v[-1].tag == VALUE_STRING);
				lString *name = (--cs->v)->s;
				langGC_markpink((lObject*)name);
				lang_loadfile(cs,name,b.y);
				langGC_markwhite((lObject*)name);
			} break;
			case BYTE_J: {
				c->j = jp + b.i;
			} break;
			case BYTE_JZ: {
				lbool j = (-- cs->v)->i;
				if (j == 0) c->j = jp + b.i;
			} break;
			case BYTE_JNZ: {
				lbool j = (-- cs->v)->i;
				if (j != 0) c->j = jp + b.i;
			} break;
			case BYTE_DROP: {
				-- cs->v;
			} break;
			case BYTE_DUPL: {
				for (int i = 0; i < b.i; ++i) {
					*cs->v ++ = cs->v[-1];
				}
			} break;
			case BYTE_NIL: {
				/* todo: i should be how many more */
				for (int i = 0; i < b.i; ++i) {
					lang_pushnil(cs);
				}
			} break;
			case BYTE_NUM: {
				cs->v->tag = TAG_NUMBER;
				cs->v->n   = *((lnumber*)&b.i);
				++ cs->v;
			} break;
			case BYTE_LOADINT: {
				c->l[b.x].tag = TAG_INTEGER;
				c->l[b.x].i   = b.i;
			} break;
			case BYTE_INT: {
				cs->v->tag = TAG_INTEGER;
				cs->v->i   = b.i;
				++ cs->v;
			} break;
			case BYTE_CACHE: {
				LASSERT(b.i >= 0 && b.i < fn.ncaches);
				*cs->v ++ = cl->caches[b.i];
			} break;
			case BYTE_CLOSURE: {
				LASSERT(b.i >= 0 && b.i < langA_varlen(md->p));
				lang_pushnewcl(cs,md->p[b.i]);
			} break;
			case BYTE_LOCAL: {
				for (int i = 0; i < b.y; ++ i) {
					*cs->v ++ = c->l[b.x+i];
				}
			} break;
			case BYTE_SETLOCAL: {
				LASSERT(lang_leftover(cs) >= 1);
				c->l[b.i] = *(-- cs->v);
			} break;
			case BYTE_GLOBAL: {
				*cs->v ++ = md->g->v[b.i];
			} break;
			case BYTE_SETGLOBAL: {
				LASSERT(lang_leftover(cs) >= 1);
				md->g->v[b.i] = *(-- cs->v);
			} break;
			case BYTE_TABLE: {
				lang_pushnewtable(cs);
			} break;
			case BYTE_SETFIELD: {
				lValue t = cs->v[-3];
				lValue k = cs->v[-2];
				lValue v = cs->v[-1];
				cs->v -= 2;
				if (t.tag == VALUE_TABLE) {
					langH_insert(t.t,k,v);
				} else {
					LNOBRANCH;
				}
			} break;
			case BYTE_SETINDEX: {
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
				if (t.tag == VALUE_TABLE) {
					langH_insert(t.t,k,v);
				} else {
					LNOBRANCH;
				}
			} break;
			case BYTE_FIELD: {
				cs->v -= 2;
				lValue t,k;
				t = cs->v[0];
				k = cs->v[1];
				lang_pushvalue(cs,langH_lookup(t.t,k));
			} break;
			case BYTE_INDEX: {
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
				if (t.tag == VALUE_TABLE) {
					lang_pushvalue(cs,langH_lookup(t.t,k));
				} else {
					lang_pushnil(cs);
				}
			} break;

			case BYTE_METACALL: {
				cs->v -= 2;
				lValue o = cs->v[0];
				lValue m = cs->v[1];

				LASSERT(m.tag == VALUE_STRING);
				LASSERT(ttisobj(o.tag));
				lBinding d = lfndmetafn(o.j,m.s);
				if (d != lnil) {
					lang_bind(cs,o.j,d,b.x,b.y);
				}
			} break;
			case BYTE_CALL: {
				lValue v = *(-- cs->v);
				if (v.tag == VALUE_FUNC) {
					lang_call(cs,0,v.f,b.x,b.y);
				} else
				if (v.tag == VALUE_BINDING) {
					if (v.c != 0) {
						lang_bind(cs,0,v.c,b.x,b.y);
					}
				} else {
					lang_logerror("not a function");
				}
			} break;
			case BYTE_ISNIL: {
				cs->v -= 1;
				lValue x = cs->v[0];
				if (x.tag == TAG_INTEGER || x.tag == TAG_NUMBER) {
					lang_pushlong(cs,0);
				} else {
					lang_pushlong(cs,x.i == 0);
				}
			} break;
			case BYTE_EQ:
			case BYTE_NEQ: {
				cs->v -= 2;
				lValue x = cs->v[0];
				lValue y = cs->v[1];

				cs->v->tag = TAG_INTEGER;
				cs->v->i = lfalse;

				if (x.tag == VALUE_STRING && y.tag == VALUE_STRING) {
					cs->v->i = langS_eq(x.s,y.s);
				} else {
					cs->v->i = x.i == y.i;
				}

				if (b.k == BYTE_NEQ) {
					cs->v->i = !cs->v->i;
				}

				++ cs->v;
			} break;


			/* todo: make this better */
			#define CASE_IBOP(OPNAME,OP) \
			case OPNAME : {\
				cs->v -= 2;\
				lValue x = cs->v[0];\
				lValue y = cs->v[1];\
				lang_pushlong(cs, ltolong(x) OP ltolong(y));\
			} break

			/* todo: make this better */
			#define CASE_BOP(OPCODE,OP) \
			case OPCODE : {\
				cs->v -= 2;\
				lValue x = cs->v[0];\
				lValue y = cs->v[1];\
				if (x.tag == TAG_NUMBER || y.tag == TAG_NUMBER) {\
					lnumber xx = ltonumber(x);\
					lnumber yy = ltonumber(y);\
					lang_pushnum(cs, xx OP yy);\
				} else {\
					llongint xx = ltolong(x);\
					llongint yy = ltolong(y);\
					lang_pushlong(cs, xx OP yy);\
				}\
			} break

			case BYTE_LTEQ: {
				cs->v -= 2;
				lValue x = cs->v[0];
				lValue y = cs->v[1];
				if (x.tag == TAG_NUMBER || y.tag == TAG_NUMBER) {
					lang_pushlong(cs, ltonumber(x) <= ltonumber(y));
				} else {
					lang_pushlong(cs, ltolong(x) <= ltolong(y));
				}
			} break;
			case BYTE_LT: {
				cs->v -= 2;
				lValue x = cs->v[0];
				lValue y = cs->v[1];
				if (x.tag == TAG_NUMBER || y.tag == TAG_NUMBER) {
					lang_pushlong(cs, ltonumber(x) < ltonumber(y));
				} else {
					lang_pushlong(cs, ltolong(x) < ltolong(y));
				}
			} break;

			CASE_IBOP(BYTE_SHL,  <<);
			CASE_IBOP(BYTE_SHR,  >>);
			CASE_IBOP(BYTE_XOR,   ^);
			CASE_IBOP(BYTE_MOD,   %);

			CASE_BOP(BYTE_ADD,   +);
			CASE_BOP(BYTE_SUB,   -);
			CASE_BOP(BYTE_MUL,   *);
			CASE_BOP(BYTE_DIV,   /);
			#undef CASE_BOP

			default: {

				LNOBRANCH;
			} break;
		}
		if (cs->v < c->l+fn.nlocals) LNOBRANCH;
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
	LASSERT(v.tag == VALUE_FUNC);
	return v.f;
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
	LASSERT(c->v - c->s < c->z);
	*(c->v ++) = v;
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


void lang_pushclosure(lRuntime *c, lClosure *f) {
	c->v->tag = VALUE_FUNC;
	c->v->f = f;
	++ c->v;
}


void lang_pushtable(lRuntime *c, Table *t) {
	c->v->tag = VALUE_TABLE;
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


lClosure *lang_pushnewcl(lRuntime *c, lProto fn) {
	lClosure *cl = langF_newclosure(c,fn);
	LASSERT(lang_leftover(c) >= fn.ncaches);
	c->v -= fn.ncaches;
	int i;
	for (i=0; i<fn.ncaches; ++i) {
		cl->caches[i] = c->v[i];
	}
	lang_pushclosure(c,cl);
	return cl;
}

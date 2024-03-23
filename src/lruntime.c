/*
** See Copyright Notice In lang.h
** lruntime.c
** Runtime
*/


int findmetafunc(Object *j, char const *name) {
	for (int i = 0; i < j->_n; i ++) {
		if (S_eq(j->_m[i].name,name)) {
			return i;
		}
	}
	return -1;
}


int langR_callfuncargs(Runtime *c, Closure *cl, int n, ...) {
	va_list v;
	va_start(v,n);
	int i;
	for (i=0;i<n;++i) {
		langR_pushV(c,va_arg(v,Value));
	}
	va_end(v);
	return langR_callfunc(c,cl,n);
}


int langR_callCfunc(Runtime *c, CFunc cf, int n) {
	LASSERT(langR_stacksize(c) >= n);
	CallFrame s = {0};
	s.l  = c->v - n;
	s.n  = n;
	CallFrame *f = c->f;
	c->f = &s;
	int r = cf(c);
	/* shift up all of the return values, c->v
	should point to one past the last return
	value, the first return value is at p-r */
	for (int p = 0; p < r; ++ p) {
		s.l[p] = c->v[p-r];
	}
	c->f = f;
	c->v = s.l + r;
	LASSERT(langR_stacksize(c) >= r);
	return r;
}


int langR_callmetafunc(Runtime *c, Object *m, String *name, int n) {
	LASSERT(langR_stacksize(c) >= n);

	int fm = findmetafunc(m,name->string);
	if (fm == -1) {
		return -1;
	}


	CallFrame s = {0};
	s.l = c->v - n;
	s.n = n;
	s.obj = m;
	CallFrame *f = c->f;
	c->f = &s;
	int r = m->_m[fm].c(c);
	/* shift up all of the return values, c->v
	should point to one past the last return
	value, the first return value is at p-r */
	for (int p = 0; p < r; ++ p) {
		s.l[p] = c->v[p-r];
	}
	c->f = f;
	c->v = s.l + r;
	LASSERT(langR_stacksize(c) >= r);
	return r;
}


int langR_callfunc(Runtime *c, Closure *cl, int n) {
	/* clear the locals? */
	for (; n < cl->fn.nlocals; ++ n) {
		c->v->tag = VALUE_NONE;
		c->v->i   = 0;
		c->v ++;
	}

	CallFrame s = {0};
	s.cl = cl;
	s.l  = c->v - n;
	s.n  = n;
	CallFrame *f = c->f;
	c->f = &s;
	int r = langR_resume(c);
	c->f = f;
	/* shift up all of the return values, c->v
	should point to one past the last return
	value, the first return value is at p-r */
	for (int p = 0; p < r; ++ p) {
		s.l[p] = c->v[p-r];
	}
	c->v = s.l + r;
	return r;
}


/*
** Loads a file and calls its function,
** returns the number of results.
** todo: this func will probably be split.
*/
int langR_loadfile(Runtime *rt, String *filename) {

	char *contents;
	Error error = sys_loadfilebytes(elHEAP,&contents,filename->string);
	if (LFAILED(error)) {
		return -1;
	}

	Module *md = rt->md;

	FileState fs = {md};
	fs.rt = rt;
	fs.md = md;
	fs.bytes = md->nbytes;
	fs.filename = filename->string;
	fs.contents = contents;
	fs.thischar = contents;
	fs.linechar = contents;
	fs.linenumber = 1;

	/* Create the file function state, this is
	mainly to ensure we emit jump instructions
	that are relative to where the file is,
	within the module, otherwise the code
	generator would think that we're at byte offset 0,
	besides, the code generator will crash if there
	isn't a function present anyways.
	We also treat files as functions, similar
	to what lua does.
	When you load a file you're effectively
	loading its data and calling its function,
	which is a function that encloses all of
	the bytes within that file. */
	FuncState fn = {0};
	fn.bytes = fs.bytes;

	fs.fn = &fn;

	// langY_enterlevel(&fs);
	/* kick start by lexing the first
	two tokens */
	langX_yield(&fs);
	langX_yield(&fs);
	while (!langY_testtk(&fs,0)) {
		if (langY_parsestat(&fs) < 0) break;
	}
	// langY_leavelevel(&fs);

	fs.nbytes = md->nbytes - fs.bytes;

	Closure cl = {0};
	cl.fn.nlocals = fs.nlocals;
	cl.fn.bytes = fs.bytes;
	cl.fn.nbytes = fs.nbytes;

	int nleft = langR_callfunc(rt,&cl,0);

	/* free stuff */
	langM_dealloc(elHEAP,contents);

	return nleft;
}


int langR_resume(Runtime *cs) {
	Module *md = cs->md;
	CallFrame *c = cs->f;
	Closure *cl = c->cl;
	Proto fn = cl->fn;

	while (c->j < fn.nbytes) {
		Integer bc = fn.bytes + c->j ++;
		Bytecode b = md->bytes[bc];

		// lang_loginfo("%lli: %s, %lli",c->j-1,lang_bytename(b.k),b.i);
		// langX_error(fs,md->lineinfo[bc],"%lli: %s, %lli",c->j-1,lang_bytename(b.k),b.i);

		switch (b.k) {
			case BYTE_RET: {
				return b.i;
			}
			case BYTE_LOADFILE: {
				/* notice how the string is left
				on the stack, this is so that
				it won't be gc'd while we load
				the file */
				/* todo: pop the string somehow? */
				LASSERT(langR_stacksize(cs) >= 1);
				LASSERT(cs->v[-1].tag == VALUE_STRING);
				String *f = (--cs->v)->s;
				langGC_markpink((Object*)f);
				langR_loadfile(cs,f);
			} break;
			case BYTE_J: {
				/* todo?: should be additive */
				c->j = b.i;
			} break;
			case BYTE_JZ: {
				int j = (-- cs->v)->i;
				LASSERT(b.i >= 0);
				/* todo?: should we make this additive? */
				if (j == 0) c->j = b.i;
			} break;
			case BYTE_JNZ: {
				int j = (-- cs->v)->i;
				LASSERT(b.i >= 0);
				/* todo?: should we make this additive? */
				if (j != 0) c->j = b.i;
			} break;
			case BYTE_POP: {
				-- cs->v;
			} break;
			case BYTE_DUP: {
				*cs->v ++ = cs->v[-1];
			} break;
			case BYTE_INT: {
				cs->v->tag = VALUE_INTEGER;
				cs->v->i   = b.i;
				++ cs->v;
			} break;
			case BYTE_UGET: {
				LASSERT(b.i >= 0 && b.i < fn.ncaches);
				*cs->v ++ = cl->caches[b.i];
			} break;
			case BYTE_FNEW: {
				LASSERT(b.i >= 0 && b.i < langA_varlen(md->p));
				langR_pushnewF(cs,md->p[b.i]);
			} break;

			case BYTE_LGET: {
				*cs->v ++ = c->l[b.i];
			} break;
			case BYTE_LSET: {
				c->l[b.i] = *(-- cs->v);
			} break;

			case BYTE_GGET: {
				*cs->v ++ = md->g->v[b.i];
			} break;

			case BYTE_GSET: {
				LASSERT(cs->v[-1].tag != VALUE_NONE);
				// lang_loginfo("gset %lli",b.i);
				md->g->v[b.i] = *(-- cs->v);
			} break;

			case BYTE_TNEW: {
				Table *t = langH_new(cs);
				cs->v->tag = VALUE_TABLE;
				cs->v->t = t;
				++ cs->v;
			} break;

			case BYTE_TSET: {
				cs->v -= 3;
				Value t,k,v;
				t = cs->v[0];
				k = cs->v[1];
				v = cs->v[2];
				if (t.tag == VALUE_TABLE) {
					langH_insert(t.t,k,v);
				}
			} break;
			case BYTE_TGET: {
				cs->v -= 2;

				Value t,k;
				t = cs->v[0];
				k = cs->v[1];

				if (t.tag == VALUE_TABLE) {
					if (k.tag == VALUE_INTEGER) {
						if (k.i >= 0 && k.i < langA_varlen(t.t->v)) {
							langR_pushV(cs,t.t->v[k.i]);
						} else {
							langR_pushNil(cs);
						}
					} else {
						langR_pushV(cs,langH_lookup(t.t,k));
					}
				} else {
					langR_pushNil(cs);
				}
			} break;

			case BYTE_MCALL: {
				cs->v -= 2;
				Value o = cs->v[0];
				Value m = cs->v[1];
				LASSERT(o.tag == VALUE_TABLE || o.tag == VALUE_STRING);
				LASSERT(m.tag == VALUE_STRING);

				langR_callmetafunc(cs,o.j,m.s,b.i);
			} break;
			case BYTE_CALL: {
				Value x = *(-- cs->v);
				if (x.tag == VALUE_FUNC) {
					langR_callfunc(cs,x.f,b.i);
				} else
				if (x.tag == VALUE_CFUN) {
					if (x.c != 0) {
						langR_callCfunc(cs,x.c,b.i);
					}
				} else {
					lang_logerror("not a function");
				}
			} break;
			case BYTE_EQ:
			case BYTE_NEQ: {
				cs->v -= 2;
				Value x = cs->v[0];
				Value y = cs->v[1];

				cs->v->tag = VALUE_INTEGER;
				cs->v->i = False;

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

			#define CASE_IBOP(j,f)       \
			case j : {                   \
				cs->v -= 2;               \
				Integer x = cs->v[0].i;   \
				Integer y = cs->v[1].i;   \
				langR_pushI(cs, x f y);   \
			} break
				// printf("(%s %lli, %lli) = %lli\n",lang_bytename(b.k),x,y,tip(c).i); \

			CASE_IBOP(BYTE_LT ,  <);
			CASE_IBOP(BYTE_ADD,  +);
			CASE_IBOP(BYTE_SUB,  -);
			CASE_IBOP(BYTE_MOD,  %);
			CASE_IBOP(BYTE_MUL,  *);
			CASE_IBOP(BYTE_DIV,  /);
			#undef CASE_IBOP

			default: {

				LNOCHANCE;
			} break;
		}
	}

	return 0;
}

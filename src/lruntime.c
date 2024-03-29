/*
** See Copyright Notice In lang.h
** lruntime.c
** Runtime
*/


Binding lfndmetafn(Object *j, String *name) {
	for (int i = 0; i < j->_n; i ++) {
		if (S_eq(j->_m[i].name,name->c)) {
			return j->_m[i].c;
		}
	}
	return lnil;
}


int lang_callargs(Runtime *c, Closure *cl, int x, int y, ...) {
	va_list v;
	va_start(v,y);
	for (int i = 0; i < x; ++i) {
		lang_pushvalue(c,va_arg(v,Value));
	}
	va_end(v);
	return lang_call(c,0,cl,x,y);
}


/*
** Calls a binding function, takes an optional
** object for meta functions, x and y are the
** number of in and out values respectively.
*/
int lang_bind(Runtime *c, Object *obj, Binding b, int x, int y) {
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
	LASSERT(lang_leftover(c) >= r);
	return r;
}


/*
** Calls a bytecode function, takes an optional
** object for meta functions, x and y are the
** number of in and out values respectively.
*/
int lang_call(Runtime *c, Object *obj, Closure *cl, int x, int y) {
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
	llocalid nleft = lang_exec(c);
	c->f = f;
	/* all the hosted return registers are above s.l */
	c->v = s.l;
	return nleft;
}


/*
** Loads a file and calls its function,
** returns the number of results.
*/
int lang_loadfile(Runtime *rt, String *filename, int y) {

	char *contents;
	Error error = sys_loadfilebytes(lHEAP,&contents,filename->string);
	if (LFAILED(error)) {
		return -1;
	}

	Module *md = rt->md;

	FileState fs = {md};
	fs.rt = rt;
	fs.md = md;
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

	Closure cl = {0};
	cl.fn.nlocals = fn.nlocals;
	cl.fn.bytes = fn.bytes;
	cl.fn.nbytes = md->nbytes - fn.bytes;

	int nleft = lang_call(rt,lnil,&cl,0,y);

	/* free stuff */
	langM_dealloc(lHEAP,contents);

	return nleft;
}


int lang_exec(Runtime *cs) {
	Module *md = cs->md;
	CallFrame *c = cs->f;
	Closure *cl = c->cl;
	Proto fn = cl->fn;

	while (c->j < fn.nbytes) {
		llong bc = fn.bytes + c->j ++;
		Bytecode b = md->bytes[bc];

		#if 1
		if (cs->logging) {
			printf(" | %-4lli: %-4lli, %s"
			,	c->j-1,cs->v-c->l,lang_bytename(b.k));
			if (b.k == BYTE_YIELD || b.k == BYTE_CALL || b.k == BYTE_METACALL) {
				printf("(%i, %i)",b.x,b.y);
			} else {
				printf("(%lli)",b.i);
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
			/* todo: maybe the leave instruction
			itself could specify a jump address,
			if the jump address is zero then
			return, otherwise jump to that address */
			case BYTE_LEAVE: {
				goto leave;
			}
			case BYTE_YIELD: {
				LASSERT(b.x >= 0);

				/* move return values to hosted return registers,
				discarding any excess returns. */
				int y = b.y < c->y ? b.y : c->y;
				for (int p = 0; p < y; ++p) {
					c->l[p-c->y] = cs->v[p-b.y];
				}
				c->y = y;
				c->j = b.x;
			} break;
			case BYTE_LOADFILE: {
				LASSERT(lang_leftover(cs) >= 1);
				LASSERT(cs->v[-1].tag == VALUE_STRING);
				String *name = (--cs->v)->s;
				langGC_markpink((Object*)name);
				lang_loadfile(cs,name,b.y);
				langGC_markwhite((Object*)name);
			} break;
			case BYTE_J: {
				LASSERT(b.i >= 0);
				/* todo?: should be additive */
				c->j = b.i;
			} break;
			case BYTE_JZ: {
				lbool j = (-- cs->v)->i;
				LASSERT(b.i >= 0);
				if (j == 0) c->j = b.i;
			} break;
			case BYTE_JNZ: {
				lbool j = (-- cs->v)->i;
				LASSERT(b.i >= 0);
				if (j != 0) c->j = b.i;
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
				cs->v->tag = VALUE_REAL;
				cs->v->n   = *((lnumber*)&b.i);
				++ cs->v;
			} break;
			case BYTE_INT: {
				cs->v->tag = VALUE_LONG;
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
				*cs->v ++ = c->l[b.i];
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
				lang_pushtable(cs,langH_new(cs));
			} break;
			case BYTE_SETFIELD: {
				Value t = cs->v[-3];
				Value k = cs->v[-2];
				Value v = cs->v[-1];
				cs->v -= 2;
				if (t.tag == VALUE_TABLE) {
					langH_insert(t.t,k,v);
				} else {
					LNOCHANCE;
				}
			} break;
			case BYTE_SETINDEX: {
				Value t = cs->v[-3];
				Value k = cs->v[-2];
				Value v = cs->v[-1];
				cs->v -= 2;

				if (t.tag == VALUE_STRING) {
					if (v.tag == VALUE_LONG) {
						t.s->string[k.i] = v.i;
					} else {
						LNOCHANCE;
					}
				} else
				if (t.tag == VALUE_TABLE) {
					langH_insert(t.t,k,v);
				} else {
					LNOCHANCE;
				}
			} break;
			case BYTE_FIELD: {
				cs->v -= 2;
				Value t,k;
				t = cs->v[0];
				k = cs->v[1];
				lang_pushvalue(cs,langH_lookup(t.t,k));
			} break;
			case BYTE_INDEX: {
				cs->v -= 2;
				Value t,k;
				t = cs->v[0];
				k = cs->v[1];
				if (t.tag == VALUE_STRING) {
					if (k.tag == VALUE_LONG) {
						lang_pushlong(cs,t.s->string[k.i]);
					} else {
						LNOCHANCE;
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
				Value o = cs->v[0];
				Value m = cs->v[1];

				LASSERT(m.tag == VALUE_STRING);
				LASSERT(ttisobj(o.tag));
				Binding d = lfndmetafn(o.j,m.s);
				if (d != lnil) {
					lang_bind(cs,o.j,d,b.x,b.y);
				}
			} break;
			case BYTE_CALL: {
				Value v = *(-- cs->v);
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
				Value x = cs->v[0];
				if (x.tag == VALUE_LONG || x.tag == VALUE_REAL) {
					lang_pushlong(cs,0);
				} else {
					lang_pushlong(cs,x.i == 0);
				}
			} break;
			case BYTE_EQ:
			case BYTE_NEQ: {
				cs->v -= 2;
				Value x = cs->v[0];
				Value y = cs->v[1];

				cs->v->tag = VALUE_LONG;
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
				Value x = cs->v[0];\
				Value y = cs->v[1];\
				lang_pushlong(cs, ltolong(x) OP ltolong(y));\
			} break

			/* todo: make this better */
			#define CASE_BOP(OPCODE,OP) \
			case OPCODE : {\
				cs->v -= 2;\
				Value x = cs->v[0];\
				Value y = cs->v[1];\
				if (x.tag == VALUE_REAL || y.tag == VALUE_REAL) {\
					lnumber xx = ltoreal(x);\
					lnumber yy = ltoreal(y);\
					lang_pushnum(cs, xx OP yy);\
				} else {\
					llong xx = ltolong(x);\
					llong yy = ltolong(y);\
					lang_pushlong(cs, xx OP yy);\
				}\
			} break

			case BYTE_LTEQ: {
				cs->v -= 2;
				Value x = cs->v[0];
				Value y = cs->v[1];
				if (x.tag == VALUE_REAL || y.tag == VALUE_REAL) {
					lang_pushlong(cs, ltoreal(x) <= ltoreal(y));
				} else {
					lang_pushlong(cs, ltolong(x) <= ltolong(y));
				}
			} break;
			case BYTE_LT: {
				cs->v -= 2;
				Value x = cs->v[0];
				Value y = cs->v[1];
				if (x.tag == VALUE_REAL || y.tag == VALUE_REAL) {
					lang_pushlong(cs, ltoreal(x) < ltoreal(y));
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

				LNOCHANCE;
			} break;
		}
		if (cs->v < c->l+fn.nlocals) LNOCHANCE;
	}

	leave:
	return c->y;
}



lapi llong lang_poplong(Runtime *c) {
	Value v = *(-- c->v);
	LASSERT(v.tag == VALUE_LONG);
	return v.i;
}


lapi llong lang_leftover(Runtime *c) {
	return c->v - c->f->l;
}


lapi Value lang_load(Runtime *c, int x) {
	return c->f->l[x];
}


lapi String *lang_loadS(Runtime *c, int x) {
	Value v = c->f->l[x];
	LASSERT(v.tag == VALUE_STRING);
	return v.s;
}


lapi Closure *lang_loadcl(Runtime *c, int x) {
	Value v = c->f->l[x];
	LASSERT(v.tag == VALUE_FUNC);
	return v.f;
}


lapi llong lang_loadlong(Runtime *c, int x) {
	Value v = c->f->l[x];
	if (v.tag == VALUE_REAL) {
		return (llong) v.n;
	}
	LASSERT(v.tag == VALUE_LONG);
	return v.i;
}


lapi lnumber lang_loadnum(Runtime *c, llocalid x) {
	Value v = c->f->l[x];
	if (v.tag == VALUE_LONG) return (lnumber) v.i;
	LASSERT(v.tag == VALUE_REAL);
	return v.n;
}


lapi Handle lang_loadhandle(Runtime *c, llocalid x) {
	Value v = c->f->l[x];
	LASSERT(v.tag == VALUE_HANDLE);
	return v.h;
}


void lang_pushvalue(Runtime *c, Value v) {
	LASSERT(c->v - c->s < c->z);
	*(c->v ++) = v;
}


void lang_pushnil(Runtime *c) {
	c->v->tag = VALUE_NONE;
	++ c->v;
}


void lang_pushlong(Runtime *c, llong i) {
	c->v->tag = VALUE_LONG;
	c->v->i = i;
	++ c->v;
}


void lang_pushnum(Runtime *c, lnumber n) {
	c->v->tag = VALUE_REAL;
	c->v->n = n;
	++ c->v;
}


void lang_pushhandle(Runtime *c, Handle h) {
	c->v->tag = VALUE_HANDLE;
	c->v->h = h;
	++ c->v;
}


void lang_pushString(Runtime *c, String *s) {
	c->v->tag = VALUE_STRING;
	c->v->s = s;
	++ c->v;
}


void lang_pushclosure(Runtime *c, Closure *f) {
	c->v->tag = VALUE_FUNC;
	c->v->f = f;
	++ c->v;
}


void lang_pushtable(Runtime *c, Table *t) {
	c->v->tag = VALUE_TABLE;
	c->v->t = t;
	++ c->v;
}


void lang_pushbinding(Runtime *c, Binding b) {
	c->v->tag = VALUE_BINDING;
	c->v->c = b;
	++ c->v;
}


Table *lang_pushnewtable(Runtime *c) {
	Table *t = langH_new(c);
	lang_pushtable(c,t);
	return t;
}


String *lang_pushnewS(Runtime *c, char const *junk) {
	String *s = langS_new(c,junk);
	lang_pushString(c,s);
	return s;
}


Closure *lang_pushnewcl(Runtime *c, Proto fn) {
	Closure *cl = langF_newclosure(c,fn);
	LASSERT(lang_leftover(c) >= fn.ncaches);
	c->v -= fn.ncaches;
	int i;
	for (i=0; i<fn.ncaches; ++i) {
		cl->caches[i] = c->v[i];
	}
	lang_pushclosure(c,cl);
	return cl;
}

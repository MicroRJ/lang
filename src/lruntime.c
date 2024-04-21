/*
** See Copyright Notice In lang.h
** lruntime.c
** Runtime
*/


lnumber ltonumber(lValue v) {
	return v.tag == TAG_INT ? (lnumber) v.i : v.n;
}


llongint ltolong(lValue v) {
	return v.tag == TAG_NUM ? (llongint) v.n : v.i;
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


int langR_typecheck(lRuntime *R, lbyteid id, llocalid loc, lvaluetag x, lvaluetag y) {
	if (x != y) {
		langR_error(R,id,S_tpf("$%i, expected %s, instead got %s",loc,tag2s[x],tag2s[y]));
	}
	return x == y;
}


int lang_rootcall(lRuntime *R, llocalid rxy, int nx, int ny) {
	return lang_call(R,lnil,rxy,rxy,nx,ny);
}


int lang_call(lRuntime *R, lObject *obj, llocalid rx, llocalid ry, int nx, int ny) {
	LASSERT((R->top - R->call->base) >= 0);
	/* clear the locals? */
	lCallFrame *caller = R->call;
	lValue fn = caller->locals[rx];
	lValue *locals = caller->locals + rx + 1;
	/* top always points to one past locals,
	so far we only have nx argument locals,
	top is later incremented to match nlocals */
	lValue *top  = locals + nx;
	lCallFrame call = {0};
	call.caller = caller;
	call.top = R->top;
	call.obj = obj;
	call.locals = locals;
	call.rx = rx; call.ry = ry;
	call.nx = nx; call.ny = ny;
	if (fn.tag == TAG_CLS) {
		call.cl = fn.f;
		/* increment top to fill the function's locals */
		/* todo: replace with faster memset? */
		for (; nx < call.cl->fn.nlocals; ++ nx) {
			top->tag = TAG_NIL;
			top->i   = 0;
			top ++;
		}
	}
	R->top = top;
	R->call = &call;
	llocalid nyield = 0;
	if (fn.tag == TAG_CLS) {
		nyield = lang_resume(R);
	} else
	if (fn.tag == TAG_BID) {
		if (fn.c != lnil) {
			nyield = fn.c(R);
			/* ensure that the results were pushed to the stack */
			LASSERT(nyield <= (R->top - call.locals));
			/* todo: we only do this to not have to make
			the user write to rx directly? */
			/* hoist results */
			for (int p = 0; p < MIN(nyield,ny); ++ p) {
				caller->locals[ry] = R->top[p-nyield];
			}
		}
	} else {
		nyield = -1;
		langR_error(R,NO_BYTE,"not a function");
	}
	/* finally restore stack */
	R->call = caller;
	R->top = call.top;
	return nyield;
}


/*
** Loads an expression from source string.
** The expression is converted to a function
** and is called* as a root function, can only
** reference global expressions.
** The expression can be a function itself,
** in which case you may pass in arguments.
** * maybe we should instead return the function
** and not call it.
*/
int lang_loadexpr(lRuntime *R, lString *contents, llocalid rxy, int ny) {
	lModule *M = R->M;
	FileState fs = {0};
	fs.R = R;
	fs.M = M;
	fs.bytes = M->nbytes;
	fs.filename = "(anonymous)";
	fs.contents = contents->c;
	fs.thischar = contents->c;
	fs.linechar = contents->c;
	fs.linenumber = 1;

	/* kick start by lexing the first two tokens */
	langX_yield(&fs);
	langX_yield(&fs);

	FileFunc fn = {0};
	langY_beginfn(&fs,&fn,fs.tk.line);
	lnodeid id = langY_loadexpr(&fs);
	langL_yield(&fs,fs.tk.line,id);
	langY_closefn(&fs);

	/* todo: this is temporary, please remove this or make
	some sort of object out of it... */
	lFile fl = {0};
	fl.bytes = fn.bytes;
	fl.nbytes = M->nbytes - fn.bytes;
	fl.lines = contents->c;
	fl.nlines = strlen(contents->c);
	langA_varadd(M->files,fl);

	lProto p = {0};
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes = M->nbytes - fn.bytes;

	/* base register becomes closure */
	R->frame->locals[rxy].tag = TAG_CLS;
	R->frame->locals[rxy].f   = langF_newclosure(R,p);

	int nyield = lang_rootcall(R,rxy,0,ny);
	return nyield;
}


/*
** Loads a file and calls its function,
** returns the number of results.
*/
int lang_loadfile(lRuntime *R, FileState *fs, lString *filename, llocalid x, int y) {

	if (filename == lnil) filename = lang_checkString(R,x);

	char *contents;
	Error error = sys_loadfilebytes(lHEAP,&contents,filename->string);
	if (LFAILED(error)) return -1;

	lModule *M = R->M;
	fs->R = R;
	fs->M = M;
	fs->bytes = M->nbytes;
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
	fl.nbytes = M->nbytes - fn.bytes;
	fl.name = filename->c;
	fl.lines = contents;
	fl.nlines = strlen(contents);
	fl.pathondisk = filename->c;
	langA_varadd(M->files,fl);

	lProto p = {0};
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes = M->nbytes - fn.bytes;

	R->frame->locals[x].tag = TAG_CLS;
	R->frame->locals[x].f   = langF_newclosure(R,p);
	if (R->top == R->call->locals) __debugbreak(); // ++ R->top;
	int nyield = lang_call(R,lnil,x,x,0,y);

	/* todo: lines? */
	// langM_dealloc(lHEAP,contents);
	return nyield;
}


int lang_resume(lRuntime *R) {
	/* todo: these names are deprecated */
	lCallFrame *c = R->f;
	lCallFrame *frame = R->frame;
	lModule *md = R->md;
	//
	lCallFrame *call = R->call;
	lClosure *cl = c->cl;
	lProto fn = cl->fn;
	lCallFrame *caller = call->caller;

	while (c->j < fn.nbytes) {
		llongint jp = c->j ++;
		llongint bc = fn.bytes + jp;
		lBytecode b = md->bytes[bc];

#ifdef _DEBUG
		if (R->logging) bytefpf(md,stdout,jp,b);
		if (R->debugbreak) __debugbreak();
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
				/* check that we don't exceed number of
				expected outputs */
				int ny = MIN(b.z,call->ny);
				for (llocalid y = 0; y < ny; ++y) {
					caller->locals[call->ry+y] = call->locals[b.y+y];
				}
				call->ny = ny;
				call->j = jp + b.x;
			} break;
			case BC_STKGET: {
				langR_typecheck(R,bc,0,TAG_INT,frame->base[b.y].tag);
				frame->base[b.x] = frame->base[frame->base[b.y].i];
			} break;
			case BC_STKLEN: {
				frame->base[b.x].tag = TAG_INT;
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
			case BC_LOADTHIS: {
				frame->base[b.x].tag = ttobj2val(frame->obj->type);
				frame->base[b.x].x_obj = frame->obj;
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
				frame->base[b.x].tag = TAG_INT;
				frame->base[b.x].i   = md->ki[b.y];
			} break;
			case BC_LOADNUM: {
				frame->base[b.x].tag = TAG_NUM;
				frame->base[b.x].n   = md->kn[b.y];
			} break;
			case BC_LOADCACHED: {
				LASSERT(b.y >= 0 && b.y < fn.ncaches);
				frame->base[b.x] = cl->caches[b.y];
			} break;
			case BC_CLOSURE: {
				LASSERT(b.y >= 0 && b.y < langA_varlen(md->p));
				lProto p = md->p[b.y];
				lClosure *ncl = langF_newclosure(R,p);
				for (int i = 0; i < p.ncaches; ++i) {
					ncl->caches[i] = frame->base[b.x+i];
				}
				frame->base[b.x].tag = TAG_CLS;
				frame->base[b.x].f   = ncl;
			} break;
			case BC_TABLE: {
				frame->base[b.x].tag = TAG_TAB;
				frame->base[b.x].t   = langH_new(R);
			} break;
			case BC_TYPEGUARD: {
				langR_typecheck(R,bc,b.x,b.y,frame->base[b.x].tag);
			} break;
			case BC_INDEX: case BC_FIELD: {
				#define LNIL (lValue){TAG_NIL}
				if (frame->base[b.y].tag == TAG_TAB) {
					frame->base[b.x] = langH_lookup(frame->base[b.y].t,frame->base[b.z]);
				} else frame->base[b.x] = LNIL;
			} break;
			case BC_SETINDEX: case BC_SETFIELD: {
				if (langR_typecheck(R,bc,b.x,TAG_TAB,frame->base[b.x].tag)) {
					langH_insert(frame->base[b.x].t,frame->base[b.y],frame->base[b.z]);
				} else LNOBRANCH;
			} break;
			case BC_SETMETACLASS: {
				if (ttisobj(frame->base[b.x].tag) && ttisobj(frame->base[b.y].tag)) {
					frame->base[b.x].x_obj->metaclass = frame->base[b.y].x_obj->metaclass;
				} else LNOBRANCH;
			} break;
			case BC_SETMETAFIELD: {
				if (ttisobj(frame->base[b.x].tag)) {
					langH_insert(frame->base[b.x].x_obj->metaclass,frame->base[b.y],frame->base[b.z]);
				} else LNOBRANCH;
			} break;
			case BC_METAFIELD: {
				lValue *yy = &frame->base[b.y];
				if (ttisobj(yy->tag)) {
					frame->base[b.x] = langH_lookup(yy->j->metaclass,frame->base[b.z]);
				} else {
					frame->base[b.x] = (lValue){TAG_NIL};
					langR_error(R,bc,S_tpf("'%s': not an object", tag2s[yy->tag]));
				}
			} break;
			case BC_METACALL: {
				R->j = bc;
				lang_call(R,frame->base[b.x].j,b.x+1,b.x,b.y,b.z);
			} break;
			case BC_CALL: {
				R->j = bc;
				lang_rootcall(R,b.x,b.y,b.z);
			} break;
			case BC_ISNIL: {
				lValue x = frame->base[b.y];
				lbool nan = x.tag != TAG_INT && x.tag != TAG_NUM;
				frame->base[b.x].tag = TAG_INT;
				frame->base[b.x].i   = x.tag == TAG_NIL || (nan && x.i == 0);
			} break;
			case BC_EQ: case BC_NEQ: {
				lValue x = frame->base[b.y];
				lValue y = frame->base[b.z];
				lbool eq = lfalse;
				/* todo: fix nil comparisons */
				if (x.tag == TAG_NIL || y.tag == TAG_NIL) {
					eq = tisnil(x) == tisnil(y);
				} else
				if (x.tag == TAG_STR && y.tag == TAG_STR) {
					eq = langS_eq(x.s,y.s);
				} else {
					eq = x.i == y.i;
				}
				if (b.k == BC_NEQ) eq = !eq;

				frame->base[b.x].tag = TAG_INT;
				frame->base[b.x].i   = eq;
			} break;


			/* todo: make this better */
			#define CASE_IBOP(OPNAME,OP) \
			case OPNAME : {\
				c->l[b.x].tag = TAG_INT;\
				c->l[b.x].i   = ltolong(c->l[b.y]) OP ltolong(c->l[b.z]);\
			} break

			/* todo: make this better */
			#define CASE_BOP(OPCODE,OP) \
			case OPCODE : {\
				if (c->l[b.y].tag == TAG_NUM || c->l[b.z].tag == TAG_NUM) {\
					c->l[b.x].tag = TAG_NUM;\
					c->l[b.x].n   = ltonumber(c->l[b.y]) OP ltonumber(c->l[b.z]);\
				} else {\
					c->l[b.x].tag = TAG_INT;\
					c->l[b.x].i   = ltolong(c->l[b.y]) OP ltolong(c->l[b.z]);\
				}\
			} break

			case BC_LTEQ: {
				lValue x = c->l[b.y];
				lValue y = c->l[b.z];
				if (x.tag == TAG_NUM || y.tag == TAG_NUM) {
					c->l[b.x].tag = TAG_INT;
					c->l[b.x].i   = ltonumber(x) <= ltonumber(y);
				} else {
					c->l[b.x].tag = TAG_INT;
					c->l[b.x].i   = ltolong(x) <= ltolong(y);
				}
			} break;
			case BC_LT: {
				lValue x = c->l[b.y];
				lValue y = c->l[b.z];
				if (x.tag == TAG_NUM || y.tag == TAG_NUM) {
					c->l[b.x].tag = TAG_NUM;
					c->l[b.x].n   = ltonumber(x) < ltonumber(y);
				} else {
					c->l[b.x].tag = TAG_INT;
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


lapi llongint lang_toplen(lRuntime *R) {
	return R->top - R->call->locals;
}


lapi lValue lang_load(lRuntime *R, llocalid x) {
	return R->call->locals[x];
}


lapi llongint lang_poplong(lRuntime *R) {
	LASSERT(lang_toplen(R) >= 1);
	lValue v = *(-- R->top);
	langR_typecheck(R,NO_BYTE,-1,TAG_INT,v.tag);
	return v.i;
}


lapi lString *lang_getstr(lRuntime *R, llocalid x) {
	lValue v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_STR) {
		langR_error(R,NO_BYTE,S_tpf("expected string at local %i",x));
		LNOBRANCH;
	}
	return v.s;
}


lapi lObject *lang_getobj(lRuntime *R, llocalid x) {
	lValue v = R->call->locals[x];
	if (v.tag != TAG_NIL && !ttisobj(v.tag)) {
		langR_error(R,NO_BYTE,S_tpf("expected object at local %i",x));
		LNOBRANCH;
	}
	return v.x_obj;
}


lapi lTable *lang_gettab(lRuntime *R, llocalid x) {
	lValue v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_TAB) {
		langR_error(R,NO_BYTE,S_tpf("expected table at local %i",x));
		LNOBRANCH;
	}
	return v.x_tab;
}


lapi lClosure *lang_loadcl(lRuntime *R, llocalid x) {
	lValue v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_CLS) {
		langR_error(R,NO_BYTE,S_tpf("expected closure at local %i",x));
		LNOBRANCH;
	}
	return v.f;
}


lapi lsysobj lang_getsysobj(lRuntime *R, llocalid x) {
	lValue v = R->call->locals[x];
	if (v.tag != TAG_NIL && v.tag != TAG_SYS) {
		langR_error(R,NO_BYTE,S_tpf("expected system object at local %i",x));
		LNOBRANCH;
	}
	return v.h;
}


lapi void lang_checkcl(lRuntime *R, llocalid x) {
	LASSERT(R->call->locals[x].tag == TAG_CLS);
}


lapi lString *lang_checkString(lRuntime *R, llocalid x) {
	LASSERT(R->call->locals[x].tag == TAG_STR);
	return R->call->locals[x].s;
}


lapi llongint lang_getlong(lRuntime *R, int x) {
	lValue v = R->call->locals[x];
	if (v.tag == TAG_NUM) {
		return (llongint) v.n;
	}
	LASSERT(v.tag == TAG_INT);
	return v.i;
}


lapi lnumber lang_getnum(lRuntime *R, llocalid x) {
	lValue v = R->call->locals[x];
	if (v.tag == TAG_INT) return (lnumber) v.i;
	if (v.tag != TAG_NUM) {
		langR_error(R,NO_BYTE,S_tpf("expected number at local %i",x));
		LNOBRANCH;
	}
	return v.n;
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


void lang_pushnil(lRuntime *R) {
	R->top->tag = TAG_NIL;
	++ R->top;
}


void lang_pushlong(lRuntime *R, llongint i) {
	R->top->tag = TAG_INT;
	R->top->i = i;
	++ R->top;
}


void lang_pushnum(lRuntime *R, lnumber n) {
	R->top->tag = TAG_NUM;
	R->top->n = n;
	++ R->top;
}


void lang_pushsysobj(lRuntime *R, lsysobj h) {
	R->top->tag = TAG_SYS;
	R->top->h = h;
	++ R->top;
}


void lang_pushString(lRuntime *R, lString *s) {
	R->top->tag = TAG_STR;
	R->top->s = s;
	++ R->top;
}


llocalid lang_pushclosure(lRuntime *R, lClosure *cl) {
	R->top->tag = TAG_CLS;
	R->top->f   = cl;
	return R->top ++ - R->call->locals;
}


void lang_pushtable(lRuntime *R, lTable *t) {
	R->top->tag = TAG_TAB;
	R->top->t = t;
	++ R->top;
}


void lang_pushbinding(lRuntime *R, lBinding b) {
	R->top->tag = TAG_BID;
	R->top->c = b;
	++ R->top;
}


lTable *lang_pushnewtable(lRuntime *R) {
	lTable *t = langH_new(R);
	lang_pushtable(R,t);
	return t;
}


lString *lang_pushnewS(lRuntime *R, char const *junk) {
	lString *s = langS_new(R,junk);
	lang_pushString(R,s);
	return s;
}


llocalid lang_pushnewclosure(lRuntime *R, lProto fn) {
	lClosure *cl = langF_newclosure(R,fn);
	LASSERT(lang_toplen(R) >= fn.ncaches);
	R->top -= fn.ncaches;
	int i;
	for (i=0; i<fn.ncaches; ++i) {
		cl->caches[i] = R->top[i];
	}
	return lang_pushclosure(R,cl);
}

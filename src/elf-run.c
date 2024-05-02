/*
** See Copyright Notice In elf.h
** elf-run.c
** Runtime
*/


void elf_runini(elf_ThreadState *R, elf_Module *M) {
	R->M = M;
	R->logging = lfalse;
	R->stklen = 4096;
	R->stk = R->top = elf_clearalloc(lHEAP,sizeof(elf_val)*R->stklen);
	R->metatable_str = elf_newstrmetatab(R);
	R->metatable_tab = elf_newtabmetatab(R);
	elf_CallFrame Y = {0};
	Y.base = R->top;
	R->frame = &Y;
	M->globals = elf_newloctab(R);
	R->cache.x = elf_newlocstr(R,"x");
	R->cache.y = elf_newlocstr(R,"y");
	R->cache.z = elf_newlocstr(R,"z");
	R->cache.w = elf_newlocstr(R,"w");
	R->cache.width = elf_newlocstr(R,"width");
	R->cache.height = elf_newlocstr(R,"height");
	R->cache.__add = elf_newlocstr(R,"__add");
	R->cache.__sub = elf_newlocstr(R,"__sub");
	R->cache.__mul = elf_newlocstr(R,"__mul");
	R->cache.__div = elf_newlocstr(R,"__div");
	R->cache.__add1 = elf_newlocstr(R,"__add1");
	R->cache.__sub1 = elf_newlocstr(R,"__sub1");
	R->cache.__mul1 = elf_newlocstr(R,"__mul1");
	R->cache.__div1 = elf_newlocstr(R,"__div1");

	#if !defined(ELF_NOLIBS)
	/* todo: eventually we'll load these from the
	source code, each lib will be built independently,
	or maybe we can add a flag so that we don't load
	these libraries. */
	netlib_load(R);
	elflib_load(R);
	tstlib_load(R);
	crtlib_load(R);
	#endif
}



int elf_callfn(elf_ThreadState *R, llocalid rxy, int nx, int ny) {
	return elf_callex(R,lnil,rxy,rxy,nx,ny);
}


/* todo: this should be different, rx should be the destination
registers, and ry the input registers */
int elf_callexx(elf_ThreadState *R, elf_Object *obj, elf_val fn, llocalid rx, llocalid ry, int nx, int ny) {
	elf_CallFrame *caller = R->call;
	// elf_ensure(R->top - caller->locals+caller->cl->fn.nlocals > -1);
	elf_val *locals = caller->locals + rx;
	/* top always points to one past locals,
	so far we only have nx argument locals,
	top is later incremented to match nlocals */
	elf_val *top  = locals + nx;
	elf_CallFrame call = {0};
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
		nyield = elf_run(R);
	} else
	if (fn.tag == TAG_BID) {
		if (fn.c != lnil) {
			nyield = fn.c(R);
			/* ensure that the results were pushed to the stack */
			elf_ensure(nyield <= (R->top - call.locals));
			/* todo: we only do this to not have to make
			the user write to rx directly? */
			/* hoist results */
			for (int p = 0; p < MIN(nyield,ny); ++ p) {
				caller->locals[ry] = R->top[p-nyield];
			}
		}
	} else {
		nyield = -1;
		elf_throw(R,NO_BYTE,elf_tpf("'%s': is not a function",tag2s[fn.tag]));
	}
	/* finally restore stack */
	R->call = caller;
	R->top = call.top;
	return nyield;
}


int elf_callex(elf_ThreadState *R, elf_Object *obj, llocalid rx, llocalid ry, int nx, int ny) {
	elf_CallFrame *caller = R->call;
	return elf_callexx(R,obj,caller->locals[rx],rx+1,ry,nx,ny);
}


int elf_loadexpr(elf_ThreadState *R, elf_String *contents, llocalid rxy, int ny) {
	elf_Module *M = R->M;
	elf_FileState fs = {0};
	fs.R = R;
	fs.M = M;
	fs.bytes = M->nbytes;
	fs.filename = "(anonymous)";
	fs.contents = contents->c;
	fs.thischar = contents->c;
	fs.linechar = contents->c;
	fs.linenumber = 1;

	/* kick start by lexing the first two tokens */
	elf_lexone(&fs);
	elf_lexone(&fs);

	elf_FileFunc fn = {0};
	elfY_beginfn(&fs,&fn,fs.tk.line);
	lnodeid id = elf_fsloadexpr(&fs);
	langL_yield(&fs,fs.tk.line,id);
	elfY_closefn(&fs);

	/* todo: this is temporary, please remove this or make
	some sort of object out of it... */
	elf_File fl = {0};
	fl.bytes = fn.bytes;
	fl.nbytes = M->nbytes - fn.bytes;
	fl.lines = contents->c;
	fl.nlines = strlen(contents->c);
	elf_varadd(M->files,fl);

	elf_Proto p = {0};
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes = M->nbytes - fn.bytes;

	return elf_callexx(R,lnil,elf_valcls(elf_newcls(R,p)),rxy,rxy,0,ny);
}



int elf_loadcode(elf_ThreadState *R, elf_FileState *fs, elf_String *filename, const llocalid rxy, int ny, char *contents) {
	if (filename == lnil) {
	/* todo: don't like this */
	 	filename = elf_checkstr(R,rxy);
	}
	if (contents == lnil) {
		return -1;
	}

	elf_Module *M = R->M;
	fs->R = R;
	fs->M = M;
	fs->bytes = M->nbytes;
	fs->filename = filename->c;
	fs->contents = contents;
	fs->thischar = contents;
	fs->linechar = contents;
	fs->linenumber = 1;

	/* kick start by lexing the first two tokens */
	elf_lexone(fs);
	elf_lexone(fs);

	elf_FileFunc fn = {0};
	elfY_beginfn(fs,&fn,fs->tk.line);
	while (!elf_testtk(fs,0)) elf_fsloadstat(fs);
	elfY_closefn(fs);

	/* todo: this is temporary, please remove this or make
	some sort of object out of it... */
	elf_File fl = {0};
	fl.bytes = fn.bytes;
	fl.nbytes = M->nbytes - fn.bytes;
	fl.name = filename->c;
	fl.lines = contents;
	fl.nlines = strlen(contents);
	fl.pathondisk = filename->c;
	elf_varadd(M->files,fl);

	elf_Proto p = {0};
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes = M->nbytes - fn.bytes;

	elf_val cls = elf_valcls(elf_newcls(R,p));
	return elf_callexx(R,lnil,cls,rxy,rxy,0,ny);
}


int elf_loadfile(elf_ThreadState *R, elf_FileState *fs, elf_String *name, llocalid x, int y) {
	if (name == lnil) {
		name = elf_checkstr(R,x);
	}
	char *contents;
	Error error = sys_loadfilebytes(lHEAP,(void**)&contents,name->string);
	if (LFAILED(error)) {
		elf_logerror("'%s': could not read file",name->c);
		return -1;
	}
	return elf_loadcode(R,fs,name,x,y,contents);
}


/* todo: Can we add a failsafe system that attempts
to recover from failed instructions?
So any instructions that depend on a previous
one are skipped?
For instance, table:add(table:length()), here if
table is nil or not even a table, you have to skip
the call instruction and its arguments. */
int elf_run(elf_ThreadState *R) {
	/* todo: these names are deprecated */
	elf_CallFrame *c = R->f;
	elf_Module *md = R->md;
	//
	elf_CallFrame *call = R->call;
	elf_Closure *cl = call->cl;
	elf_Proto fn = cl->fn;
	elf_CallFrame *caller = call->caller;
	elf_val *locals = call->locals;
	elf_ensure((elf_int)(R->top - locals) >= fn.nlocals);

	while (call->j < fn.nbytes) {
		elf_int jp = call->j ++;
		elf_int bc = fn.bytes + jp;
		lBytecode b = md->bytes[bc];
		md->track[bc] ++;
#if defined(_DEBUG)
		if (R->logging || call->logging) elf_bytefpf(stdout,md,-1,jp,b);
		if (R->debugbreak) elf_debugger("elf-run: debugger break");
#endif

		switch (b.k) {
	case BC_LEAVE: {
		if (call->dl != lnil) {
			call-> j = call->dl->j;
			call->dl = call->dl->n;
		} else goto leave;
	} break;
	case BC_DELAY: {
		/* todo: can we make this better */
		ldelaylist *dl = elf_alloc(lHEAP,sizeof(ldelaylist));
		dl->n = c->dl;
		dl->j = c->j;
		c->dl = dl;

		elf_ensure(b.i >= 0);
		c->j = jp + b.i;
	} break;
	case BC_YIELD: {
		elf_ensure(b.x >= 0);
		/* check that we don't exceed number of
		expected outputs */
		int ny = MIN(b.z,call->ny);
		for (llocalid y = 0; y < ny; ++y) {
			caller->locals[call->ry+y] = locals[b.y+y];
		}
		call->ny = ny;
		call->j = jp + b.x;
	} break;
	case BC_STKGET: {
		elf_tycheck(R,bc,0,TAG_INT,locals[b.y].tag);
		locals[b.x] = locals[locals[b.y].i];
	} break;
	case BC_STKLEN: {
		locals[b.x].tag = TAG_INT;
		locals[b.x].i   = R->top - locals;
	} break;
	case BC_LOADFILE: {
		elf_FileState fs = {0};
		elf_loadfile(R,&fs,lnil,b.x,b.y);
	} break;
	case BC_J: {
		c->j = jp + b.i;
	} break;
	case BC_JZ: {
		if (locals[b.y].i == 0) call->j = jp + b.x;
	} break;
	case BC_JNZ: {
		if (locals[b.y].i != 0) call->j = jp + b.x;
	} break;
	case BC_LOADTHIS: {
		locals[b.x].tag = elf_objtotag(call->obj->type);
		locals[b.x].x_obj = call->obj;
	} break;
	case BC_RELOAD: {
		locals[b.x] = locals[b.y];
	} break;
	case BC_LOADGLOBAL: {
		locals[b.x] = md->g->v[b.y];
	} break;
	case BC_SETGLOBAL: {
		md->g->v[b.x] = call->base[b.y];
	} break;
	case BC_LOADNIL: {
		locals[b.x].tag = TAG_NIL;
		locals[b.x].i   = 0;
	} break;
	case BC_LOADINT: {
		locals[b.x].tag = TAG_INT;
		locals[b.x].i   = md->ki[b.y];
	} break;
	case BC_LOADNUM: {
		locals[b.x].tag = TAG_NUM;
		locals[b.x].n   = md->kn[b.y];
	} break;
	case BC_LOADCACHED: {
		elf_ensure(b.y >= 0 && b.y < fn.ncaches);
		locals[b.x] = cl->caches[b.y];
	} break;
	case BC_CLOSURE: {
		elf_ensure(b.y >= 0 && b.y < elf_varlen(md->p));
		elf_Proto p = md->p[b.y];
		elf_Closure *ncl = elf_newcls(R,p);
		for (int i = 0; i < p.ncaches; ++i) {
			ncl->caches[i] = locals[b.x+i];
		}
		locals[b.x].tag = TAG_CLS;
		locals[b.x].f   = ncl;
		// ncl->obj.gccolor = GC_WHITE;
	} break;
	case BC_TABLE: {
		/* ensure objects are created, then the
		local is renamed atomically, otherwise
		gc could trigger in between think the
		local is some other type */
		elf_Table *tab = elf_newtab(R);
		locals[b.x].tag = TAG_TAB;
		locals[b.x].x_tab = tab;
	} break;
	case BC_TYPEGUARD: {
		elf_tycheck(R,bc,b.x,b.y,locals[b.x].tag);
	} break;
	case BC_INDEX: case BC_FIELD: {
		if (locals[b.y].tag == TAG_STR) {
			elf_tycheck(R,bc,0,TAG_INT,locals[b.z].tag);
			locals[b.x].tag = TAG_INT;
			locals[b.x].i   = locals[b.y].s->c[locals[b.z].i];
		} else
		if (locals[b.y].tag == TAG_TAB) {
			locals[b.x] = elf_tablookup(locals[b.y].x_tab,locals[b.z]);
		} else locals[b.x] = (elf_val){TAG_NIL};
	} break;
	case BC_SETINDEX: case BC_SETFIELD: {
		if (elf_tycheck(R,bc,b.x,TAG_TAB,locals[b.x].tag)) {
			elf_tabset(locals[b.x].t,locals[b.y],locals[b.z]);
		} else LNOBRANCH;
	} break;
	case BC_SETMETATABLE: {
		elf_val xx = locals[b.x];
		elf_val yy = locals[b.y];
		if (elf_tagisobj(xx.tag) && yy.tag == TAG_TAB) {
			xx.x_obj->metatable = yy.x_tab;
		} else LNOBRANCH;
	} break;
	case BC_SETMETAFIELD: {
		elf_val xx = locals[b.x];
		if (elf_tagisobj(xx.tag)) {
			elf_val yy = locals[b.y];
			elf_val zz = locals[b.z];
			elf_tabset(xx.x_obj->metatable,yy,zz);
		} else elf_throw(R,bc,elf_tpf("'%s': not an object", tag2s[xx.tag]));
	} break;
	case BC_METAFIELD: {
		elf_val *yy = &locals[b.y];
		if (elf_tagisobj(yy->tag)) {
			locals[b.x] = elf_tablookup(yy->j->metatable,locals[b.z]);
		} else {
			locals[b.x] = (elf_val){TAG_NIL};
			elf_throw(R,bc,elf_tpf("'%s': not an object", tag2s[yy->tag]));
		}
	} break;
	case BC_METACALL: {
		R->j = bc;
		elf_callex(R,locals[b.x].x_obj,b.x+1,b.x,b.y,b.z);
		elf_ensure((elf_int)(R->top - locals) >= fn.nlocals);
	} break;
	case BC_CALL: {
		R->j = bc;
		elf_callfn(R,b.x,b.y,b.z);
		elf_ensure((elf_int)(R->top - locals) >= fn.nlocals);
	} break;
	case BC_ISNIL: {
		elf_val x = locals[b.y];
		elf_bool nan = x.tag != TAG_INT && x.tag != TAG_NUM;
		locals[b.x].tag = TAG_INT;
		locals[b.x].i   = x.tag == TAG_NIL || (nan && x.i == 0);
	} break;
	case BC_EQ: case BC_NEQ: {
		elf_val x = locals[b.y];
		elf_val y = locals[b.z];
		elf_bool eq = lfalse;
		/* todo: fix nil comparisons */
		if ((x.tag == TAG_NIL) || (y.tag == TAG_NIL)) {
			eq = elf_valisnil(x) == elf_valisnil(y);
		} else if ((x.tag == TAG_STR) && (y.tag == TAG_STR)) {
			eq = elf_streq(x.x_str,y.x_str);
		} else eq = x.i == y.i;
		if (b.k == BC_NEQ) eq = !eq;
		locals[b.x].tag = TAG_INT;
		locals[b.x].i   = eq;
	} break;
	/* todo: make this better */
	#define CASE_IBOP(OPNAME,OP) \
	case OPNAME : {\
		locals[b.x].tag = TAG_INT;\
		locals[b.x].x_int = elf_toint(c->l[b.y]) OP elf_toint(c->l[b.z]);\
	} break
	// case BC_ADD: {
	// 	elf_val xx = locals[b.y];
	// 	elf_val yy = locals[b.z];
	// 	if ((xx.tag == TAG_NUM) || (yy.tag == TAG_NUM)) {
	// 		locals[b.x].tag = TAG_NUM;
	// 		locals[b.x].x_num = elf_tonum(xx) + elf_tonum(yy);
	// 	} else if ((xx.tag == TAG_INT) || (yy.tag == TAG_INT)) {
	// 		locals[b.x].tag = TAG_INT;
	// 		locals[b.x].x_int = elf_toint(xx) + elf_toint(yy);
	// 	} else if (elf_tagisobj(xx.tag) || elf_tagisobj(yy.tag)) {
	// 		elf_ensure((elf_int)(R->top - locals) >= fn.nlocals);
	// 		elf_val mfn = elf_tabget(xx.x_obj->metatable,elf_newlocstr(R,"__add"));
	// 		if (mfn.tag == TAG_CLS) {
	// 			locals[b.x] = yy;
	// 			elf_loginfo("__add(%i,%i)",b.x,b.x);
	// 			int ny = elf_callexx(R,xx.x_obj,mfn,b.x,b.x,1,1);
	// 			if (ny < 1) elf_throw(R,bc,"function must return atleast one value");
	// 		} else elf_throw(R,bc,elf_tpf("'%s': overload is %s, not a function","__add",tag2s[mfn.tag]));
	// 	} else LNOBRANCH;
	// } break;
	/* todo: make this better */
	#define CASE_BOP(OPCODE,OP,FN,FN1) \
	case OPCODE : {\
		elf_val xx = locals[b.y];\
		elf_val yy = locals[b.z];\
		if (elf_tagisobj(xx.tag)) {\
			elf_String *mfname = FN;\
			if (!elf_tagisobj(yy.tag)) mfname = FN1;\
			elf_val mfield = elf_tabget(xx.x_obj->metatable,mfname);\
			if (mfield.tag == TAG_CLS) {\
				locals[b.x] = yy;\
				int ny = elf_callexx(R,xx.x_obj,mfield,b.x,b.x,1,1);\
				if (ny < 1) elf_throw(R,bc,"function must return atleast one value");\
			} else elf_throw(R,bc,elf_tpf("'%s': overload is %s, not a function",mfname->c,tag2s[mfield.tag]));\
		} else if ((xx.tag == TAG_NUM) || (yy.tag == TAG_NUM)) {\
			locals[b.x].tag = TAG_NUM;\
			locals[b.x].x_num = elf_tonum(xx) OP elf_tonum(yy);\
		} else if ((xx.tag == TAG_INT) || (yy.tag == TAG_INT)) {\
			locals[b.x].tag = TAG_INT;\
			locals[b.x].x_int = elf_toint(xx) OP elf_toint(yy);\
		} else LNOBRANCH;\
	} break
	case BC_LTEQ: {
		elf_val xx = locals[b.y];
		elf_val yy = locals[b.z];
		if ((xx.tag == TAG_NUM) || (yy.tag == TAG_NUM)) {
			locals[b.x].tag = TAG_INT;
			locals[b.x].i   = elf_tonum(xx) <= elf_tonum(yy);
		} else {
			locals[b.x].tag = TAG_INT;
			locals[b.x].i   = elf_toint(xx) <= elf_toint(yy);
		}
	} break;
	case BC_LT: {
		elf_val x = c->l[b.y];
		elf_val y = c->l[b.z];
		if (x.tag == TAG_NUM || y.tag == TAG_NUM) {
			c->l[b.x].tag = TAG_NUM;
			c->l[b.x].n   = elf_tonum(x) < elf_tonum(y);
		} else {
			c->l[b.x].tag = TAG_INT;
			c->l[b.x].i   = elf_toint(x) < elf_toint(y);
		}
	} break;
	CASE_IBOP(BC_SHL,  <<);
	CASE_IBOP(BC_SHR,  >>);
	CASE_IBOP(BC_XOR,   ^);
	CASE_IBOP(BC_MOD,   %);
	/* todo: could we cache these strings */
	CASE_BOP(BC_ADD, +, R->cache.__add, R->cache.__add1);
	CASE_BOP(BC_SUB, -, R->cache.__sub, R->cache.__sub1);
	CASE_BOP(BC_MUL, *, R->cache.__mul, R->cache.__mul1);
	CASE_BOP(BC_DIV, /, R->cache.__div, R->cache.__div1);
	#undef CASE_BOP
	default: {
		LNOBRANCH;
	} break;
		}
		elf_ensure((elf_int)(R->top - locals) >= fn.nlocals);
		int BREAKPOINT;
		BREAKPOINT = 0; (void) BREAKPOINT;
	}

	leave:
	return c->y;
}


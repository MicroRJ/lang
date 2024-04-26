/*
** See Copyright Notice In elf.h
** lruntime.c
** Runtime
*/


elf_num elf_iton(elf_val v) {
	return v.tag == TAG_INT ? (elf_num) v.i : v.n;
}


elf_int elf_ntoi(elf_val v) {
	return v.tag == TAG_NUM ? (elf_int) v.n : v.i;
}


int fndfile(elf_Module *md, llineid line) {
	elf_File *files = md->files;
	int nfiles = elf_arrlen(files);
	for (int x = 0; x < nfiles; ++ x) {
		if (line < files[x].lines) continue;
		if (line > files[x].lines+files[x].nlines-1) continue;
		return x;
	}
	return -1;
}


void elf_throw(lRuntime *R, lbyteid id, char *error) {
	elf_Module *md = R->md;
	if (id == NO_BYTE) id = R->j;
	llineid line = md->lines[id];
	int fileid = fndfile(md,line);
	if (fileid != -1) {
		elf_File *file = &md->files[fileid];
		elfX_error2(file->name,file->lines,line,error);
	}
}


int langR_typecheck(lRuntime *R, lbyteid id, llocalid loc, lvaluetag x, lvaluetag y) {
	if (x != y) {
		elf_throw(R,id,S_tpf("$%i, expected %s, instead got %s",loc,tag2s[x],tag2s[y]));
	}
	return x == y;
}


int elf_rootcall(lRuntime *R, llocalid rxy, int nx, int ny) {
	return elf_callfn(R,lnil,rxy,rxy,nx,ny);
}


int elf_callfn(lRuntime *R, elf_Object *obj, llocalid rx, llocalid ry, int nx, int ny) {
	elf_CallFrame *caller = R->call;
	elf_val fn = caller->locals[rx];
	elf_val *locals = caller->locals + rx + 1;
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
			elf_assert(nyield <= (R->top - call.locals));
			/* todo: we only do this to not have to make
			the user write to rx directly? */
			/* hoist results */
			for (int p = 0; p < MIN(nyield,ny); ++ p) {
				caller->locals[ry] = R->top[p-nyield];
			}
		}
	} else {
		nyield = -1;
		elf_throw(R,NO_BYTE,"not a function");
	}
	/* finally restore stack */
	R->call = caller;
	R->top = call.top;
	return nyield;
}


int elf_loadexpr(lRuntime *R, elf_String *contents, llocalid rxy, int ny) {
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
	elf_yieldtk(&fs);
	elf_yieldtk(&fs);

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
	elf_arradd(M->files,fl);

	elf_Proto p = {0};
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes = M->nbytes - fn.bytes;

	/* base register becomes closure */
	R->call->locals[rxy].tag = TAG_CLS;
	R->call->locals[rxy].f   = elf_newcl(R,p);
	return elf_rootcall(R,rxy,0,ny);
}


int elf_loadfile(lRuntime *R, elf_FileState *fs, elf_String *filename, llocalid x, int y) {

	if (filename == lnil) filename = elf_checkstr(R,x);

	char *contents;
	Error error = sys_loadfilebytes(lHEAP,&contents,filename->string);
	if (LFAILED(error)) return -1;

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
	elf_yieldtk(fs);
	elf_yieldtk(fs);


	elf_FileFunc fn = {0};
	elfY_beginfn(fs,&fn,fs->tk.line);
	while (!elf_testtk(fs,0)) elfY_loadstat(fs);
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
	elf_arradd(M->files,fl);

	elf_Proto p = {0};
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes = M->nbytes - fn.bytes;

	R->frame->locals[x].tag = TAG_CLS;
	R->frame->locals[x].f   = elf_newcl(R,p);
	if (R->top == R->call->locals) __debugbreak(); // ++ R->top;
	int nyield = elf_callfn(R,lnil,x,x,0,y);

	/* todo: lines? */
	// elf_delmem(lHEAP,contents);
	return nyield;
}


/* todo: Can we add a failsafe system that attempts
to recover from failed instructions?
So any instructions that depend on a previous
one are skipped?
For instance, table:add(table:length()), here if
table if nil or not even a table, you have to skip
the call instruction and its arguments. */
int elf_run(lRuntime *R) {
	/* todo: these names are deprecated */
	elf_CallFrame *c = R->f;
	elf_CallFrame *frame = R->frame;
	elf_Module *md = R->md;
	//
	elf_CallFrame *call = R->call;
	elf_Closure *cl = call->cl;
	elf_Proto fn = cl->fn;
	elf_CallFrame *caller = call->caller;
	elf_val *locals = call->locals;

	while (c->j < fn.nbytes) {
		elf_int jp = c->j ++;
		elf_int bc = fn.bytes + jp;
		lBytecode b = md->bytes[bc];

#ifdef _DEBUG
		if (R->logging || call->logging) {
			bytefpf(md,stdout,jp,b);
		}
		if (R->debugbreak) __debugbreak();
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
				ldelaylist *dl = langM_alloc(lHEAP,sizeof(ldelaylist));
				dl->n = c->dl;
				dl->j = c->j;
				c->dl = dl;

				elf_assert(b.i >= 0);
				c->j = jp + b.i;
			} break;
			case BC_YIELD: {
				elf_assert(b.x >= 0);
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
				langR_typecheck(R,bc,0,TAG_INT,locals[b.y].tag);
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
				locals[b.x].tag = elf_tttotag(call->obj->type);
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
				elf_assert(b.y >= 0 && b.y < fn.ncaches);
				locals[b.x] = cl->caches[b.y];
			} break;
			case BC_CLOSURE: {
				elf_assert(b.y >= 0 && b.y < elf_arrlen(md->p));
				elf_Proto p = md->p[b.y];
				elf_Closure *ncl = elf_newcl(R,p);
				for (int i = 0; i < p.ncaches; ++i) {
					ncl->caches[i] = locals[b.x+i];
				}
				locals[b.x].tag = TAG_CLS;
				locals[b.x].f   = ncl;
			} break;
			case BC_TABLE: {
				/* ensure the object is created before
				we modify the value, because gc could
				trigger and actually attempt to collect
				this value */
				elf_tab *tab = elf_newtab(R);
				locals[b.x].tag = TAG_TAB;
				locals[b.x].t   = tab;
			} break;
			case BC_TYPEGUARD: {
				langR_typecheck(R,bc,b.x,b.y,locals[b.x].tag);
			} break;
			case BC_INDEX: case BC_FIELD: {
				if (locals[b.y].tag == TAG_STR) {
					langR_typecheck(R,bc,0,TAG_INT,locals[b.z].tag);
					locals[b.x].tag = TAG_INT;
					locals[b.x].i   = locals[b.y].s->c[locals[b.z].i];
				} else
				if (locals[b.y].tag == TAG_TAB) {
					locals[b.x] = elf_tablookup(locals[b.y].t,locals[b.z]);
				} else locals[b.x] = (elf_val){TAG_NIL};
			} break;
			case BC_SETINDEX: case BC_SETFIELD: {
				if (langR_typecheck(R,bc,b.x,TAG_TAB,locals[b.x].tag)) {
					elf_tabput(locals[b.x].t,locals[b.y],locals[b.z]);
				} else LNOBRANCH;
			} break;
			case BC_SETMETATABLE: {
				elf_val xx = locals[b.x];
				elf_val yy = locals[b.y];
				if (elf_tagisobj(xx.tag) && elf_tagisobj(yy.tag)) {
					xx.x_obj->metatable = yy.x_obj->metatable;
				} else LNOBRANCH;
			} break;
			case BC_SETMETAFIELD: {
				elf_val xx = locals[b.x];
				if (elf_tagisobj(xx.tag)) {
					elf_val yy = locals[b.y];
					elf_val zz = locals[b.z];
					elf_tabput(xx.x_obj->metatable,yy,zz);
				} else elf_throw(R,bc,S_tpf("'%s': not an object", tag2s[xx.tag]));
			} break;
			case BC_METAFIELD: {
				elf_val *yy = &locals[b.y];
				if (elf_tagisobj(yy->tag)) {
					locals[b.x] = elf_tablookup(yy->j->metatable,locals[b.z]);
				} else {
					locals[b.x] = (elf_val){TAG_NIL};
					elf_throw(R,bc,S_tpf("'%s': not an object", tag2s[yy->tag]));
				}
			} break;
			case BC_METACALL: {
				R->j = bc;
				elf_callfn(R,locals[b.x].j,b.x+1,b.x,b.y,b.z);
			} break;
			case BC_CALL: {
				R->j = bc;
				elf_rootcall(R,b.x,b.y,b.z);
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
				if (x.tag == TAG_NIL || y.tag == TAG_NIL) {
					eq = tisnil(x) == tisnil(y);
				} else
				if (x.tag == TAG_STR && y.tag == TAG_STR) {
					eq = elf_streq(x.s,y.s);
				} else {
					eq = x.i == y.i;
				}
				if (b.k == BC_NEQ) eq = !eq;

				locals[b.x].tag = TAG_INT;
				locals[b.x].i   = eq;
			} break;


			/* todo: make this better */
			#define CASE_IBOP(OPNAME,OP) \
			case OPNAME : {\
				c->l[b.x].tag = TAG_INT;\
				c->l[b.x].i   = elf_ntoi(c->l[b.y]) OP elf_ntoi(c->l[b.z]);\
			} break

			/* todo: make this better */
			#define CASE_BOP(OPCODE,OP) \
			case OPCODE : {\
				if (c->l[b.y].tag == TAG_NUM || c->l[b.z].tag == TAG_NUM) {\
					c->l[b.x].tag = TAG_NUM;\
					c->l[b.x].n   = elf_iton(c->l[b.y]) OP elf_iton(c->l[b.z]);\
				} else {\
					c->l[b.x].tag = TAG_INT;\
					c->l[b.x].i   = elf_ntoi(c->l[b.y]) OP elf_ntoi(c->l[b.z]);\
				}\
			} break

			case BC_LTEQ: {
				elf_val x = c->l[b.y];
				elf_val y = c->l[b.z];
				if (x.tag == TAG_NUM || y.tag == TAG_NUM) {
					c->l[b.x].tag = TAG_INT;
					c->l[b.x].i   = elf_iton(x) <= elf_iton(y);
				} else {
					c->l[b.x].tag = TAG_INT;
					c->l[b.x].i   = elf_ntoi(x) <= elf_ntoi(y);
				}
			} break;
			case BC_LT: {
				elf_val x = c->l[b.y];
				elf_val y = c->l[b.z];
				if (x.tag == TAG_NUM || y.tag == TAG_NUM) {
					c->l[b.x].tag = TAG_NUM;
					c->l[b.x].n   = elf_iton(x) < elf_iton(y);
				} else {
					c->l[b.x].tag = TAG_INT;
					c->l[b.x].i   = elf_ntoi(x) < elf_ntoi(y);
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


/*
** See Copyright Notice In elf.h
** (Y) lfile.c
** Parsing Stuff
*/



elf_bool elf_fscheckexpr(elf_FileState *fs, llineid line, lnodeid id) {
	if (id != NO_NODE) return lfalse;
	elf_lineerror(fs,line,"invalid expression");
	return ltrue;
}


elf_bool elf_testtk(elf_FileState *fs, ltokentype k) {
	return fs->tk.type == k;
}


elf_bool elf_testthentk(elf_FileState *fs, ltokentype k) {
	return fs->thentk.type == k;
}


/*
** Returns true whether there are no more tokens
** or whehter the current token is a match.
*/
elf_bool elfX_term(elf_FileState *fs, ltokentype k) {
	return fs->tk.type == TK_NONE || fs->tk.type == k;
}


elf_bool elfX_termeol(elf_FileState *fs) {
	return fs->tk.type == TK_NONE || fs->lasttk.eol == ltrue;
}


/*
** Consumes the current token only if it is a match,
** returning whether it was a match or not.
*/
elf_bool elf_picktk(elf_FileState *fs, ltokentype k) {
	return elf_testtk(fs,k) && (elf_yieldtk(fs), ltrue);
}


/*
** Same as pick, only this time there are two possibilities.
*/
elf_bool elf_choosetk(elf_FileState *fs, ltokentype x, ltokentype y) {
	return (elf_testtk(fs,x) || elf_testtk(fs,y)) && (elf_yieldtk(fs), ltrue);
}


/*
** Check whether the current token is a match,
** if so pick it, otherwise error.
*/
ltoken elf_taketk(elf_FileState *fs, int k) {
	ltoken tk = fs->tk;
	if (!elf_picktk(fs,k)) {
		elf_lineerror(fs,fs->tk.line,"expected '%s'\n",elfX_tokenintel[k].name);
	}
	return tk;
}


lentityid elfY_allocentity(elf_FileState *fs, llineid line) {
	elf_FileFunc *ff = fs->fn;
	lentityid id = {fs->nentities ++};
	if (elf_varlen(fs->entities) < fs->nentities) {
		elf_varaddi(fs->entities,1);
	}

	for (llocalid i = id.x; i < fs->nentities; ++i) {
		fs->entities[i].level = fs->level;
		fs->entities[i].line  = line;
		fs->entities[i].name  = 0;
		fs->entities[i].slot  = 0;
		fs->entities[i].enm   = lfalse;
	}
	// elf_lineerror(fs,line,"%i, fs=%i, ff=%i",id,fs->nentities,nentities);
	return id;
}


llocalid elfY_numentinlevl(elf_FileState *fs) {
	lentity *entities = fs->entities;
	int nentities = fs->nentities;
	int n;
	for (n = 0; n < nentities; ++ n) {
		if (entities[nentities-1-n].level < fs->level) {
			break;
		}
	}
	return n;
}


lnodeid elfY_numnodesinlev(elf_FileState *fs) {
	lNode *nodes = fs->nodes;
	lnodeid nnodes = fs->nnodes;
	lnodeid n;
	for (n = 0; n < nnodes; ++n) {
		if (nodes[nnodes-1-n].level < fs->level) {
			break;
		}
	}
	return n;
}


void elfY_leavelevel(elf_FileState *fs, llineid line) {
	fs->nentities -= elfY_numentinlevl(fs); /* close scope */
	fs->nnodes -= elfY_numnodesinlev(fs);
	fs->fn->xmemory -= 0; /* todo: deallocate leftover locals */
	/* ensure that we don't deallocate more than we can */
	elf_assert(fs->nentities >= fs->fn->entities);
	-- fs->level;
}


void elf_fsenterlevel(elf_FileState *fs) {
	++ fs->level;
}


void elfY_checkassign(elf_FileState *fs, llineid line, lnodeid x) {
	/* todo: welp, we can't do this anymore */
	// lNode n = fs->nodes[x];
	// if (n.k == NODE_LOCAL) {
	// 	lentity fnn = fs->entities[fs->fn->entities+n.x];
	// 	if (fnn.enm) {
	// 		elf_lineerror(fs,line,"enum value is constant, you're attempting to modify its value");
	// 	}
	// }
}


/*
** Looks for a captured entity within the function's captures
** and returns the index where the captured entity resides.
** the index is then used to emit instructions targeting
** captures, the first capture corresponds to index 0.
*/
int elfY_indexofentityincache(elf_FileFunc *fn, lentityid id) {
	elf_arrfori(fn->captures) {
		if (fn->captures[i].x == id.x) return i;
	}
	return -1;
}


/*
** Captures an entitity (if not already) from the enclosing
** function, storing a copy of the entity id in the function's
** captures.
*/
void elf_fscapent(elf_FileState *fs, elf_FileFunc *fn, lentityid id) {
	/* ensure the entity should actually be captured */
	elf_assert(id.x < fn->entities);
	elf_arrfori(fn->captures) {
		if (fn->captures[i].x == id.x) return;
	}
	elf_varadd(fn->captures,id);
}


/*
** Find the closest entity in order of lexical
** relevance, starting from the last declared entity.
** The id returned is an absolute index into fs->entities,
** you should make it relative to the current function.
** If the entity is outside of this function, then it caches it.
*/
lentityid elf_fsfndent(elf_FileState *fs, llineid line, char *name) {
	elf_FileFunc *fn = fs->fn;
	for (int x = fs->nentities-1; x >= 0; --x) {
		if (S_eq(fs->entities[x].name,name)) {
			lentityid id = {x};
	if (x < fn->entities) {
		elf_fscapent(fs,fn,id);
	}
			return id;
		}
	}
	return NO_ENTITY;
}


/*
** Register a local entity within the current function and level,
** if already declared issue a warning or error depending on
** whether is shadows or redeclares and existing entity, however,
** still returns a valid id.
*/
lnodeid elf_fsnewlocalentity(elf_FileState *fs, llineid line, char *name, elf_bool enm) {
	elf_FileFunc *fn = fs->fn;
	lentityid id = elf_fsfndent(fs,line,name);
	if (id.x == NO_ENTITY.x) {
		id = elfY_allocentity(fs,line);

		/* -- todo: for compile time constants,
		no slot allocation required */
		llocalid slot = langL_localalloc(fs,1);
		fs->entities[id.x].slot = slot;
		fs->entities[id.x].enm  = enm;
		fs->entities[id.x].name = name;

		return elf_nodelocal(fs,line,slot);
	} else {
		lentity entity = fs->entities[id.x];
		/* is this variable name already present in this level? */
		if (entity.level == fs->level) {
			elf_lineerror(fs,line,"'%s': already declared",name);
		} else {
			elf_lineerror(fs,line,"'%s': this declaration shadows another one",name);
		}
		return elf_nodelocal(fs,line,entity.slot);
	}
}


lnodeid elf_fsfndentitynode(elf_FileState *fs, llineid line, char *name) {
	lentityid id = elf_fsfndent(fs,line,name);
	if (id.x == NO_ENTITY.x) return NO_NODE;
	elf_FileFunc *fn = fs->fn;
	/* to figure out whether this is capture, simply
	check whether the entity id is higher than that
	of the first entity id within this function, in
	other words, if this entity is outside of this
	function's scope */
	if (id.x < fn->entities) {
		/* -- todo: implement multilayer caching */
		if (id.x < fn->enclosing->entities) {
			elf_lineerror(fs,line,"too many layers for caching");
		}
		return elf_nodecache(fs,line,elfY_indexofentityincache(fn,id));
	} else return elf_nodelocal(fs,line,fs->entities[id.x].slot);
}


void elfY_beginfn(elf_FileState *fs, elf_FileFunc *fn, char *line) {
	elf_fsenterlevel(fs);
	fn->enclosing = fs->fn;
	fn->entities = fs->nentities;
	fn->bytes = fs->md->nbytes;
	fn->line = line;
	fn->yj = lnil;
	fs->fn = fn;
}


void elfY_closefn(elf_FileState *fs) {
	langL_fnepiloge(fs,fs->lasttk.line);
	elfY_leavelevel(fs,fs->lasttk.line);
	/* ensure all locals were deallocated
	properly */
	elf_assert(fs->nentities == fs->fn->entities);
	fs->fn = fs->fn->enclosing;
}


lnodeid *elf_fsloadcallargs(elf_FileState *fs) {
	/* ( x { , x } ) */
	lnodeid *z = 0;
	if (elf_picktk(fs,TK_PAREN_LEFT)) {
		if (!elf_testtk(fs,TK_PAREN_RIGHT)) do {
			lnodeid x = elf_fsloadexpr(fs);
			if (x == -1) break;
			elf_varadd(z,x);
		} while (elf_picktk(fs,TK_COMMA));
		elf_taketk(fs,TK_PAREN_RIGHT);
	}
	return z;
}


/* pretty self explanatory function,
could have used the token itself, but
that's for smart people to do. */
lnodeop tktonode(ltokentype tk) {
	switch (tk) {
		case TK_DOT_DOT:            return NODE_RANGE;
		case TK_LOG_AND:            return NODE_AND;
		case TK_LOG_OR:             return NODE_OR;
		case TK_ADD:                return NODE_ADD;
		case TK_SUB:                return NODE_SUB;
		case TK_DIV:                return NODE_DIV;
		case TK_MUL:                return NODE_MUL;
		case TK_MODULUS:            return NODE_MOD;
		case TK_NOT_EQUALS:         return NODE_NEQ;
		case TK_EQUALS:             return NODE_EQ;
		case TK_GREATER_THAN:       return NODE_GT;
		case TK_GREATER_THAN_EQUAL: return NODE_GTEQ;
		case TK_LESS_THAN:          return NODE_LT;
		case TK_LESS_THAN_EQUAL:    return NODE_LTEQ;
		case TK_LEFT_SHIFT:         return NODE_BITSHL;
		case TK_RIGHT_SHIFT:        return NODE_BITSHR;
		case TK_BIT_XOR:            return NODE_BITXOR;
		default: 						 return NODE_NONE;
	}
}


lnodeid elfY_loadsubexpr(elf_FileState *fs, int rank) {
	lnodeid x = elf_fsloadunary(fs);
	if (x == NO_NODE) return x;
	for (;;) {
		int thisrank = elfX_tokenintel[fs->tk.type].prec;
		/* auto breaks when not a binary operator */
		if (thisrank <= rank) break;
		ltoken tk = elf_yieldtk(fs);
		lnodeid y = elfY_loadsubexpr(fs,thisrank);
		if (y == NO_NODE) break;
		x = elf_nodebinary(fs,tk.line,tktonode(tk.type),NT_ANY,x,y);
	}
	return x;
}


lnodeid elfY_loadfn(elf_FileState *fs) {

	ltoken tk = elf_taketk(fs,TK_FUN);

	/* All bytecode is outputted to the same
	module, the way this language works is
	that we just run the entire module,
	for that reason add a jump byte to
	skip this function's code. */
	int fj = langL_jump(fs,tk.line,-1);

	elf_FileFunc fn = {0};
	elfY_beginfn(fs,&fn,tk.line);

	int arity = 0;

	elf_taketk(fs,TK_PAREN_LEFT);
	if (!elf_testtk(fs,TK_PAREN_RIGHT)) do {

		ltoken n = elf_taketk(fs,TK_WORD);
		elf_fsnewlocalentity(fs,n.line,n.s,lfalse);

		arity ++;
	} while (elf_picktk(fs,TK_COMMA));
	elf_taketk(fs,TK_PAREN_RIGHT);

	elf_taketk(fs,TK_QMARK);

	// ? { .. }
	if (elf_testtk(fs,TK_CURLY_LEFT)) {
		elf_taketk(fs,TK_CURLY_LEFT);
		while (!elfX_term(fs,TK_CURLY_RIGHT)) {
			elfY_loadstat(fs);
		}
		elf_taketk(fs,TK_CURLY_RIGHT);
	} else {
		langL_yield(fs,tk.line,elf_fsloadexpr(fs));
	}

	elfY_closefn(fs);

	/* add this function to the type table */
	elf_Proto p = {0};
	p.x = arity;
	p.y = fn.nyield;
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes  = fs->md->nbytes - fn.bytes;
	p.ncaches = elf_varlen(fn.captures);
	int f = lang_addproto(fs->md,p);

	/* patch jump */
	langL_tieloosej(fs,fj);

	/* todo: */
	lnodeid *z = lnil;
	elf_arrfori(fn.captures) {
		elf_varadd(z
		,	elf_nodelocal(fs,tk.line,fs->entities[fn.captures[i].x].slot));
	}

	return elf_nodecls(fs,tk.line,f,z);
}


void elfY_maybeassign(elf_FileState *fs, lnodeid x) {
	ltoken tk = fs->tk;
	llocalid mem = fs->fn->xmemory;
	if (elf_picktk(fs,TK_ASSIGN)) {
		elfY_checkassign(fs,tk.line,x);
		lnodeid y = elf_fsloadexpr(fs);
		langL_moveto(fs,tk.line,x,y);
	} else
	if (elf_picktk(fs,TK_ASSIGN_QUESTION)) {
		elfY_checkassign(fs,tk.line,x);
		lnodeid y = elf_fsloadexpr(fs);
		ljlist js = {0};
		/* todo: this will evaluate the expression twice, we don't
		want that, isntead we can reuse the previous registers by
		deffering deallocation of those registers until statement
		end, for instance:
		x.y ?= 0
		here x.y will be evaluated twice, once to check whether
		it is nil, and once more to actually assign 0 to it.
		to do this we have to split this function into its components,
		allocate a register for getting x.y and letting jumpifxx
		allocate whatever temporary registers it needs and freeing
		them automatically, the only remaining register will be where
		x.y is at, at which point once the assignment is done it can be
		deallocated. */
		langL_jumpifnotnil(fs,tk.line,&js,x);
		langL_moveto(fs,tk.line,x,y);
		langL_tieloosejs(fs,js.f);
	} else {
		llocalid r = langL_localalloc(fs,1);
		langL_localload(fs,NO_LINE,lfalse,r,0,x);
		fs->fn->xmemory = mem;
	}
	elf_assert(fs->fn->xmemory == mem);
}


lnodeid elf_fsloadtable(elf_FileState *fs) {
	ltoken tk = fs->tk;
	elf_taketk(fs,TK_CURLY_LEFT);
	lnodeid *z = lnil;
	lnodeid table = elf_nodetab(fs,tk.line,lnil);
	int index = 0;
	if (!elf_testtk(fs,TK_CURLY_RIGHT)) do {
		/* -- todo: instead of doing this,
		convert the node to a refernce instead */
		if (elf_testthentk(fs,TK_ASSIGN)) {
			tk = fs->lasttk;
			fs->flags |= NOTANENTITY;
			lnodeid key = elf_fsloadexpr(fs);
			fs->flags &= ~NOTANENTITY;
			elf_taketk(fs,TK_ASSIGN);
			lnodeid val = elf_fsloadexpr(fs);
	if (elf_fscheckexpr(fs,fs->tk.line,val)) {
		break;
	}
			lnodeid fld = elf_nodefield(fs,fs->lasttk.line,table,key);
			lnodeid f = elf_nodeload(fs,fs->lasttk.line,fld,val);
			elf_varadd(z,f);
		} else if (elf_testtk(fs,TK_COMMA) || elf_testtk(fs,TK_CURLY_RIGHT)) {
			/* trap */
		} else {
			lnodeid val = elf_fsloadexpr(fs);
			if (elf_fscheckexpr(fs,fs->tk.line,val)) break;
			lnodeid ii = elf_nodeint(fs,fs->lasttk.line,index ++);
			lnodeid f = elf_nodeload(fs,fs->lasttk.line,elf_nodeindex(fs,fs->lasttk.line,table,ii),val);
			elf_varadd(z,f);
		}
	} while(elf_picktk(fs,TK_COMMA));
	elf_taketk(fs,TK_CURLY_RIGHT);
	fs->nodes[table].z = z;
	return table;
}


lnodeid elf_fsloadexpr(elf_FileState *fs) {
	/* todo?: do this in else where? */
	switch (fs->tk.type) {
		case TK_NONE:
		case TK_PAREN_RIGHT:
		case TK_CURLY_RIGHT:
		case TK_SQUARE_RIGHT: {
			return NO_NODE;
		}
	}
	return elfY_loadsubexpr(fs,0);
}


lnodeid elf_fsloadunary(elf_FileState *fs) {
	lnodeid v = NO_NODE;
	ltoken tk = fs->tk;
	switch (tk.type) {
		/* elf is a reserved keyword used
		for the core namespace */
		case TK_ELF: {
			elf_yieldtk(fs);
			char buf[MAX_PATH] = {"elf"};
			if (elf_testtk(fs,TK_DOT)) {

		while (elf_picktk(fs,TK_DOT)) {
			elf_taketk(fs,TK_WORD);
			strcat(buf,".");
			strcat(buf,fs->lasttk.s);
		}

				lglobalid x = lang_addsymbol(fs->M,elf_newstr(fs->R,buf));
				v = elf_nodeglobal(fs,tk.line,x);

			/* for all intended purposes, this is an error */
			} else elf_lineerror(fs,tk.line,"expected '.' after 'elf', incomplete name");
		} break;
		case TK_WORD: {
			elf_yieldtk(fs);
			if (~fs->flags & NOTANENTITY) {
				v = elf_fsfndentitynode(fs,tk.line,tk.s);
		if (v == NO_NODE) {
			/* todo!: gc'd string, all these objects
			created when parsing should be attached
			to the file, and the file should be an
			object of sorts, when the file is done
			with, we deallocate it, since the file
			is a closure of sorts, this should be
			pretty straight forward... */
			lglobalid x = lang_addsymbol(fs->M,elf_newstr(fs->R,tk.s));
			if (x != NO_NODE) {
				v = elf_nodeglobal(fs,tk.line,x);
			}
		}
		if (v == NO_NODE) {
			elf_lineerror(fs,tk.line,"'%s': undeclared identifier",tk.s);
		}
			} else {
				v = elf_nodestr(fs,tk.line,tk.s);
			}
		} break;
		case TK_SUB: {
			elf_yieldtk(fs);
			v = elfY_loadsubexpr(fs,10000);
			v = elf_nodebinary(fs,tk.line,NODE_SUB,NT_INT,elf_nodeint(fs,tk.line,0),v);
		} break;
		case TK_ADD: {
			elf_yieldtk(fs);
			v = elfY_loadsubexpr(fs,10000);
		} break;
		case TK_CURLY_LEFT: {
			v = elf_fsloadtable(fs);
		} break;
		case TK_PAREN_LEFT: {
			elf_yieldtk(fs);
			v = elf_fsloadexpr(fs);
			elf_taketk(fs,TK_PAREN_RIGHT);
			/* allow for empty () */
			if (v != NO_NODE) {
				v = elf_nodegroup(fs,tk.line,v);
			}
		} break;
		case TK_FUN: {
			v = elfY_loadfn(fs);
		} break;
		case TK_STKGET: case TK_STKLEN: {
			elf_yieldtk(fs);
			lnodeid *z = elf_fsloadcallargs(fs);
			v = elf_nodebuiltincall(fs,tk.line,tk.type,z);
		} break;
		case TK_LOAD: {
			elf_yieldtk(fs);
			v = elf_fsloadexpr(fs);
			v = elf_nodeloadfile(fs,tk.line,v);
		} break;
		case TK_THIS: { elf_yieldtk(fs);
			v = elf_nodenullary(fs,tk.line,NODE_THIS,NT_ANY);
		} break;
		case TK_NIL: { elf_yieldtk(fs);
			v = elf_nodenil(fs,tk.line);
		} break;
		/* todo: maybe use proper boolean node? */
		case TK_TRUE: case TK_FALSE: { elf_yieldtk(fs);
			v = elf_nodeint(fs,tk.line,tk.type == TK_TRUE);
		} break;
		case TK_LETTER: case TK_INTEGER: { elf_yieldtk(fs);
			v = elf_nodeint(fs,tk.line,tk.i);
		} break;
		case TK_NUMBER: { elf_yieldtk(fs);
			v = elf_nodenum(fs,tk.line,tk.n);
		} break;
		case TK_STRING: { elf_yieldtk(fs);
			v = elf_nodestr(fs,tk.line,tk.s);
		} break;
		default: {
			elf_lineerror(fs,tk.line,"'%s': unexpected token", elfX_tokenintel[tk.type].name);
		} break;
	}


	while (!elfX_termeol(fs)) {
		tk = fs->tk;
		switch (tk.type) {
			case TK_DOT: { elf_yieldtk(fs);
				ltoken n = elf_taketk(fs,TK_WORD);
				lnodeid i = elf_nodestr(fs,n.line,n.s);
				v = elf_nodefield(fs,tk.line,v,i);
			} break;
			case TK_SQUARE_LEFT: {
				elf_taketk(fs,TK_SQUARE_LEFT);
				lnodeid i = elf_fsloadexpr(fs);
				elf_taketk(fs,TK_SQUARE_RIGHT);
				if (fs->nodes[i].k == NODE_RANGE_INDEX) {
					LNOBRANCH;
				} else
				if (fs->nodes[i].k == NODE_RANGE) {
					v = elf_noderangedindex(fs,tk.line,v,i);
				} else {
					v = elf_nodeindex(fs,tk.line,v,i);
				}
			} break;
			case TK_COLON: { elf_yieldtk(fs);
				ltoken n = elf_taketk(fs,TK_WORD);
				lnodeid y = elf_nodestr(fs,n.line,n.s);
				v = elf_nodemetafield(fs,tk.line,v,y);
			} break;
			case TK_PAREN_LEFT: {
				lnodeid *z = elf_fsloadcallargs(fs);
				v = elf_nodecall(fs,tk.line,v,z);
			} break;
			default: goto leave;
		}
	}

	leave:
	return v;
}


/* todo: make this legit */
void elfY_loadenumlist(elf_FileState *fs) {
	if (!elf_testtk(fs,TK_CURLY_RIGHT)) {
		do {
			/* , } */
			if (elf_testtk(fs,TK_CURLY_RIGHT)) {
				break;
			}
			ltoken tk = fs->tk;
			ltoken n = elf_taketk(fs,TK_WORD);
			lnodeid x = elf_fsnewlocalentity(fs,n.line,n.s,ltrue);
			elf_taketk(fs,TK_ASSIGN);
			lnodeid y = elf_fsloadexpr(fs);
			langL_moveto(fs,tk.line,x,y);
		} while(elf_picktk(fs,TK_COMMA));
	}
}


void elfY_loadstat(elf_FileState *fs) {
	llocalid mem = fs->fn->xmemory;
	ltoken tk = fs->tk;
	switch (tk.type) {
		case TK_THEN: case TK_ELSE: case TK_ELIF: {
		} break;
		case TK_LEAVE: { elf_yieldtk(fs);
			lnodeid x = elf_fsloadexpr(fs);
			langL_yield(fs,tk.line,x);
			elf_assert(fs->fn->xmemory == mem);
		} break;
		case TK_FINALLY: { elf_taketk(fs,TK_FINALLY);
			FileBlock bl = {0};
			langL_begindelayedblock(fs,tk.line,&bl);
			elfY_loadstat(fs);
			langL_closedelayedblock(fs,tk.line,&bl);
			elf_assert(fs->fn->xmemory == mem);
		} break;
		case TK_WHILE: { elf_yieldtk(fs);
			lnodeid x = elf_fsloadexpr(fs);
			elf_taketk(fs,TK_QMARK);
			Loop loop = {0};
			langL_beginwhile(fs,tk.line,&loop,x);
			elfY_loadstat(fs);
			langL_closewhile(fs,tk.line,&loop);
			elf_assert(fs->fn->xmemory == mem);
		} break;
		case TK_DO: { elf_yieldtk(fs);
			Loop loop = {0};
			langL_begindowhile(fs,tk.line,&loop);
			elfY_loadstat(fs);
			elf_taketk(fs,TK_WHILE);
			lnodeid x = elf_fsloadexpr(fs);
			langL_closedowhile(fs,tk.line,&loop,x);
			elf_assert(fs->fn->xmemory == mem);
		} break;
		case TK_IF: case TK_IFF: { elf_yieldtk(fs);
			lnodeid x = elf_fsloadexpr(fs);
			elf_taketk(fs,TK_QMARK);
			Select s = {0};
			langL_beginif(fs,tk.line,&s,x,tk.type==TK_IFF?L_IFF:L_IF);
			elfY_loadstat(fs);
			while (!elf_testtk(fs,TK_NONE)) {
				if (elf_picktk(fs,TK_ELIF)) {
					x = elf_fsloadexpr(fs);
					elf_taketk(fs,TK_QMARK);
					langL_addelif(fs,fs->lasttk.line,&s,x);
					elfY_loadstat(fs);
				} else
				if (elf_picktk(fs,TK_THEN)) {
					langL_addthen(fs,fs->lasttk.line,&s);
					elfY_loadstat(fs);
				} else
				if (elf_picktk(fs,TK_ELSE)) {
					langL_addelse(fs,fs->lasttk.line,&s);
					elfY_loadstat(fs);
				} else break;
			}
			langL_closeif(fs,fs->lasttk.line,&s);
			elf_assert(fs->fn->xmemory == mem);
		} break;
		case TK_LET: case TK_LOCAL: { elf_yieldtk(fs);
			if (tk.type == TK_LOCAL) {
				elf_lineerror(fs,tk.line,"consider using 'let' instead");
			}
			if (tk.type == TK_ENUM) {
				elf_lineerror(fs,tk.line,"global enums are not supported yet, this enum will be made local");
			} else
			/* allows for 'let enum' */
			if (tk.type == TK_LET) {
				if (elf_testtk(fs,TK_ENUM)) {
					tk = elf_yieldtk(fs);
				}
			}
			elf_bool enm = tk.type == TK_ENUM;
			if (elf_picktk(fs,TK_CURLY_LEFT)) {
				elfY_loadenumlist(fs);
				elf_taketk(fs,TK_CURLY_RIGHT);
			} else {
				do {
					ltoken n = elf_taketk(fs,TK_WORD);
					lnodeid x = elf_fsnewlocalentity(fs,n.line,n.s,enm);
					elfY_maybeassign(fs,x);
				} while (elf_picktk(fs,TK_COMMA));
			}
		} break;
		case TK_FOREACH: case TK_FOR: {
			elf_yieldtk(fs);
			elf_fsenterlevel(fs);
			ltoken n = elf_taketk(fs,TK_WORD);
			lnodeid i, x, y, lo, hi;
			x = elf_fsnewlocalentity(fs,n.line,n.s,lfalse);
			elf_taketk(fs,TK_IN);
			y = elf_fsloadexpr(fs);
			elf_taketk(fs,TK_QMARK);
			i = x;
			lo = NO_NODE;
			hi = NO_NODE;
			if (tk.type == TK_FOR) {
				if (fs->nodes[y].k == NODE_RANGE) {
					lo = fs->nodes[y].x;
					hi = fs->nodes[y].y;
				} else {
					lo = elf_nodeint(fs,tk.line,0);
					hi = y;
				}
			} else {
				LNOBRANCH;/* todo: emit code to initialize x to i'th item of y */
				if (fs->nodes[y].k == NODE_RANGE) {
					lo = fs->nodes[y].x;
					hi = fs->nodes[y].y;
				} else {
					lo = elf_nodeint(fs,tk.line,0);
					/* todo: add typeguard */
					hi = elf_nodemetafield(fs,tk.line,y,elf_nodestr(fs,tk.line,"length"));
					i = elf_nodelocal(fs,tk.line,langL_localalloc(fs,1));
				}
			}
			Loop loop = {0};
			langL_beginrangedloop(fs,tk.line,&loop,i,lo,hi);
			elfY_loadstat(fs);
			langL_closerangedloop(fs,tk.line,&loop);
			elfY_leavelevel(fs,tk.line);
			fs->fn->xmemory = mem;
		} break;
		case TK_CURLY_LEFT: { elf_yieldtk(fs);
			elf_fsenterlevel(fs);
			while (!elfX_term(fs,TK_CURLY_RIGHT)) {
				elfY_loadstat(fs);
			}
			elf_taketk(fs,TK_CURLY_RIGHT);
			elfY_leavelevel(fs,fs->lasttk.line);
			fs->fn->xmemory = mem;
		} break;
		default: {
			lnodeid x = elf_fsloadexpr(fs);
			elfY_maybeassign(fs,x);
			elf_assert(fs->fn->xmemory == mem);
		} break;
	}
}



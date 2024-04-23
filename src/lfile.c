/*
** See Copyright Notice In lang.h
** (Y) lfile.c
** Parsing Stuff
*/


lbool langX_checkexpr(FileState *fs, llineid line, lnodeid id) {
	if (id != NO_NODE) return lfalse;
	langX_error(fs,line,"invalid expression");
	return ltrue;
}


lbool langX_test(FileState *fs, ltokentype k) {
	return fs->tk.type == k;
}


lbool langX_testthen(FileState *fs, ltokentype k) {
	return fs->thentk.type == k;
}


/*
** Returns true whether there are no more tokens
** or whehter the current token is a match.
*/
lbool langX_term(FileState *fs, ltokentype k) {
	return fs->tk.type == TK_NONE || fs->tk.type == k;
}


lbool langX_termeol(FileState *fs) {
	return fs->tk.type == TK_NONE || fs->lasttk.eol == ltrue;
}


/*
** Consumes the current token only if it is a match,
** returning whether it was a match or not.
*/
lbool langX_pick(FileState *fs, ltokentype k) {
	return langX_test(fs,k) && (langX_yield(fs), ltrue);
}


/*
** Same as pick, only this time there are two possibilities.
*/
lbool langX_choose(FileState *fs, ltokentype x, ltokentype y) {
	return (langX_test(fs,x) || langX_test(fs,y)) && (langX_yield(fs), ltrue);
}


/*
** Check whether the current token is a match,
** if so pick it, otherwise error.
*/
ltoken langX_take(FileState *fs, int k) {
	ltoken tk = fs->tk;
	if (!langX_pick(fs,k)) {
		langX_error(fs,fs->tk.line,"expected '%s'\n",langX_tokenintel[k].name);
	}
	return tk;
}


lentityid langY_allocentity(FileState *fs, llineid line) {
	FileFunc *ff = fs->fn;
	lentityid id = {fs->nentities ++};
	if (langA_varlen(fs->entities) < fs->nentities) {
		langA_variadd(fs->entities,1);
	}

	for (llocalid i = id.x; i < fs->nentities; ++i) {
		fs->entities[i].level = fs->level;
		fs->entities[i].line  = line;
		fs->entities[i].name  = 0;
		fs->entities[i].slot  = 0;
		fs->entities[i].enm   = lfalse;
	}
	// langX_error(fs,line,"%i, fs=%i, ff=%i",id,fs->nentities,nentities);
	return id;
}


llocalid langY_numentinlevl(FileState *fs) {
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


lnodeid langY_numnodesinlev(FileState *fs) {
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


void langY_leavelevel(FileState *fs, llineid line) {
	fs->nentities -= langY_numentinlevl(fs); /* close scope */
	fs->nnodes -= langY_numnodesinlev(fs);
	fs->fn->xmemory -= 0; /* todo: deallocate leftover locals */
	/* ensure that we don't deallocate more than we can */
	LASSERT(fs->nentities >= fs->fn->entities);
	-- fs->level;
}


void langY_enterlevel(FileState *fs) {
	++ fs->level;
}


void langY_checkassign(FileState *fs, llineid line, lnodeid x) {
	/* todo: welp, we can't do this anymore */
	// lNode n = fs->nodes[x];
	// if (n.k == NODE_LOCAL) {
	// 	lentity fnn = fs->entities[fs->fn->entities+n.x];
	// 	if (fnn.enm) {
	// 		langX_error(fs,line,"enum value is constant, you're attempting to modify its value");
	// 	}
	// }
}


/*
** Looks for a captured entity within the function's captures
** and returns the index where the captured entity resides.
** the index is then used to emit instructions targeting
** captures, the first capture corresponds to index 0.
*/
int langY_indexofentityincache(FileFunc *fn, lentityid id) {
	langA_varifor(fn->captures) {
		if (fn->captures[i].x == id.x) return i;
	}
	return -1;
}


/*
** Captures an entitity (if not already) from the enclosing
** function, storing a copy of the entity id in the function's
** captures.
*/
void langY_captureentity(FileState *fs, FileFunc *fn, lentityid id) {
	/* ensure the entity should actually be captured */
	LASSERT(id.x < fn->entities);
	langA_varifor(fn->captures) {
		if (fn->captures[i].x == id.x) return;
	}
	langA_varadd(fn->captures,id);
}


/*
** Find the closest entity in order of lexical relevance,
** starting from the last declared entity.
** The id returned is an absolute index into fs->entities,
** you should make it relative to the current function.
** If the entity is outside of this function, then it caches it.
*/
lentityid langY_fndentity(FileState *fs, llineid line, char *name) {
	FileFunc *fn = fs->fn;
	for (int x = fs->nentities-1; x >= 0; --x) {
		if (S_eq(fs->entities[x].name,name)) {
			lentityid id = {x};
			if (x < fn->entities) langY_captureentity(fs,fn,id);
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
lnodeid langY_newlocalentity(FileState *fs, llineid line, char *name, lbool enm) {
	FileFunc *fn = fs->fn;
	lentityid id = langY_fndentity(fs,line,name);
	if (id.x == NO_ENTITY.x) {
		id = langY_allocentity(fs,line);

		/* -- todo: for compile time constants,
		no slot allocation required */
		llocalid slot = langL_localalloc(fs,1);
		fs->entities[id.x].slot = slot;
		fs->entities[id.x].enm  = enm;
		fs->entities[id.x].name = name;

		return langN_local(fs,line,slot);
	} else {
		lentity entity = fs->entities[id.x];
		/* is this variable name already present in this level? */
		if (entity.level == fs->level) {
			langX_error(fs,line,"'%s': already declared",name);
		} else {
			langX_error(fs,line,"'%s': this declaration shadows another one",name);
		}
		return langN_local(fs,line,entity.slot);
	}
}


lnodeid langY_fndentitynode(FileState *fs, llineid line, char *name) {
	lentityid id = langY_fndentity(fs,line,name);
	if (id.x == NO_ENTITY.x) return NO_NODE;
	FileFunc *fn = fs->fn;
	/* to figure out whether this is capture, simply
	check whether the entity id is higher than that
	of the first entity id within this function, in
	other words, if this entity is outside of this
	function's scope */
	if (id.x < fn->entities) {
		/* -- todo: implement multilayer caching */
		if (id.x < fn->enclosing->entities) {
			langX_error(fs,line,"too many layers for caching");
		}
		return langN_cache(fs,line,langY_indexofentityincache(fn,id));
	} else return langN_local(fs,line,fs->entities[id.x].slot);
}


void langY_beginfn(FileState *fs, FileFunc *fn, char *line) {
	langY_enterlevel(fs);
	fn->enclosing = fs->fn;
	fn->entities = fs->nentities;
	fn->bytes = fs->md->nbytes;
	fn->line = line;
	fn->yj = lnil;
	fs->fn = fn;
}


void langY_closefn(FileState *fs) {
	langL_fnepiloge(fs,fs->lasttk.line);
	langY_leavelevel(fs,fs->lasttk.line);
	/* ensure all locals were deallocated
	properly */
	LASSERT(fs->nentities == fs->fn->entities);
	fs->fn = fs->fn->enclosing;
}


lnodeid *langY_loadcallargs(FileState *fs) {
	/* ( x { , x } ) */
	lnodeid *z = 0;
	if (langX_pick(fs,TK_PAREN_LEFT)) {
		if (!langX_test(fs,TK_PAREN_RIGHT)) do {
			lnodeid x = langY_loadexpr(fs);
			if (x == -1) break;
			langA_varadd(z,x);
		} while (langX_pick(fs,TK_COMMA));
		langX_take(fs,TK_PAREN_RIGHT);
	}
	return z;
}


/* -- Pretty self explanatory function,
- could have used the token itself, but
- that's for smart people to do.
- */
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
	}
	return Y_NONE;
}


lnodeid langY_loadsubexpr(FileState *fs, int rank) {
	lnodeid x = langY_loadunary(fs);
	if (x == NO_NODE) return x;
	for (;;) {
		int thisrank = langX_tokenintel[fs->tk.type].prec;
		/* auto breaks when not a binary operator */
		if (thisrank <= rank) break;
		ltoken tk = langX_yield(fs);
		lnodeid y = langY_loadsubexpr(fs,thisrank);
		if (y == NO_NODE) break;
		x = langN_xy(fs,tk.line,tktonode(tk.type),NT_ANY,x,y);
	}
	return x;
}


lnodeid langY_loadfn(FileState *fs) {

	ltoken tk = langX_take(fs,TK_FUN);

	/* All bytecode is outputted to the same
	module, the way this language works is
	that we just run the entire module,
	for that reason add a jump byte to
	skip this function's code. */
	int fj = langL_jump(fs,tk.line,-1);

	FileFunc fn = {0};
	langY_beginfn(fs,&fn,tk.line);

	int arity = 0;

	langX_take(fs,TK_PAREN_LEFT);
	if (!langX_test(fs,TK_PAREN_RIGHT)) do {

		ltoken n = langX_take(fs,TK_WORD);
		langY_newlocalentity(fs,n.line,n.s,lfalse);

		arity ++;
	} while (langX_pick(fs,TK_COMMA));
	langX_take(fs,TK_PAREN_RIGHT);

	langX_take(fs,TK_QMARK);

	// ? { .. }
	if (langX_test(fs,TK_CURLY_LEFT)) {
		langX_take(fs,TK_CURLY_LEFT);
		while (!langX_term(fs,TK_CURLY_RIGHT)) {
			langY_loadstat(fs);
		}
		langX_take(fs,TK_CURLY_RIGHT);
	} else {
		langL_yield(fs,tk.line,langY_loadexpr(fs));
	}

	langY_closefn(fs);

	/* add this function to the type table */
	lProto p = {0};
	p.x = arity;
	p.y = fn.nyield;
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes  = fs->md->nbytes - fn.bytes;
	p.ncaches = langA_varlen(fn.captures);
	int f = lang_addproto(fs->md,p);

	/* patch jump */
	langL_tieloosej(fs,fj);

	/* todo: */
	lnodeid *z = lnil;
	langA_varifor(fn.captures) {
		langA_varadd(z
		,	langN_local(fs,tk.line,fs->entities[fn.captures[i].x].slot));
	}

	return langN_closure(fs,tk.line,f,z);
}


void langY_maybeassign(FileState *fs, lnodeid x) {
	ltoken tk = fs->tk;
	llocalid mem = fs->fn->xmemory;
	if (langX_pick(fs,TK_ASSIGN)) {
		langY_checkassign(fs,tk.line,x);
		lnodeid y = langY_loadexpr(fs);
		langL_moveto(fs,tk.line,x,y);
	} else
	if (langX_pick(fs,TK_ASSIGN_QUESTION)) {
		langY_checkassign(fs,tk.line,x);
		lnodeid y = langY_loadexpr(fs);
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
	LASSERT(fs->fn->xmemory == mem);
}


lnodeid langY_loadtable(FileState *fs) {
	ltoken tk = fs->tk;
	langX_take(fs,TK_CURLY_LEFT);
	lnodeid *z = lnil;
	lnodeid table = langN_table(fs,tk.line,lnil);
	int index = 0;
	if (!langX_test(fs,TK_CURLY_RIGHT)) do {
		if (langX_testthen(fs,TK_ASSIGN)) {
			/* -- todo: we could support any arbitrary expression here,
			to do this, flag the loadexpr so that it doesn't bind entities
			at first, otherwise the result of that expression should be
			the key */
			if (langX_choose(fs,TK_WORD,TK_LETTER)
			||  langX_choose(fs,TK_INTEGER,TK_STRING)) {
				tk = fs->lasttk;
				lnodeid key;
				if (tk.type == TK_INTEGER || tk.type == TK_LETTER) key = langN_longint(fs,tk.line,tk.i);
				else key = langN_S(fs,tk.line,tk.s);
				langX_take(fs,TK_ASSIGN);
				lnodeid val = langY_loadexpr(fs);
				if (langX_checkexpr(fs,fs->tk.line,val)) break;
				lnodeid f = langN_load(fs,fs->lasttk.line,langN_field(fs,fs->lasttk.line,table,key),val);
				langA_varadd(z,f);
			} else langX_error(fs,fs->tk.line,"expected either word, string, character or integer for key-value designator");
		} else if (langX_test(fs,TK_COMMA) || langX_test(fs,TK_CURLY_RIGHT)) {
			/* trap */
		} else {
			lnodeid val = langY_loadexpr(fs);
			if (langX_checkexpr(fs,fs->tk.line,val)) break;
			lnodeid ii = langN_longint(fs,fs->lasttk.line,index ++);
			lnodeid f = langN_load(fs,fs->lasttk.line,langN_index(fs,fs->lasttk.line,table,ii),val);
			langA_varadd(z,f);
		}
	} while(langX_pick(fs,TK_COMMA));
	langX_take(fs,TK_CURLY_RIGHT);
	fs->nodes[table].z = z;
	return table;
}


lnodeid langY_loadexpr(FileState *fs) {
	/* todo?: do this in else where? */
	switch (fs->tk.type) {
		case TK_NONE:
		case TK_PAREN_RIGHT:
		case TK_CURLY_RIGHT:
		case TK_SQUARE_RIGHT: {
			return NO_NODE;
		}
	}
	return langY_loadsubexpr(fs,0);
}


lnodeid langY_loadunary(FileState *fs) {
	lnodeid v = - 1;
	ltoken tk = fs->tk;
	switch (tk.type) {
		case TK_SUB: {
			langX_yield(fs);
			v = langY_loadsubexpr(fs,10000);
			v = langN_xy(fs,tk.line,NODE_SUB,NT_INT,langN_longint(fs,tk.line,0),v);
		} break;
		case TK_ADD: {
			langX_yield(fs);
			v = langY_loadsubexpr(fs,10000);
		} break;
		case TK_CURLY_LEFT: {
			v = langY_loadtable(fs);
		} break;
		case TK_PAREN_LEFT: {
			langX_yield(fs);
			v = langY_loadexpr(fs);
			langX_take(fs,TK_PAREN_RIGHT);
			/* allow for empty () */
			if (v != NO_NODE) {
				v = langN_group(fs,tk.line,v);
			}
		} break;
		case TK_FUN: {
			v = langY_loadfn(fs);
		} break;
		case TK_STKGET: case TK_STKLEN: {
			langX_yield(fs);
			lnodeid *z = langY_loadcallargs(fs);
			v = langN_builtincall(fs,tk.line,tk.type,z);
		} break;
		case TK_LOAD: {
			langX_yield(fs);
			v = langY_loadexpr(fs);
			v = langN_loadfile(fs,tk.line,v);
		} break;
		case TK_THIS: { langX_yield(fs);
			v = langN_nullary(fs,tk.line,NODE_THIS,NT_ANY);
		} break;
		case TK_WORD: { langX_yield(fs);
			v = langY_fndentitynode(fs,tk.line,tk.s);
			if (v == NO_NODE) {
				/* todo:! gc'd string */
				lglobalid x = lang_addsymbol(fs->md,langS_new(fs->rt,tk.s));
				if (x != -1) v = langN_global(fs,tk.line,x);
			}
			if (v == NO_NODE) {
				langX_error(fs,tk.line,"'%s': undeclared identifier",tk.s);
			}
		} break;
		case TK_NIL: { langX_yield(fs);
			v = langN_nil(fs,tk.line);
		} break;
		/* todo: maybe use proper boolean node? */
		case TK_TRUE: case TK_FALSE: { langX_yield(fs);
			v = langN_longint(fs,tk.line,tk.type == TK_TRUE);
		} break;
		case TK_LETTER: case TK_INTEGER: { langX_yield(fs);
			v = langN_longint(fs,tk.line,tk.i);
		} break;
		case TK_NUMBER: { langX_yield(fs);
			v = langN_number(fs,tk.line,tk.n);
		} break;
		case TK_STRING: { langX_yield(fs);
			v = langN_S(fs,tk.line,tk.s);
		} break;
		default: {
			langX_error(fs,tk.line,"'%s': unexpected token", langX_tokenintel[tk.type].name);
		} break;
	}


	while (!langX_termeol(fs)) {
		tk = fs->tk;
		switch (tk.type) {
			case TK_DOT: { langX_yield(fs);
				ltoken n = langX_take(fs,TK_WORD);
				lnodeid i = langN_S(fs,n.line,n.s);
				v = langN_field(fs,tk.line,v,i);
			} break;
			case TK_SQUARE_LEFT: {
				langX_take(fs,TK_SQUARE_LEFT);
				lnodeid i = langY_loadexpr(fs);
				langX_take(fs,TK_SQUARE_RIGHT);
				if (fs->nodes[i].k == NODE_RANGE_INDEX) {
					LNOBRANCH;
				} else
				if (fs->nodes[i].k == NODE_RANGE) {
					v = langN_rangedindex(fs,tk.line,v,i);
				} else {
					v = langN_index(fs,tk.line,v,i);
				}
			} break;
			case TK_COLON: {
				langX_take(fs,TK_COLON);
				ltoken n = langX_take(fs,TK_WORD);
				lnodeid y = langN_S(fs,n.line,n.s);
				v = langN_metafield(fs,tk.line,v,y);
			} break;
			case TK_PAREN_LEFT: {
				lnodeid *z = langY_loadcallargs(fs);
				v = langN_call(fs,tk.line,v,z);
			} break;
			default: goto leave;
		}
	}

	leave:
	return v;
}


/* todo: make this legit */
void langY_loadenumlist(FileState *fs) {
	if (!langX_test(fs,TK_CURLY_RIGHT)) {
		do {
			/* , } */
			if (langX_test(fs,TK_CURLY_RIGHT)) {
				break;
			}
			ltoken tk = fs->tk;
			ltoken n = langX_take(fs,TK_WORD);
			lnodeid x = langY_newlocalentity(fs,n.line,n.s,ltrue);
			langX_take(fs,TK_ASSIGN);
			lnodeid y = langY_loadexpr(fs);
			langL_moveto(fs,tk.line,x,y);
		} while(langX_pick(fs,TK_COMMA));
	}
}


void langY_loadstat(FileState *fs) {
	llocalid mem = fs->fn->xmemory;
	ltoken tk = fs->tk;
	switch (tk.type) {
		case TK_THEN: case TK_ELSE: case TK_ELIF: {
		} break;
		case TK_LEAVE: { langX_yield(fs);
			lnodeid x = langY_loadexpr(fs);
			langL_yield(fs,tk.line,x);
			LASSERT(fs->fn->xmemory == mem);
		} break;
		case TK_FINALLY: { langX_take(fs,TK_FINALLY);
			FileBlock bl = {0};
			langL_begindelayedblock(fs,tk.line,&bl);
			langY_loadstat(fs);
			langL_closedelayedblock(fs,tk.line,&bl);
			LASSERT(fs->fn->xmemory == mem);
		} break;
		case TK_WHILE: { langX_yield(fs);
			lnodeid x = langY_loadexpr(fs);
			langX_take(fs,TK_QMARK);
			Loop loop = {0};
			langL_beginwhile(fs,tk.line,&loop,x);
			langY_loadstat(fs);
			langL_closewhile(fs,tk.line,&loop);
			LASSERT(fs->fn->xmemory == mem);
		} break;
		case TK_DO: { langX_yield(fs);
			Loop loop = {0};
			langL_begindowhile(fs,tk.line,&loop);
			langY_loadstat(fs);
			langX_take(fs,TK_WHILE);
			lnodeid x = langY_loadexpr(fs);
			langL_closedowhile(fs,tk.line,&loop,x);
			LASSERT(fs->fn->xmemory == mem);
		} break;
		case TK_IF: case TK_IFF: { langX_yield(fs);
			lnodeid x = langY_loadexpr(fs);
			langX_take(fs,TK_QMARK);
			Select s = {0};
			langL_beginif(fs,tk.line,&s,x,tk.type==TK_IFF?L_IFF:L_IF);
			langY_loadstat(fs);
			while (!langX_test(fs,TK_NONE)) {
				if (langX_pick(fs,TK_ELIF)) {
					x = langY_loadexpr(fs);
					langX_take(fs,TK_QMARK);
					langL_addelif(fs,fs->lasttk.line,&s,x);
					langY_loadstat(fs);
				} else
				if (langX_pick(fs,TK_THEN)) {
					langL_addthen(fs,fs->lasttk.line,&s);
					langY_loadstat(fs);
				} else
				if (langX_pick(fs,TK_ELSE)) {
					langL_addelse(fs,fs->lasttk.line,&s);
					langY_loadstat(fs);
				} else break;
			}
			langL_closeif(fs,fs->lasttk.line,&s);
			LASSERT(fs->fn->xmemory == mem);
		} break;
		case TK_LET: case TK_LOCAL: { langX_yield(fs);
			if (tk.type == TK_LOCAL) {
				langX_error(fs,tk.line,"consider using 'let' instead");
			}
			if (tk.type == TK_ENUM) {
				langX_error(fs,tk.line,"global enums are not supported yet, this enum will be made local");
			} else
			/* allows for 'let enum' */
			if (tk.type == TK_LET) {
				if (langX_test(fs,TK_ENUM)) {
					tk = langX_yield(fs);
				}
			}
			lbool enm = tk.type == TK_ENUM;
			if (langX_pick(fs,TK_CURLY_LEFT)) {
				langY_loadenumlist(fs);
				langX_take(fs,TK_CURLY_RIGHT);
			} else {
				do {
					ltoken n = langX_take(fs,TK_WORD);
					lnodeid x = langY_newlocalentity(fs,n.line,n.s,enm);
					langY_maybeassign(fs,x);
				} while (langX_pick(fs,TK_COMMA));
			}
		} break;
		case TK_FOREACH: case TK_FOR: { langX_yield(fs);
			langY_enterlevel(fs);
			ltoken n = langX_take(fs,TK_WORD);
			lnodeid i, x, y;
			lnodeid lo, hi;
			x = langY_newlocalentity(fs,n.line,n.s,lfalse);
			langX_take(fs,TK_IN);
			y = langY_loadexpr(fs);
			langX_take(fs,TK_QMARK);
			i = x;
			lo = NO_NODE;
			hi = NO_NODE;
			if (tk.type == TK_FOR) {
				if (fs->nodes[y].k == NODE_RANGE) {
					lo = fs->nodes[y].x;
					hi = fs->nodes[y].y;
				} else {
					lo = langN_longint(fs,tk.line,0);
					hi = y;
				}
			} else {
				LNOBRANCH;/* todo: emit code to initialize x to i'th item of y */
				if (fs->nodes[y].k == NODE_RANGE) {
					lo = fs->nodes[y].x;
					hi = fs->nodes[y].y;
				} else {
					lo = langN_longint(fs,tk.line,0);
					/* todo: add typeguard */
					hi = langN_metafield(fs,tk.line,y,langN_S(fs,tk.line,"length"));
					i = langN_local(fs,tk.line,langL_localalloc(fs,1));
				}
			}
			Loop loop = {0};
			langL_beginrangedloop(fs,tk.line,&loop,i,lo,hi);
			langY_loadstat(fs);
			langL_closerangedloop(fs,tk.line,&loop);
			langY_leavelevel(fs,tk.line);
			fs->fn->xmemory = mem;
		} break;
		case TK_CURLY_LEFT: { langX_yield(fs);
			langY_enterlevel(fs);
			while (!langX_term(fs,TK_CURLY_RIGHT)) {
				langY_loadstat(fs);
			}
			langX_take(fs,TK_CURLY_RIGHT);
			langY_leavelevel(fs,fs->lasttk.line);
			fs->fn->xmemory = mem;
		} break;
		default: {
			lnodeid x = langY_loadexpr(fs);
			langY_maybeassign(fs,x);
			LASSERT(fs->fn->xmemory == mem);
		} break;
	}
}



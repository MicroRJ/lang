/*
** See Copyright Notice In lang.h
** lfile.h
** Parsing Stuff
*/


Bool langY_testtk(FileState *fs, TokenName k) {
	return fs->tk.type == k;
}


Bool langY_picktk(FileState *fs, TokenName k) {
	if (!langY_testtk(fs,k)) return False;

	langX_yield(fs);
	return True;
}


/*
** Check whether the current token is of type k,
** if so pick it, otherwise error
*/
Token langY_taketk(FileState *fs, int k) {
	Token tk = fs->tk;
	if (!langY_picktk(fs,k)) {
		langX_error(fs,fs->tk.loc,"expected '%s'\n",langX_tokenname(k));
	}
	return tk;
}


void langY_enterlevel(FileState *fs) {
	++ fs->level;
}


void langY_leavelevel(FileState *fs) {
	-- fs->level;
	/* todo: recycle nodes here */
}


/*
** Looks for an cache value pointing to x,
** if found returns the index within the
** array where it resides.
*/
int langY_cacheindex(FuncState *fn, LocalId x) {
	langA_varifor(fn->caches) {
		if (fn->caches[i] == x) return i;
	}
	return -1;
}


/*
** Caches a local from the enclosing function,
** storing a copy of the value within cache
** storage in the closure.
*/
void langY_cachelocal(FileState *fs, FuncState *fn, char *loc, LocalId local) {
	/* ensure the local should actually be captured,
	either of these checks should suffice */
	LASSERT(local < fn->locals);
	LASSERT(fs->locals[local].level < fn->level);

	/* is this local cached already? */
	langA_varifor(fn->caches) {
		if (fn->caches[i] == local) return;
	}
	langA_varadd(fn->caches,local);
	// langX_error(fs,loc,"'%s': cached %i in:",fs->locals[local].name,local);
	// langX_error(fs,fn->loc,0);
}


/*
** Find the closest local variable in order of
** lexical relevance, starting from the last
** declared local.
** The id returned is an absolute index into
** fs->locals and you should make it relative
** to the current function.
** If the local is outside of this function,
** then it caches it.
*/
LocalId langY_localorcacheid(FileState *fs, char *loc, char *name) {
	FuncState *fn = fs->fn;
	int locals = fn->locals;
	for (int i = fs->nlocals-1; i >= 0; --i) {
		if (S_eq(fs->locals[i].name,name)) {
			if (i < locals) langY_cachelocal(fs,fn,loc,i);
			return i;
		}
	}
	return -1;
}


/*
** Register a local variable within
** the current function.
*/
LocalId langY_localname(FileState *fs, char *loc, char *name) {
	LocalId id = langY_localorcacheid(fs,loc,name);
	if (id == -1) {
		if (langA_varmin(fs->locals) <= fs->nlocals) {
			langA_variadd(fs->locals,1);
		}

		id = fs->nlocals ++;

		FuncState *fn = fs->fn;
		NodeId n = langY_localnode(fs,loc,id-fn->locals);

		fs->locals[id].name  = name;
		fs->locals[id].level = fs->level;
		fs->locals[id].loc   = loc;
		fs->locals[id].n     = n;

		return id;
	}

	FuncLocal local = fs->locals[id];
		/* is this variable name already present in
		this level? */
	if (local.level == fs->level) {
		langX_error(fs,loc,"'%s': already declared",name);
	} else {
		langX_error(fs,loc,"'%s': this declaration shadows another one",name);
	}
	return -1;
}


NodeId langY_cacheorlocalnode(FileState *fs, char *loc, char *name) {
	LocalId id = langY_localorcacheid(fs,loc,name);
	if (id == -1) return -1;

	LocalId x = -1;

	FuncState *fn = fs->fn;
	/* is this a capture? */
	if (id < fn->locals) {
		/* This is technically impossible,
		function at level 0 cannot capture
		anything above. If the level is less
		than 0, then we've royally screwed up. */
		if (fn->level == 0) LNOCHANCE;

		if (id < fn->enclosing->locals) {
			langX_error(fs,loc,"too many layers for caching");
		} else {
			x = langY_cachenode(fs,loc,langY_cacheindex(fn,id));
		}
	} else {
		x = fs->locals[id].n;
		/* make the id relative */
		// x = langY_localnode(fs,loc,id-fn->locals);
	}
	return x;
}


NodeId *langY_parsecallargs(FileState *fs) {
	/* ( x { , x } ) */
	NodeId *z = 0;
	if (langY_picktk(fs,TK_PAREN_LEFT)) {
		if (!langY_testtk(fs,TK_PAREN_RIGHT)) do {
			NodeId x = langY_loadexpr(fs);
			if (x == -1) break;
			langA_varadd(z,x);
		} while (langY_picktk(fs,TK_COMMA));
		langY_taketk(fs,TK_PAREN_RIGHT);
	}
	return z;
}


int langY_parsesuffix(FileState *fs, int v) {
	while (fs->tk.type != TK_NONE) {
		const Token tk = fs->tk;
		switch (tk.type) {
			// x . { y }
			case TK_DOT: {
				langX_yield(fs);
				Token n = langY_taketk(fs,TK_WORD);
				int i = langY_nodeS(fs,n.loc,n.s);
				v = langY_fieldnode(fs,tk.loc,v,i);
			} break;
			// {x} . [ {x} ]
			case TK_SQUARE_LEFT: {
				langY_taketk(fs,TK_SQUARE_LEFT);
				int i = langY_loadexpr(fs);
				langY_taketk(fs,TK_SQUARE_RIGHT);
				if (fs->nodes[i].k == NODE_RANGE) {
					v = langY_rangeindexnode(fs,tk.loc,v,i);
				} else {
					v = langY_fieldnode(fs,tk.loc,v,i);
				}
			} break;
			/* {x} : {x} () */
			case TK_COLON: {
				langY_taketk(fs,TK_COLON);
				Token n = langY_taketk(fs,TK_WORD);
				NodeId y = langY_nodeS(fs,n.loc,n.s);
				NodeId *z = langY_parsecallargs(fs);
				v = langY_metacallnode(fs,tk.loc,v,y,z);
			} break;
			case TK_PAREN_LEFT: {
				NodeId *z = langY_parsecallargs(fs);
				v = langY_callnode(fs,tk.loc,v,z);
			} break;
			default: goto leave;
		}
	}

	leave:
	return v;
}


NodeId langY_parseunary(FileState *fs) {
	NodeId v = - 1;
	Token tk = fs->tk;
	switch (tk.type) {
		case TK_LOAD: {
			langX_yield(fs);
			v = langY_loadexpr(fs);
			v = langY_loadfilenode(fs,tk.loc,v);
		} break;
		case TK_WORD: {
			langX_yield(fs);

			v = langY_cacheorlocalnode(fs,tk.loc,(char*)tk.s);

			if (v == -1) {
				/* todo:! gc'd string */
				int x = lang_addsymbol(fs->md,langS_new(fs->rt,tk.s));
				if (x != -1) {
					v = langY_globalnode(fs,tk.loc,x);
				}
			}

			if (v == -1) {
				langX_error(fs,tk.loc,"'%s': undeclared identifier",tk.s);
			}
		} break;
		case TK_TRUE: {
			/* todo: maybe use proper boolean node? */
			langX_yield(fs);
			v = langY_nodeI(fs,tk.loc,1);
		} break;
		case TK_FALSE: {
			/* todo: maybe use proper boolean node? */
			langX_yield(fs);
			v = langY_nodeI(fs,tk.loc,0);
		} break;
		case TK_INTEGER: {
			langX_yield(fs);
			v = langY_nodeI(fs,tk.loc,tk.i);
		} break;
		case TK_STRING: {
			langX_yield(fs);
			v = langY_nodeS(fs,tk.loc,tk.s);
		} break;
		default: {
			langX_error(fs,tk.loc,"'%s': unexpected token", langX_tokenname(tk.type));
		} break;
	}

	return langY_parsesuffix(fs,v);
}


NodeId langY_parsesubexpr(FileState *fs, int rank) {
	int x = langY_parseunary(fs);
	if (x == -1) return x;

	for (;;) {
		int thisrank = langX_tokenprec(fs->tk.type);
		/* auto breaks when is not a binary operator */
		if (thisrank <= rank) {
			break;
		}
		Token tk = langX_yield(fs);
		int y = langY_parsesubexpr(fs,thisrank);
		if (y == -1) {
			break;
		}
		x = langY_node2(fs,tk.loc,tk.type,x,y);
	}
	return x;
}


void langY_beginfunc(FileState *fs, FuncState *fn, char *loc) {
	/* Function should be at the same level as
	its locals */
	langY_enterlevel(fs);
	fn->enclosing = fs->fn;
	fn->locals = fs->nlocals;
	fn->level = fs->level;
	fn->bytes = fs->md->nbytes;
	fn->loc = loc;
	fs->fn = fn;
}


void langY_closefunc(FileState *fs) {
	fs->nlocals = fs->fn->locals;
	fs->fn = fs->fn->enclosing;
	langY_leavelevel(fs);
}


NodeId langY_loadfunc(FileState *fs) {

	Token tk = langY_taketk(fs,TK_FUN);

	/* All bytecode is outputted to the same
	module, and the way this language work is
	that we just run the entire thing,
	for that reason add a jump byte to
	skip this function's code. */
	int fj = langC_jump(fs,tk.loc,-1);

	FuncState fn = {0};
	langY_beginfunc(fs,&fn,tk.loc);


	int arity = 0;

	langY_taketk(fs,TK_PAREN_LEFT);
	if (!langY_testtk(fs,TK_PAREN_RIGHT)) do {

		Token n = langY_taketk(fs,TK_WORD);
		langY_localname(fs,n.loc,(char*)n.s);

		arity ++;
	} while (langY_picktk(fs,TK_COMMA));
	langY_taketk(fs,TK_PAREN_RIGHT);

	langY_taketk(fs,TK_QMARK);

	langY_parsestat(fs);

	/* add this function to the type table */
	Proto p = {0};
	p.bytes = fn.bytes;
	p.arity = arity;
	p.nbytes  = fs->md->nbytes - fn.bytes;
	p.nlocals = fs->nlocals - fn.locals;
	p.ncaches = langA_varlen(fn.caches);
	int f = lang_addproto(fs->md,p);

	langY_closefunc(fs);

	/* patch jump */
	langC_patchjtohere(fs,fj);

	/* todo: */
	NodeId *z = 0;
	langA_varifor(fn.caches) {
		int localid = fn.caches[i];
		int local = localid - fs->fn->locals;
		/* todo: here's the big question, should
		we preserve local nodes or not? We could
		store the node in the local structure? */
		NodeId n = langY_localnode(fs,fs->locals[localid].loc,local);
		langA_varadd(z,n);
	}

	return langY_nodeF(fs,tk.loc,f,z);
}


NodeId langY_parseinit(FileState *fs) {
	Token tk = fs->tk;
	langY_taketk(fs,TK_CURLY_LEFT);
	NodeId *z = Null;
	if (!langY_testtk(fs,TK_CURLY_RIGHT)) {
		do {
			NodeId x = langY_loadexpr(fs);
			if (x == -1) break;

			if (langY_testtk(fs,TK_ASSIGN)) {
				tk = langX_yield(fs);

				NodeId y = langY_loadexpr(fs);
				x = langY_designode(fs,tk.loc,x,y);
			}
			langA_varadd(z,x);
		} while(langY_picktk(fs,TK_COMMA));
	}
	langY_taketk(fs,TK_CURLY_RIGHT);

	NodeId x = langY_nodeH(fs,tk.loc,z);
	return x;
}


NodeId langY_loadexpr(FileState *fs) {
	NodeId x;
	switch (fs->tk.type) {
		case TK_CURLY_LEFT: {
			x = langY_parseinit(fs);
		} break;
		case TK_FUN: {
			x = langY_loadfunc(fs);
		} break;
		default: {
			x = langY_parsesubexpr(fs,0);
		} break;
	}
	return x;
}


int langY_parsestat(FileState *fs) {
	NodeId x = -1;
	switch (fs->tk.type) {
		case TK_LEAVE: {
			Token tk = langX_yield(fs);
			x = langY_loadexpr(fs);
			langC_leave(fs,tk.loc,x);
		} break;
		case TK_CURLY_LEFT: {
			langY_taketk(fs,TK_CURLY_LEFT);
			while (!langY_testtk(fs,TK_NONE) && !langY_testtk(fs,TK_CURLY_RIGHT)) {
				x = langY_parsestat(fs);
				if (x == -1) break;
			}
			langY_taketk(fs,TK_CURLY_RIGHT);
		} break;
		case TK_FOR: {
			Token tk = fs->tk;
			int y;
			langY_taketk(fs,TK_FOR);
			x = langY_loadexpr(fs);
			langY_taketk(fs,TK_IN);
			y = langY_loadexpr(fs);
			langY_taketk(fs,TK_QMARK);

			Loop loop = {0};
			langC_beginloop(fs,tk.loc,&loop,x,y);
			langY_parsestat(fs);
			langC_closeloop(fs,tk.loc,&loop);
		} break;
		case TK_IF: {
			Token tk = fs->tk;
			langY_taketk(fs,TK_IF);
			x = langY_loadexpr(fs);
			langY_taketk(fs,TK_QMARK);
			Select s = {0};
			langC_beginif(fs,tk.loc,&s,x);
			x = langY_parsestat(fs);
			while (langY_testtk(fs,TK_ELIF)) {
				tk = langX_yield(fs);
				x = langY_loadexpr(fs);
				langY_taketk(fs,TK_QMARK);
				// ?
				langC_addelif(fs,tk.loc,&s,x);
				x = langY_parsestat(fs);
			}
			tk = fs->tk;
			if (langY_picktk(fs,TK_ELSE)) {
				langC_addelse(fs,tk.loc,&s);
				x = langY_parsestat(fs);
			}
			langC_closeif(fs,fs->tk.loc,&s);
		} break;
		case TK_LOCAL: {
			Token tk = langX_yield(fs);
			Token n = langY_taketk(fs,TK_WORD);

			NodeId v = langY_localname(fs,n.loc,(char*)n.s);
			if (v == -1) goto leave;

			FuncState *fn = fs->fn;
			x = langY_localnode(fs,tk.loc,v - fn->locals);
			tk = fs->tk;
			if (langY_picktk(fs,TK_ASSIGN)) {
				int y = langY_loadexpr(fs);
				langC_store(fs,tk.loc,x,y);
			}
		} break;
		default: {
			x = langY_loadexpr(fs);
			if (langY_testtk(fs,TK_ASSIGN)) {
				Token tk = langX_yield(fs);
				int y = langY_loadexpr(fs);
				langC_store(fs,tk.loc,x,y);
			} else {
				langC_load(fs,0,x);
				// langC_pop(fs);
			}
		} break;
	}

	leave:
	return x;
}



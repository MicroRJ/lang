/*
** See Copyright Notice In lang.h
** (Y) lfile.c
** Parsing Stuff
*/




lbool langX_test(FileState *fs, ltokentype k) {
	return fs->tk.type == k;
}


/*
** Returns true whether there are no more tokens
** or whehter the current token is a match.
*/
lbool langX_term(FileState *fs, ltokentype k) {
	return fs->tk.type == TK_NONE || fs->tk.type == k;
}


/*
** Consumes the current token only if it is a
** match.
*/
lbool langX_pick(FileState *fs, ltokentype k) {
	return langX_test(fs,k) && (langX_yield(fs), ltrue);
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


void langY_enterlevel(FileState *fs) {
	++ fs->level;
}


void langY_checkassign(FileState *fs, llineid line, lnodeid x) {
	Node n = fs->nodes[x];
	if (n.k == NODE_LOCAL) {
		FileName fnn = fs->locals[fs->fn->locals+n.x];
		if (fnn.enm) {
			langX_error(fs,line,"enum value is constant, you're attempting to modify its value");
		}
	}
}


llocalid langY_numlocinlev(FileState *fs) {
	FileName *locals = fs->locals;
	llocalid nlocals = fs->nlocals;
	llocalid n;
	for (n = 0; n < nlocals; ++n) {
		if (locals[nlocals-1-n].level < fs->level) {
			break;
		}
	}
	return n;
}


lnodeid langY_numnodesinlev(FileState *fs) {
	Node *nodes = fs->nodes;
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
	llocalid nlocals = langY_numlocinlev(fs);
	if (nlocals != 0) {
		langL_localalloc(fs,line,-nlocals);
	}
	lnodeid nnodes = langY_numnodesinlev(fs);
	fs->nnodes -= nnodes;
	-- fs->level;
}


/*
** Looks for an cache value (closure value)
** pointing to x, if found returns the index
** within the array where it resides.
*/
int langY_cacheindex(FileFunc *fn, llocalid x) {
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
void langY_cachelocal(FileState *fs, FileFunc *fn, llocalid local) {
	/* ensure the local should actually be captured,
	either of these checks should suffice */
	LASSERT(local < fn->locals);

	/* is this local cached already? */
	langA_varifor(fn->caches) {
		if (fn->caches[i] == local) return;
	}
	langA_varadd(fn->caches,local);
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
llocalid langY_localorcacheid(FileState *fs, char *loc, char *name) {
	FileFunc *fn = fs->fn;
	llocalid locals = fn->locals;
	for (llocalid id = fs->nlocals-1; id >= 0; --id) {
		if (S_eq(fs->locals[id].name,name)) {
			if (id < locals) {
				langY_cachelocal(fs,fn,id);
			}
			return id;
		}
	}
	return -1;
}


/*
** Register a local variable within the current
** function, if already declared issue a warning
** but still return a valid id.
*/
lnodeid langY_enrolllocalnode(FileState *fs, llineid line, char *name, lbool enm) {
	llocalid id = langY_localorcacheid(fs,line,name);
	if (id == NO_SLOT) {
		FileFunc *fn = fs->fn;
		llocalid slot = langL_localalloc(fs,line,1);
		lnodeid node = langY_localnode(fs,line,slot);
		fs->locals[fn->locals+slot].name = name;
		fs->locals[fn->locals+slot].node = node;
		fs->locals[fn->locals+slot].enm  = enm;
		// if (enm) {
		// 	langX_error(fs,line,"'%s': enum, %i",name,slot);
		// }
		return node;
	} else {
		FileName local = fs->locals[id];
		/* is this variable name already present in this level? */
		if (local.level == fs->level) {
			langX_error(fs,line,"'%s': already declared",name);
		} else {
			langX_error(fs,line,"'%s': this declaration shadows another one",name);
		}
		return local.node;
	}
}


lnodeid langY_cacheorlocalnode(FileState *fs, char *line, char *name) {
	llocalid id = langY_localorcacheid(fs,line,name);
	if (id == -1) return -1;

	llocalid x = -1;

	FileFunc *fn = fs->fn;
	/* is this a capture? */
	if (id < fn->locals) {
		/* This is technically impossible, function
		at level 0 cannot capture anything above.
		if the level is less than 0, then we've
		royally screwed up. */
		if (fn->locals == 0) LNOCHANCE;

		if (id < fn->enclosing->locals) {
			langX_error(fs,line,"too many layers for caching");
		} else {
			x = langY_cachenode(fs,line,langY_cacheindex(fn,id));
		}
	} else {
		x = fs->locals[id].node;
	}
	return x;
}


void langY_beginfn(FileState *fs, FileFunc *fn, char *line) {
	langY_enterlevel(fs);
	fn->enclosing = fs->fn;
	fn->locals = fs->nlocals;
	fn->bytes = fs->md->nbytes;
	fn->line = line;
	fn->yj = lnil;
	fs->fn = fn;
}


void langY_closefn(FileState *fs) {
	langL_return(fs,fs->lasttk.line);
	langY_leavelevel(fs,fs->lasttk.line);
	/* ensure all locals were deallocated
	properly */
	LASSERT(fs->nlocals == fs->fn->locals);
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


/*
** Utility function that converts a binary
** operator token to a binary operator node,
** in retrospect, one could have used the
** token itself, but its no biggie.
*/
NodeType tonode(ltokentype tk) {
	switch (tk) {
		case TK_DOT_DOT:            return NODE_RANGE;
		case TK_LOG_AND:            return NODE_LOG_AND;
		case TK_LOG_OR:             return NODE_LOG_OR;
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
		case TK_LEFT_SHIFT:         return NODE_BSHL;
		case TK_RIGHT_SHIFT:        return NODE_BSHR;
		case TK_BIT_XOR:            return NODE_BXOR;
	}
	return NODE_NONE;
}


lnodeid langY_loadsubexpr(FileState *fs, int rank) {
	lnodeid x = langY_loadunary(fs);
	if (x == NO_NODE) return x;
	for (;;) {
		int thisrank = langX_tokenintel[fs->tk.type].prec;
		/* auto breaks when is not a binary operator */
		if (thisrank <= rank) break;
		ltoken tk = langX_yield(fs);
		lnodeid y = langY_loadsubexpr(fs,thisrank);
		if (y == NO_NODE) break;
		x = langY_nodexy(fs,tk.line,tonode(tk.type),x,y);
	}
	return x;
}


lnodeid langY_loadfn(FileState *fs) {

	ltoken tk = langX_take(fs,TK_FUN);

	/* All bytecode is outputted to the same
	module, the way this language works is
	that we just run the entire thing,
	for that reason add a jump byte to
	skip this function's code. */
	int fj = langL_jump(fs,tk.line,-1);

	FileFunc fn = {0};
	langY_beginfn(fs,&fn,tk.line);

	int arity = 0;

	langX_take(fs,TK_PAREN_LEFT);
	if (!langX_test(fs,TK_PAREN_RIGHT)) do {

		ltoken n = langX_take(fs,TK_WORD);
		langY_enrolllocalnode(fs,n.line,(char*)n.s,lfalse);

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
		lnodeid x = langY_loadexpr(fs);
		langL_yield(fs,tk.line,x);
	}

	langY_closefn(fs);

	/* add this function to the type table */
	Proto p = {0};
	p.x = arity;
	p.y = fn.nyield;
	p.nlocals = fn.nlocals;
	p.bytes = fn.bytes;
	p.nbytes  = fs->md->nbytes - fn.bytes;
	p.ncaches = langA_varlen(fn.caches);
	int f = lang_addproto(fs->md,p);

	/* patch jump */
	langL_tieloosej(fs,fj);

	/* todo: */
	lnodeid *z = lnil;
	langA_varifor(fn.caches) {
		lnodeid n = fs->locals[fn.caches[i]].node;
		langA_varadd(z,n);
	}

	return langY_nodeF(fs,tk.line,f,z);
}


lnodeid langY_loadtable(FileState *fs) {
	ltoken tk = fs->tk;
	langX_take(fs,TK_CURLY_LEFT);
	lnodeid *z = lnil;
	if (!langX_test(fs,TK_CURLY_RIGHT)) {
		do {
			lnodeid x = langY_loadexpr(fs);
			if (x == -1) break;

			if (langX_test(fs,TK_ASSIGN)) {
				tk = langX_yield(fs);

				lnodeid y = langY_loadexpr(fs);
				x = langY_designode(fs,tk.line,x,y);
			}
			langA_varadd(z,x);
		} while(langX_pick(fs,TK_COMMA));
	}
	langX_take(fs,TK_CURLY_RIGHT);
	return langY_nodeH(fs,tk.line,z);
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
			v = langY_loadexpr(fs);
			v = langY_nodexy(fs,tk.line,NODE_SUB,langY_nodeI(fs,tk.line,0),v);
		} break;
		case TK_ADD: {
			langX_yield(fs);
			v = langY_loadexpr(fs);
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
				v = langY_groupnode(fs,tk.line,v);
			}
		} break;
		case TK_FUN: {
			v = langY_loadfn(fs);
		} break;
		case TK_STKGET:
		case TK_STKLEN: {
			langX_yield(fs);
			lnodeid *z = langY_loadcallargs(fs);
			v = langY_builtinnode(fs,tk.line,tk.type,z);
		} break;
		case TK_LOAD: {
			langX_yield(fs);
			v = langY_loadexpr(fs);
			v = langY_loadfilenode(fs,tk.line,v);
		} break;
		case TK_WORD: {
			langX_yield(fs);

			v = langY_cacheorlocalnode(fs,tk.line,(char*)tk.s);

			if (v == NO_NODE) {
				/* todo:! gc'd string */
				lglobalid x = lang_addsymbol(fs->md,langS_new(fs->rt,tk.s));
				if (x != -1) {
					v = langY_globalnode(fs,tk.line,x);
					// langX_error(fs,tk.line,"global");
				}
			}

			if (v == -1) {
				langX_error(fs,tk.line,"'%s': undeclared identifier",tk.s);
			}
		} break;
		case TK_NIL: {
			langX_yield(fs);
			v = langY_nodeZ(fs,tk.line);
		} break;
		case TK_TRUE: {
			/* todo: maybe use proper boolean node? */
			langX_yield(fs);
			v = langY_nodeI(fs,tk.line,1);
		} break;
		case TK_FALSE: {
			/* todo: maybe use proper boolean node? */
			langX_yield(fs);
			v = langY_nodeI(fs,tk.line,0);
		} break;
		case TK_LETTER:
		case TK_INTEGER: {
			langX_yield(fs);
			v = langY_nodeI(fs,tk.line,tk.i);
		} break;
		case TK_NUMBER: {
			langX_yield(fs);
			v = langY_nodeN(fs,tk.line,tk.n);
		} break;
		case TK_STRING: {
			langX_yield(fs);
			v = langY_nodeS(fs,tk.line,(char*) tk.s);
		} break;
		default: {
			langX_error(fs,tk.line,"'%s': unexpected token", langX_tokenintel[tk.type].name);
		} break;
	}


	while (fs->tk.type != TK_NONE) {
		tk = fs->tk;
		switch (tk.type) {
			// x . { y }
			case TK_DOT: {
				langX_yield(fs);
				ltoken n = langX_take(fs,TK_WORD);
				lnodeid i = langY_nodeS(fs,n.line,(char*) n.s);
				v = langY_fieldnode(fs,tk.line,v,i);
			} break;
			// {x} . [ {x} ]
			case TK_SQUARE_LEFT: {
				langX_take(fs,TK_SQUARE_LEFT);
				lnodeid i = langY_loadexpr(fs);
				langX_take(fs,TK_SQUARE_RIGHT);
				if (fs->nodes[i].k == NODE_RANGE_INDEX) {
					LNOCHANCE;
				} else
				if (fs->nodes[i].k == NODE_RANGE) {
					v = langY_rangeindexnode(fs,tk.line,v,i);
				} else {
					v = langY_indexnode(fs,tk.line,v,i);
				}
			} break;
			/* {x} : {x} () */
			case TK_COLON: {
				langX_take(fs,TK_COLON);
				ltoken n = langX_take(fs,TK_WORD);
				lnodeid y = langY_nodeS(fs,n.line,(char*) n.s);
				lnodeid *z = langY_loadcallargs(fs);
				v = langY_metacallnode(fs,tk.line,v,y,z);
			} break;
			case TK_PAREN_LEFT: {
				lnodeid *z = langY_loadcallargs(fs);
				v = langY_callnode(fs,tk.line,v,z);
			} break;
			default: goto leave;
		}
	}

	leave:
	return v;
}


/* todo: make this legit */
void langY_loadenumlist(FileState *fs) {
	llong nextvalue = 0;
	if (!langX_test(fs,TK_CURLY_RIGHT)) {
		do {
			/* , } */
			if (langX_test(fs,TK_CURLY_RIGHT)) {
				break;
			}
			ltoken tk = fs->tk;
			ltoken n = langX_take(fs,TK_WORD);
			lnodeid x = langY_enrolllocalnode(fs,n.line,(char*)n.s,ltrue);
			llong l = nextvalue;
			if (langX_pick(fs,TK_ASSIGN)) {
				tk = fs->tk;
				ltoken v = langX_take(fs,TK_INTEGER);
				l = v.i;
			} else nextvalue ++;
			lnodeid y = langY_nodeI(fs,tk.line,l);
			langL_loadinto(fs,tk.line,x,y);
		} while(langX_pick(fs,TK_COMMA));
	}
}


void langY_loadstat(FileState *fs) {
	switch (fs->tk.type) {
		case TK_THEN:
		case TK_ELSE:
		case TK_ELIF: {
		} break;
		case TK_LEAVE: {
			ltoken tk = langX_yield(fs);
			lnodeid x = langY_loadexpr(fs);
			langL_yield(fs,tk.line,x);
		} break;
		case TK_CURLY_LEFT: {
			langY_enterlevel(fs);
			langX_take(fs,TK_CURLY_LEFT);
			while (!langX_term(fs,TK_CURLY_RIGHT)) {
				langY_loadstat(fs);
			}
			langX_take(fs,TK_CURLY_RIGHT);
			langY_leavelevel(fs,fs->lasttk.line);
		} break;
		case TK_FINALLY: {
			ltoken tk = langX_take(fs,TK_FINALLY);

			CodeBlock bl = {0};
			langL_begindelayedblock(fs,tk.line,&bl);
			langY_loadstat(fs);
			langL_closedelayedblock(fs,tk.line,&bl);

		} break;
		case TK_WHILE: {
			ltoken tk = langX_take(fs,TK_WHILE);
			lnodeid x = langY_loadexpr(fs);
			langX_take(fs,TK_QMARK);
			Loop loop = {0};
			langL_beginwhile(fs,tk.line,&loop,x);
			langY_loadstat(fs);
			langL_closewhile(fs,tk.line,&loop);
		} break;
		case TK_DO: {
			ltoken tk = langX_take(fs,TK_DO);
			Loop loop = {0};
			langL_begindowhile(fs,tk.line,&loop);
			langY_loadstat(fs);
			langX_take(fs,TK_WHILE);
			lnodeid x = langY_loadexpr(fs);
			langL_closedowhile(fs,tk.line,&loop,x);
		} break;
		case TK_FOR: {
			ltoken tk = langX_take(fs,TK_FOR);
			lnodeid x = langY_loadexpr(fs);
			langX_take(fs,TK_IN);
			lnodeid y = langY_loadexpr(fs);
			langX_take(fs,TK_QMARK);

			Loop loop = {0};
			langL_beginloop(fs,tk.line,&loop,x,y);
			langY_loadstat(fs);
			langL_closeloop(fs,tk.line,&loop);
		} break;
		case TK_IF:
		case TK_IFF: {
			ltoken tk = langX_yield(fs);
			lnodeid x = langY_loadexpr(fs);
			langX_take(fs,TK_QMARK);
			Select s = {0};
			langL_beginif(fs,tk.line,&s,x
			,	tk.type == TK_IFF ? L_IFF : L_IF);
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
		} break;
		case TK_LET:
		case TK_LOCAL: {
			ltoken tk = langX_yield(fs);
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
				ltoken n = langX_take(fs,TK_WORD);
				lnodeid x = langY_enrolllocalnode(fs,n.line,(char*)n.s,enm);
				if (langX_pick(fs,TK_ASSIGN)) {
					tk = fs->lasttk;
					lnodeid y = langY_loadexpr(fs);
					langL_loadinto(fs,tk.line,x,y);
				}
			}

		} break;
		default: {
			lnodeid x = langY_loadexpr(fs);

			ltoken tk = fs->tk;
			if (langX_pick(fs,TK_ASSIGN_QUESTION)) {
				langY_checkassign(fs,tk.line,x);
				lnodeid c = langY_nodexy(fs,tk.line,NODE_EQ,x,langY_nodeZ(fs,tk.line));
				lnodeid y = langY_loadexpr(fs);
				ljlist js = {0};
				langL_jumpiffalse(fs,&js,c);
				langL_tieloosejs(fs,js.t);
				langL_loadinto(fs,tk.line,x,y);
				langL_tieloosejs(fs,js.f);
			} else
			if (langX_pick(fs,TK_ASSIGN)) {
				langY_checkassign(fs,tk.line,x);
				lnodeid y = langY_loadexpr(fs);
				langL_loadinto(fs,tk.line,x,y);
			} else {
				langL_loaddrop(fs,lnil,x);
			}
		} break;
	}
}



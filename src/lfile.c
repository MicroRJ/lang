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


lentityid langY_allocentity(FileState *fs, llineid line, lentityid nentitys) {
	FileFunc *ff = fs->fn;
	lentityid id = fs->nentitys;
	fs->nentitys += nentitys;
	langA_variadd(fs->entitys,nentitys);

	if (nentitys > 0) {
		for (llocalid i = id; i < fs->nentitys; ++i) {
			fs->entitys[i].level = fs->level;
			fs->entitys[i].line  = line;
			fs->entitys[i].name  = 0;
			fs->entitys[i].slot  = 0;
			fs->entitys[i].enm   = lfalse;
		}
	}
	// langX_error(fs,line,"%i, fs=%i, ff=%i",id,fs->nentitys,nentitys);
	return id;
}


llocalid langY_numentinlevl(FileState *fs) {
	lentity *entitys = fs->entitys;
	int nentitys = fs->nentitys;
	int n;
	for (n = 0; n < nentitys; ++ n) {
		if (entitys[nentitys-1-n].level < fs->level) {
			break;
		}
	}
	return n;
}


ltreeid langY_numtreesinlev(FileState *fs) {
	Tree *nodes = fs->nodes;
	ltreeid nnodes = fs->nnodes;
	ltreeid n;
	for (n = 0; n < nnodes; ++n) {
		if (nodes[nnodes-1-n].level < fs->level) {
			break;
		}
	}
	return n;
}


void langY_leavelevel(FileState *fs, llineid line) {
	/* garbage collection? */
	fs->nentitys -= langY_numentinlevl(fs);
	fs->nnodes -= langY_numtreesinlev(fs);
	/* ensure that we don't deallocate more than
	we can */
	LASSERT(fs->nentitys >= fs->fn->entitys);
	-- fs->level;
}


void langY_enterlevel(FileState *fs) {
	++ fs->level;
}


void langY_checkassign(FileState *fs, llineid line, ltreeid x) {
	/* todo: welp, we can't do this anymore */
	// Tree n = fs->nodes[x];
	// if (n.k == Y_LOCAL) {
	// 	lentity fnn = fs->entitys[fs->fn->entitys+n.x];
	// 	if (fnn.enm) {
	// 		langX_error(fs,line,"enum value is constant, you're attempting to modify its value");
	// 	}
	// }
}


/*
** Looks for an cache value (closure value)
** pointing to x, if found returns the index
** within the cache where it resides.
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
void langY_cachelocal(FileState *fs, FileFunc *fn, llocalid loctag) {
	/* ensure the loctag should actually be captured,
	either of these checks should suffice */
	LASSERT(loctag < fn->entitys);

	/* is this loctag cached already? */
	langA_varifor(fn->caches) {
		if (fn->caches[i] == loctag) return;
	}
	langA_varadd(fn->caches,loctag);
}


/*
** Find the closest local tag in order of lexical
** relevance, starting from the last declared tag.
** The id returned is an absolute index into
** fs->entitys, you should make it relative
** to the current function.
** If the local is outside of this function,
** then it caches it.
*/
lentityid langY_fndentity(FileState *fs, llineid line, char *name) {
	FileFunc *fn = fs->fn;
	lentityid entitys = fn->entitys;
	for (lentityid id = fs->nentitys-1; id >= 0; --id) {
		if (S_eq(fs->entitys[id].name,name)) {
			if (id < entitys) {
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
llocalid langY_newloctag(FileState *fs, llineid line, char *name, lbool enm) {
	FileFunc *fn = fs->fn;
	lentityid id = langY_fndentity(fs,line,name);
	if (id == NO_ENTITY) {
		id = langY_allocentity(fs,line,1);

		/* -- todo: instead replace this with flags,
		like if it is a variable or so. */
		llocalid slot = langL_localalloc(fs,1);
		fs->entitys[id].slot = slot;
		fs->entitys[id].enm  = enm;

		fs->entitys[id].name = name;

		return langY_treelocal(fs,line,slot);
		// if (enm) {
		// 	langX_error(fs,line,"'%s': enum, %i",name,slot);
		// }
	} else {
		lentity loctag = fs->entitys[id];
		/* is this variable name already present in this level? */
		if (loctag.level == fs->level) {
			langX_error(fs,line,"'%s': already declared",name);
		} else {
			langX_error(fs,line,"'%s': this declaration shadows another one",name);
		}
		return langY_treelocal(fs,line,loctag.slot);
	}
}


ltreeid langY_cacheorlocaltree(FileState *fs, llineid line, char *name) {
	lentityid id = langY_fndentity(fs,line,name);
	if (id == NO_ENTITY) return NO_TREE;
	FileFunc *fn = fs->fn;
	llocalid slot = fs->entitys[id].slot;
	/* -- the slot or register index in the entity
	- is relative to its function, there's no
	- way to know whether this is a cache value by
	- simply looking at the register, instead we
	- use the entity id and check whether it
	- is above the current function's scope,
	- which should tells us whether this is a
	- cache value or not. */
	if (id < fn->entitys) {
		/* -- todo: implement multilayer caching */
		if (id < fn->enclosing->entitys) {
			langX_error(fs,line,"too many layers for caching");
		}
		return langY_treecache(fs,line,slot);
	} else return langY_treelocal(fs,line,slot);
}


void langY_beginfn(FileState *fs, FileFunc *fn, char *line) {
	langY_enterlevel(fs);
	fn->enclosing = fs->fn;
	fn->entitys = fs->nentitys;
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
	LASSERT(fs->nentitys == fs->fn->entitys);
	fs->fn = fs->fn->enclosing;
}


ltreeid *langY_loadcallargs(FileState *fs) {
	/* ( x { , x } ) */
	ltreeid *z = 0;
	if (langX_pick(fs,TK_PAREN_LEFT)) {
		if (!langX_test(fs,TK_PAREN_RIGHT)) do {
			ltreeid x = langY_loadexpr(fs);
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
ltreetype tktonode(ltokentype tk) {
	switch (tk) {
		case TK_DOT_DOT:            return Y_RANGE;
		case TK_LOG_AND:            return Y_LOG_AND;
		case TK_LOG_OR:             return Y_LOG_OR;
		case TK_ADD:                return Y_ADD;
		case TK_SUB:                return Y_SUB;
		case TK_DIV:                return Y_DIV;
		case TK_MUL:                return Y_MUL;
		case TK_MODULUS:            return Y_MOD;
		case TK_NOT_EQUALS:         return Y_NEQ;
		case TK_EQUALS:             return Y_EQ;
		case TK_GREATER_THAN:       return Y_GT;
		case TK_GREATER_THAN_EQUAL: return Y_GTEQ;
		case TK_LESS_THAN:          return Y_LT;
		case TK_LESS_THAN_EQUAL:    return Y_LTEQ;
		case TK_LEFT_SHIFT:         return Y_BSHL;
		case TK_RIGHT_SHIFT:        return Y_BSHR;
		case TK_BIT_XOR:            return Y_BXOR;
	}
	return Y_NONE;
}


ltreeid langY_loadsubexpr(FileState *fs, int rank) {
	ltreeid x = langY_loadunary(fs);
	if (x == NO_TREE) return x;
	for (;;) {
		int thisrank = langX_tokenintel[fs->tk.type].prec;
		/* auto breaks when is not a binary operator */
		if (thisrank <= rank) break;
		ltoken tk = langX_yield(fs);
		ltreeid y = langY_loadsubexpr(fs,thisrank);
		if (y == NO_TREE) break;
		x = langY_treexy(fs,tk.line,tktonode(tk.type),x,y);
	}
	return x;
}


ltreeid langY_loadfn(FileState *fs) {

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
		langY_newloctag(fs,n.line,(char*)n.s,lfalse);

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
		ltreeid x = langY_loadexpr(fs);
		langL_yield(fs,tk.line,x);
	}

	langY_closefn(fs);

	/* add this function to the type table */
	lProto p = {0};
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
	ltreeid *z = lnil;
	langA_varifor(fn.caches) {
		langA_varadd(z
		,	langY_treelocal(fs,tk.line,fn.caches[i]));
	}

	return langY_treeclosure(fs,tk.line,f,z);
}


ltreeid langY_loadtable(FileState *fs) {
	ltoken tk = fs->tk;
	langX_take(fs,TK_CURLY_LEFT);
	ltreeid *z = lnil;
	if (!langX_test(fs,TK_CURLY_RIGHT)) {
		do {
			ltreeid x = langY_loadexpr(fs);
			if (x == -1) break;

			if (langX_test(fs,TK_ASSIGN)) {
				tk = langX_yield(fs);

				ltreeid y = langY_loadexpr(fs);
				x = langY_treedesig(fs,tk.line,x,y);
			}
			langA_varadd(z,x);
		} while(langX_pick(fs,TK_COMMA));
	}
	langX_take(fs,TK_CURLY_RIGHT);
	return langY_treetable(fs,tk.line,z);
}


ltreeid langY_loadexpr(FileState *fs) {
	/* todo?: do this in else where? */
	switch (fs->tk.type) {
		case TK_NONE:
		case TK_PAREN_RIGHT:
		case TK_CURLY_RIGHT:
		case TK_SQUARE_RIGHT: {
			return NO_TREE;
		}
	}
	return langY_loadsubexpr(fs,0);
}


ltreeid langY_loadunary(FileState *fs) {
	ltreeid v = - 1;
	ltoken tk = fs->tk;
	switch (tk.type) {
		case TK_SUB: {
			langX_yield(fs);
			v = langY_loadexpr(fs);
			v = langY_treexy(fs,tk.line,Y_SUB,langY_treelongint(fs,tk.line,0),v);
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
			if (v != NO_TREE) {
				v = langY_treegroup(fs,tk.line,v);
			}
		} break;
		case TK_FUN: {
			v = langY_loadfn(fs);
		} break;
		case TK_STKGET:
		case TK_STKLEN: {
			langX_yield(fs);
			ltreeid *z = langY_loadcallargs(fs);
			v = langY_treebuiltincall(fs,tk.line,tk.type,z);
		} break;
		case TK_LOAD: {
			langX_yield(fs);
			v = langY_loadexpr(fs);
			v = langY_treeloadfile(fs,tk.line,v);
		} break;
		case TK_WORD: {
			langX_yield(fs);

			v = langY_cacheorlocaltree(fs,tk.line,(char*)tk.s);

			if (v == NO_TREE) {
				/* todo:! gc'd string */
				lglobalid x = lang_addsymbol(fs->md,langS_new(fs->rt,tk.s));
				if (x != -1) {
					v = langY_treeglobal(fs,tk.line,x);
					// langX_error(fs,tk.line,"global");
				}
			}

			if (v == -1) {
				langX_error(fs,tk.line,"'%s': undeclared identifier",tk.s);
			}
		} break;
		case TK_NIL: {
			langX_yield(fs);
			v = langY_treenil(fs,tk.line);
		} break;
		case TK_TRUE: {
			/* todo: maybe use proper boolean node? */
			langX_yield(fs);
			v = langY_treelongint(fs,tk.line,1);
		} break;
		case TK_FALSE: {
			/* todo: maybe use proper boolean node? */
			langX_yield(fs);
			v = langY_treelongint(fs,tk.line,0);
		} break;
		case TK_LETTER:
		case TK_INTEGER: {
			langX_yield(fs);
			v = langY_treelongint(fs,tk.line,tk.i);
		} break;
		case TK_NUMBER: {
			langX_yield(fs);
			v = langY_treenumber(fs,tk.line,tk.n);
		} break;
		case TK_STRING: {
			langX_yield(fs);
			v = langY_treeString(fs,tk.line,(char*) tk.s);
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
				ltreeid i = langY_treeString(fs,n.line,(char*) n.s);
				v = langY_treefield(fs,tk.line,v,i);
			} break;
			// {x} . [ {x} ]
			case TK_SQUARE_LEFT: {
				langX_take(fs,TK_SQUARE_LEFT);
				ltreeid i = langY_loadexpr(fs);
				langX_take(fs,TK_SQUARE_RIGHT);
				if (fs->nodes[i].k == Y_RANGE_INDEX) {
					LNOBRANCH;
				} else
				if (fs->nodes[i].k == Y_RANGE) {
					v = langY_treerangedindex(fs,tk.line,v,i);
				} else {
					v = langY_treeindex(fs,tk.line,v,i);
				}
			} break;
			/* {x} : {x} () */
			case TK_COLON: {
				langX_take(fs,TK_COLON);
				ltoken n = langX_take(fs,TK_WORD);
				ltreeid y = langY_treeString(fs,n.line,(char*) n.s);
				ltreeid *z = langY_loadcallargs(fs);
				v = langY_treemetacall(fs,tk.line,v,y,z);
			} break;
			case TK_PAREN_LEFT: {
				ltreeid *z = langY_loadcallargs(fs);
				v = langY_treecall(fs,tk.line,v,z);
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
			ltreeid x = langY_newloctag(fs,n.line,(char*)n.s,ltrue);
			langX_take(fs,TK_ASSIGN);
			ltreeid y = langY_loadexpr(fs);
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
			ltreeid x = langY_loadexpr(fs);
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
			ltreeid x = langY_loadexpr(fs);
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
			ltreeid x = langY_loadexpr(fs);
			langL_closedowhile(fs,tk.line,&loop,x);
		} break;
		case TK_FOR: {
			ltoken tk = langX_take(fs,TK_FOR);
			ltreeid x = langY_loadexpr(fs);
			langX_take(fs,TK_IN);
			ltreeid y = langY_loadexpr(fs);
			langX_take(fs,TK_QMARK);

			Loop loop = {0};
			langL_beginloop(fs,tk.line,&loop,x,y);
			langY_loadstat(fs);
			langL_closeloop(fs,tk.line,&loop);
		} break;
		case TK_IF:
		case TK_IFF: {
			ltoken tk = langX_yield(fs);
			ltreeid x = langY_loadexpr(fs);
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
				ltreeid x = langY_newloctag(fs,n.line,(char*)n.s,enm);
				if (langX_pick(fs,TK_ASSIGN)) {
					tk = fs->lasttk;
					ltreeid y = langY_loadexpr(fs);
					langL_loadinto(fs,tk.line,x,y);
				}
			}

		} break;
		default: {
			ltreeid x = langY_loadexpr(fs);

			ltoken tk = fs->tk;
			if (langX_pick(fs,TK_ASSIGN_QUESTION)) {
				langY_checkassign(fs,tk.line,x);
				ltreeid c = langY_treexy(fs,tk.line,Y_EQ,x,langY_treenil(fs,tk.line));
				ltreeid y = langY_loadexpr(fs);
				ljlist js = {0};
				langL_jumpiffalse(fs,&js,c);
				langL_tieloosejs(fs,js.t);
				langL_loadinto(fs,tk.line,x,y);
				langL_tieloosejs(fs,js.f);
			} else
			if (langX_pick(fs,TK_ASSIGN)) {
				langY_checkassign(fs,tk.line,x);
				ltreeid y = langY_loadexpr(fs);
				langL_loadinto(fs,tk.line,x,y);
			} else {
				langL_reload(fs,tk.line,lfalse,NO_SLOT,0,x);
			}
		} break;
	}
}



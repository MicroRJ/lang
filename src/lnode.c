/*
** See Copyright Notice In lang.h
** (Y) lnode.c
** IR?...
*/


lnodeid langN_xyz(FileState *fs, llineid line, lnodeop k, lnodety t, lnodeid x, lnodeid y, lnodeid *z) {
	if (langA_varmin(fs->nodes) <= fs->nnodes) {
		langA_variadd(fs->nodes,1);
	}
	lNode *nd = fs->nodes + fs->nnodes;
	nd->level = fs->level;
	nd->line = line;
	nd->t = t; nd->k = k;
	nd->r = NO_SLOT;
	/* todo: temporary */
	if (k == NODE_LOCAL) nd->r = x;

	nd->x = x; nd->y = y; nd->z = z;
	return fs->nnodes ++;
}


lnodeid langN_xy(FileState *fs, llineid line, lnodeop k, lnodety t, lnodeid x, lnodeid y) {
	return langN_xyz(fs,line,k,t,x,y,lnil);
}


lnodeid langN_x(FileState *fs, llineid line, lnodeop k, lnodety t, lnodeid x) {
	return langN_xy(fs,line,k,t,x,NO_NODE);
}


lnodeid langN_nullary(FileState *fs, llineid line, lnodeop k, lnodety t) {
	return langN_x(fs,line,k,t,NO_NODE);
}


lnodeid langN_load(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return langN_xy(fs,line,NODE_LOAD,NT_ANY,x,y);
}


lnodeid langN_typeguard(FileState *fs, llineid line, lnodeid x, lnodety y) {
	lnodeid id = langN_xy(fs,line,NODE_TYPEGUARD,y,x,y);
	/* todo: please, figure out how to solve this */
	if (fs->nodes[x].k == NODE_LOCAL) {
		fs->nodes[id].r = fs->nodes[x].r;
	}
	return id;
}


lnodeid langN_group(FileState *fs, llineid line, lnodeid x) {
	return langN_x(fs,line,NODE_GROUP,fs->nodes[x].t,x);
}


lnodeid langN_longint(FileState *fs, llineid line, llongint i) {
	int v = langN_nullary(fs,line,NODE_INTEGER,NT_INT);
	fs->nodes[v].lit.i = i;
	return v;
}


lnodeid langN_number(FileState *fs, llineid line, lnumber n) {
	int v = langN_nullary(fs,line,NODE_NUMBER,NT_NUM);
	fs->nodes[v].lit.n = n;
	return v;
}


lnodeid langN_S(FileState *fs, llineid line, char *s) {
	int v = langN_nullary(fs,line,NODE_STRING,NT_STR);
	fs->nodes[v].lit.s = s;
	return v;
}


lnodeid langN_table(FileState *fs, llineid line, lnodeid *z) {
	return langN_xyz(fs,line,NODE_TABLE,NT_TAB,NO_NODE,NO_NODE,z);
}


lnodeid langN_closure(FileState *fs, llineid line, lnodeid x, lnodeid *z) {
	return langN_xyz(fs,line,NODE_CLOSURE,NT_FUN,x,NO_NODE,z);
}


lnodeid langN_nil(FileState *fs, llineid line) {
	return langN_nullary(fs,line,NODE_NIL,NT_NIL);
}


lnodeid langN_cache(FileState *fs, llineid line, llocalid x) {
	return langN_x(fs,line,NODE_CACHE,NT_ANY,x);
}


lnodeid langN_local(FileState *fs, llineid line, llocalid x) {
	return langN_x(fs,line,NODE_LOCAL,NT_ANY,x);
}


lnodeid langN_global(FileState *fs, llineid line, lglobalid x) {
	return langN_x(fs,line,NODE_GLOBAL,NT_ANY,x);
}


lnodeid langN_field(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return langN_xy(fs,line,NODE_FIELD,NT_ANY,x,y);
}


lnodeid langN_index(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return langN_xy(fs,line,NODE_INDEX,NT_ANY,x,y);
}


lnodeid langN_rangedindex(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return langN_xy(fs,line,NODE_RANGE_INDEX,NT_ANY,x,y);
}


lnodeid langN_metafield(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return langN_xy(fs,line,NODE_METAFIELD,NT_ANY,x,y);
}


lnodeid langN_call(FileState *fs, llineid line, lnodeid x, lnodeid *z) {
	return langN_xyz(fs,line,NODE_CALL,NT_ANY,x,NO_NODE,z);
}


lnodeid langN_loadfile(FileState *fs, llineid line, lnodeid x) {
	return langN_x(fs,line,NODE_FILE,NT_ANY,x);
}


lnodeid langN_builtincall(FileState *fs, llineid line, ltokentype k, lnodeid *z) {
	return langN_xyz(fs,line,NODE_BUILTIN,NT_ANY,k,NO_NODE,z);
}


lValue langN_tolitval(FileState *fs, lnodeid id);


void langN_litapply(FileState *fs, lTable *tab, lnodeid id) {
	lNode v = fs->nodes[id];
	switch (v.k) {
		case NODE_LOAD: {
			lNode x = fs->nodes[v.x];
			if ((x.k == NODE_LOCAL)) {
				LNOBRANCH;
			} else
			if ((x.k == NODE_FIELD) || (x.k == NODE_INDEX)) {
				lValue key = langN_tolitval(fs,x.x);
				lValue val = langN_tolitval(fs,v.y);
				langH_insert(tab,key,val);
			} else LNOBRANCH;
		} break;
		default: LNOBRANCH;
	}
}


lValue langN_tolitval(FileState *fs, lnodeid id) {
	lNode nd = fs->nodes[id];
	switch (nd.k) {
		case NODE_INTEGER: {
			return lang_I(nd.lit.i);
		}
		case NODE_NUMBER: {
			return lang_N(nd.lit.n);
		}
		case NODE_TABLE: {
			lTable *tab = langH_new(fs->R);
			langA_varifor(nd.z) {
				langN_litapply(fs,tab,nd.z[i]);
			}
			return lang_H(tab);
		}
		default: LNOBRANCH;
	}
	return (lValue){TAG_NIL};
}
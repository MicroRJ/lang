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
	if (k == N_LOCAL) nd->r = x;

	nd->x = x; nd->y = y; nd->z = z;
	return fs->nnodes ++;
}


lnodeid langN_xy(FileState *fs, llineid line, lnodeop k, lnodety t, lnodeid x, lnodeid y) {
	return langN_xyz(fs,line,k,t,x,y,lnil);
}


lnodeid langN_x(FileState *fs, llineid line, lnodeop k, lnodety t, lnodeid x) {
	return langN_xy(fs,line,k,t,x,NO_NODE);
}


lnodeid lanN_node(FileState *fs, llineid line, lnodeop k, lnodety t) {
	return langN_x(fs,line,k,t,NO_NODE);
}


lnodeid langN_load(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return langN_xy(fs,line,NODE_LOAD,NT_ANY,x,y);
}


lnodeid langN_group(FileState *fs, llineid line, lnodeid x) {
	return langN_x(fs,line,Y_GROUP,fs->nodes[x].t,x);
}


lnodeid langN_longint(FileState *fs, llineid line, llongint i) {
	int v = lanN_node(fs,line,Y_INTEGER,NT_INT);
	fs->nodes[v].lit.i = i;
	return v;
}


lnodeid langN_number(FileState *fs, llineid line, lnumber n) {
	int v = lanN_node(fs,line,Y_NUMBER,NT_NUM);
	fs->nodes[v].lit.n = n;
	return v;
}


lnodeid langN_string(FileState *fs, llineid line, char *s) {
	int v = lanN_node(fs,line,Y_STRING,NT_STR);
	fs->nodes[v].lit.s = s;
	return v;
}


lnodeid langN_table(FileState *fs, llineid line, lnodeid *z) {
	return langN_xyz(fs,line,N_TABLE,NT_TAB,NO_NODE,NO_NODE,z);
}


lnodeid langN_closure(FileState *fs, llineid line, lnodeid x, lnodeid *z) {
	return langN_xyz(fs,line,N_FUN,NT_FUN,x,NO_NODE,z);
}


lnodeid langN_nil(FileState *fs, llineid line) {
	return lanN_node(fs,line,N_NIL,NT_NIL);
}


lnodeid langN_cache(FileState *fs, llineid line, llocalid x) {
	return langN_x(fs,line,N_CACHE,NT_ANY,x);
}


lnodeid langN_local(FileState *fs, llineid line, llocalid x) {
	return langN_x(fs,line,N_LOCAL,NT_ANY,x);
}


lnodeid langN_global(FileState *fs, llineid line, lglobalid x) {
	return langN_x(fs,line,Y_GLOBAL,NT_ANY,x);
}


lnodeid langN_field(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return langN_xy(fs,line,NODE_FIELD,NT_ANY,x,y);
}


lnodeid langN_index(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return langN_xy(fs,line,NODE_INDEX,NT_ANY,x,y);
}


lnodeid langN_rangedindex(FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return langN_xy(fs,line,Y_RANGE_INDEX,NT_ANY,x,y);
}


lnodeid langN_metacall(FileState *fs, llineid line, lnodeid x, lnodeid y, lnodeid *z) {
	return langN_xyz(fs,line,NODE_METACALL,NT_ANY,x,y,z);
}


lnodeid langN_call(FileState *fs, llineid line, lnodeid x, lnodeid *z) {
	return langN_xyz(fs,line,NODE_CALL,NT_ANY,x,NO_NODE,z);
}


lnodeid langN_loadfile(FileState *fs, llineid line, lnodeid x) {
	return langN_x(fs,line,Y_LOADFILE,NT_ANY,x);
}


lnodeid langN_builtincall(FileState *fs, llineid line, ltokentype k, lnodeid *z) {
	return langN_xyz(fs,line,Y_BUILTIN,NT_ANY,k,NO_NODE,z);
}
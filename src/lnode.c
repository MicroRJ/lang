/*
** See Copyright Notice In lang.h
** (Y) lnode.c
** Simple AST For Expressions
*/


lnodeid langY_nodexyz(FileState *fs, char *line, NodeType k, lnodeid x, lnodeid y, lnodeid *z) {
	if (langA_varmin(fs->nodes) <= fs->nnodes) {
		langA_variadd(fs->nodes,1);
	}

	Node *nd = fs->nodes + fs->nnodes;
	nd->level = fs->level;
	nd->line = line;
	nd->k = k;
	nd->x = x;
	nd->y = y;
	nd->z = z;
	return fs->nnodes ++;
}


lnodeid langY_nodexy(FileState *fs, char *line, NodeType k, lnodeid x, lnodeid y) {
	return langY_nodexyz(fs,line,k,x,y,lnil);
}


lnodeid langY_nodex(FileState *fs, char *line, NodeType k, lnodeid x) {
	return langY_nodexy(fs,line,k,x,NO_NODE);
}


lnodeid langY_node(FileState *fs, char *line, int k) {
	return langY_nodex(fs,line,k,NO_NODE);
}


lnodeid langY_groupnode(FileState *fs, char *line, lnodeid x) {
	return langY_nodex(fs,line,NODE_GROUP,x);
}


lnodeid langY_nodeI(FileState *fs, char *line, llong i) {
	int v = langY_node(fs,line,NODE_INTEGER);
	fs->nodes[v].lit.i = i;
	return v;
}


lnodeid langY_nodeN(FileState *fs, char *line, lnumber n) {
	int v = langY_node(fs,line,NODE_NUMBER);
	fs->nodes[v].lit.n = n;
	return v;
}


lnodeid langY_nodeS(FileState *fs, char *line, char *s) {
	int v = langY_node(fs,line,NODE_STRING);
	fs->nodes[v].lit.s = s;
	return v;
}


lnodeid langY_nodeH(FileState *fs, char *line, lnodeid *z) {
	return langY_nodexyz(fs,line,NODE_TABLE,NO_NODE,NO_NODE,z);
}


lnodeid langY_nodeF(FileState *fs, char *line, lnodeid x, lnodeid *z) {
	return langY_nodexyz(fs,line,NODE_FUNCTION,x,NO_NODE,z);
}


lnodeid langY_nodeZ(FileState *fs, char *line) {
	return langY_node(fs,line,NODE_NIL);
}


lnodeid langY_cachenode(FileState *fs, char *line, lnodeid i) {
	return langY_nodex(fs,line,NODE_CACHE,i);
}


lnodeid langY_localnode(FileState *fs, char *line, lnodeid i) {
	// langX_error(fs,line,"locanode %i",i);
	return langY_nodex(fs,line,NODE_LOCAL,i);
}


lnodeid langY_globalnode(FileState *fs, char *line, lnodeid i) {
	return langY_nodex(fs,line,NODE_GLOBAL,i);
}


lnodeid langY_fieldnode(FileState *fs, char *line, lnodeid x, lnodeid y) {
	return langY_nodexy(fs,line,NODE_FIELD,x,y);
}


lnodeid langY_indexnode(FileState *fs, char *line, lnodeid x, lnodeid y) {
	return langY_nodexy(fs,line,NODE_INDEX,x,y);
}


lnodeid langY_designode(FileState *fs, char *line, lnodeid x, lnodeid y) {
	return langY_nodexy(fs,line,NODE_DESIG,x,y);
}


lnodeid langY_rangeindexnode(FileState *fs, char *line, lnodeid x, lnodeid y) {
	return langY_nodexy(fs,line,NODE_RANGE_INDEX,x,y);
}


lnodeid langY_metacallnode(FileState *fs, char *line, lnodeid x, lnodeid y, lnodeid *z) {
	return langY_nodexyz(fs,line,NODE_MCALL,x,y,z);
}


lnodeid langY_callnode(FileState *fs, char *line, lnodeid x, lnodeid *z) {
	return langY_nodexyz(fs,line,NODE_CALL,x,NO_NODE,z);
}


lnodeid langY_loadfilenode(FileState *fs, char *line, lnodeid x) {
	return langY_nodex(fs,line,NODE_LOADFILE,x);
}
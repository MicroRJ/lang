/*
** See Copyright Notice In lang.h
** lnode.c
** (Y) Expressing Parsing Stuff
*/


NodeId langY_node3(FileState *fs, char * line, NodeName k, NodeId x, NodeId y, NodeId *z) {
	LASSERT(x >= 0);
	LASSERT(y >= 0);
	/* todo: node recycler */
	Node *v = langA_varnadd(fs->nodes,1);
	v->line = line;
	v->k = k;
	v->x = x;
	v->y = y;
	v->z = z;
	return fs->nnodes ++;
}


NodeId langY_node2(FileState *fs, char *loc, int k, int x, int y) {
	return langY_node3(fs,loc,k,x,y,0);
}


NodeId langY_nodeI(FileState *fs, char *loc, Integer i) {
	int v = langY_node3(fs,loc,NODE_INTEGER,0,0,0);
	fs->nodes[v].lit.i = i;
	return v;
}


NodeId langY_nodeS(FileState *fs, char *loc, char const *s) {
	int v = langY_node3(fs,loc,NODE_STRING,0,0,0);
	fs->nodes[v].lit.s = s;
	return v;
}


NodeId langY_nodeH(FileState *fs, char *loc, NodeId *z) {
	return langY_node3(fs,loc,NODE_TABLE,0,0,z);
}


NodeId langY_nodeF(FileState *fs, char *loc, NodeId x, NodeId *z) {
	return langY_node3(fs,loc,NODE_FUNCTION,x,0,z);
}


NodeId langY_cachenode(FileState *fs, char *loc, NodeId i) {
	return langY_node3(fs,loc,NODE_CACHE,i,0,0);
}


NodeId langY_localnode(FileState *fs, char *loc, NodeId i) {
	return langY_node3(fs,loc,NODE_LOCAL,i,0,0);
}


NodeId langY_globalnode(FileState *fs, char *loc, NodeId i) {
	return langY_node3(fs,loc,NODE_GLOBAL,i,0,0);
}


NodeId langY_fieldnode(FileState *fs, char *loc, NodeId x, NodeId y) {
	return langY_node2(fs,loc,NODE_FIELD,x,y);
}


NodeId langY_designode(FileState *fs, char *loc, NodeId x, NodeId y) {
	return langY_node2(fs,loc,NODE_DESIG,x,y);
}


NodeId langY_rangeindexnode(FileState *fs, char *loc, NodeId x, NodeId y) {
	return langY_node3(fs,loc,NODE_RANGE_INDEX,x,y,0);
}


NodeId langY_metacallnode(FileState *fs, char *loc, NodeId x, NodeId y, NodeId *z) {
	return langY_node3(fs,loc,NODE_MCALL,x,y,z);
}


NodeId langY_callnode(FileState *fs, char *loc, NodeId x, NodeId *z) {
	return langY_node3(fs,loc,NODE_CALL,x,0,z);
}


NodeId langY_loadfilenode(FileState *fs, char *loc, NodeId x) {
	return langY_node2(fs,loc,NODE_LOADFILE,x,0);
}
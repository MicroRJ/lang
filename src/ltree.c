/*
** See Copyright Notice In lang.h
** (Y) ltree.c
** IR?...
*/


ltreeid langY_treexyz(FileState *fs, char *line, ltreetype k, ltreeid x, ltreeid y, ltreeid *z) {
	if (langA_varmin(fs->nodes) <= fs->nnodes) {
		langA_variadd(fs->nodes,1);
	}

	Tree *nd = fs->nodes + fs->nnodes;
	nd->level = fs->level;
	nd->line = line;
	nd->k = k;
	nd->x = x;
	nd->y = y;
	nd->z = z;
	return fs->nnodes ++;
}


ltreeid langY_treexy(FileState *fs, char *line, ltreetype k, ltreeid x, ltreeid y) {
	return langY_treexyz(fs,line,k,x,y,lnil);
}


ltreeid langY_treex(FileState *fs, char *line, ltreetype k, ltreeid x) {
	return langY_treexy(fs,line,k,x,NO_TREE);
}


ltreeid langY_tree(FileState *fs, char *line, int k) {
	return langY_treex(fs,line,k,NO_TREE);
}


ltreeid langY_treegroup(FileState *fs, char *line, ltreeid x) {
	return langY_treex(fs,line,Y_GROUP,x);
}


ltreeid langY_treelongint(FileState *fs, char *line, llongint i) {
	int v = langY_tree(fs,line,Y_INTEGER);
	fs->nodes[v].lit.i = i;
	return v;
}


ltreeid langY_treenumber(FileState *fs, char *line, lnumber n) {
	int v = langY_tree(fs,line,Y_NUMBER);
	fs->nodes[v].lit.n = n;
	return v;
}


ltreeid langY_treeString(FileState *fs, char *line, char *s) {
	int v = langY_tree(fs,line,Y_STRING);
	fs->nodes[v].lit.s = s;
	return v;
}


ltreeid langY_treetable(FileState *fs, char *line, ltreeid *z) {
	return langY_treexyz(fs,line,Y_TABLE,NO_TREE,NO_TREE,z);
}


ltreeid langY_treeclosure(FileState *fs, char *line, ltreeid x, ltreeid *z) {
	return langY_treexyz(fs,line,Y_FUNCTION,x,NO_TREE,z);
}


ltreeid langY_treenil(FileState *fs, char *line) {
	return langY_tree(fs,line,Y_NIL);
}


ltreeid langY_treecache(FileState *fs, char *line, ltreeid i) {
	return langY_treex(fs,line,Y_CACHED,i);
}


ltreeid langY_treelocal(FileState *fs, char *line, ltreeid i) {
	return langY_treex(fs,line,Y_LOCAL,i);
}


ltreeid langY_treeglobal(FileState *fs, char *line, ltreeid i) {
	return langY_treex(fs,line,Y_GLOBAL,i);
}


ltreeid langY_treefield(FileState *fs, char *line, ltreeid x, ltreeid y) {
	return langY_treexy(fs,line,Y_FIELD,x,y);
}


ltreeid langY_treeindex(FileState *fs, char *line, ltreeid x, ltreeid y) {
	return langY_treexy(fs,line,Y_INDEX,x,y);
}


ltreeid langY_treedesig(FileState *fs, char *line, ltreeid x, ltreeid y) {
	return langY_treexy(fs,line,Y_DESIG,x,y);
}


ltreeid langY_treerangedindex(FileState *fs, char *line, ltreeid x, ltreeid y) {
	return langY_treexy(fs,line,Y_RANGE_INDEX,x,y);
}


ltreeid langY_treemetacall(FileState *fs, char *line, ltreeid x, ltreeid y, ltreeid *z) {
	return langY_treexyz(fs,line,Y_MCALL,x,y,z);
}


ltreeid langY_treecall(FileState *fs, char *line, ltreeid x, ltreeid *z) {
	return langY_treexyz(fs,line,Y_CALL,x,NO_TREE,z);
}


ltreeid langY_treeloadfile(FileState *fs, char *line, ltreeid x) {
	return langY_treex(fs,line,Y_LOADFILE,x);
}


ltreeid langY_treebuiltincall(FileState *fs, char *line, ltokentype k, ltreeid *z) {
	return langY_treexyz(fs,line,Y_BUILTIN,k,NO_TREE,z);
}
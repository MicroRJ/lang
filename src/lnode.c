/*
** See Copyright Notice In elf.h
** (Y) lnode.c
** IR?...
*/


lnodeid elf_nodexyz(elf_FileState *fs, llineid line, lnodeop k, lnodety t, lnodeid x, lnodeid y, lnodeid *z) {
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


lnodeid elf_nodebinary(elf_FileState *fs, llineid line, lnodeop k, lnodety t, lnodeid x, lnodeid y) {
	return elf_nodexyz(fs,line,k,t,x,y,lnil);
}


lnodeid elf_nodeunary(elf_FileState *fs, llineid line, lnodeop k, lnodety t, lnodeid x) {
	return elf_nodebinary(fs,line,k,t,x,NO_NODE);
}


lnodeid elf_nodenullary(elf_FileState *fs, llineid line, lnodeop k, lnodety t) {
	return elf_nodeunary(fs,line,k,t,NO_NODE);
}


lnodeid elf_nodeload(elf_FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return elf_nodebinary(fs,line,NODE_LOAD,NT_ANY,x,y);
}


lnodeid elf_nodetypeguard(elf_FileState *fs, llineid line, lnodeid x, lnodety y) {
	lnodeid id = elf_nodebinary(fs,line,NODE_TYPEGUARD,y,x,y);
	/* todo: could we do this better! maybe we have
	a specific function that checks for these sort
	of nodes, like groups or typeguards,
	additionally, it can be an extra safety layer? */
	fs->nodes[id].r = fs->nodes[x].r;
	return id;
}


lnodeid elf_nodegroup(elf_FileState *fs, llineid line, lnodeid x) {
	lnodeid id = elf_nodeunary(fs,line,NODE_GROUP,fs->nodes[x].t,x);
	fs->nodes[id].r = fs->nodes[x].r;
	return id;
}


lnodeid elf_nodeint(elf_FileState *fs, llineid line, elf_int i) {
	lnodeid v = elf_nodenullary(fs,line,NODE_INTEGER,NT_INT);
	fs->nodes[v].lit.i = i;
	return v;
}


lnodeid elf_nodenum(elf_FileState *fs, llineid line, elf_num n) {
	lnodeid v = elf_nodenullary(fs,line,NODE_NUMBER,NT_NUM);
	fs->nodes[v].lit.n = n;
	return v;
}


lnodeid elf_nodestr(elf_FileState *fs, llineid line, char *s) {
	lnodeid v = elf_nodenullary(fs,line,NODE_STRING,NT_STR);
	fs->nodes[v].lit.s = s;
	return v;
}


lnodeid elf_nodetab(elf_FileState *fs, llineid line, lnodeid *z) {
	return elf_nodexyz(fs,line,NODE_TABLE,NT_TAB,NO_NODE,NO_NODE,z);
}


lnodeid elf_nodecls(elf_FileState *fs, llineid line, lnodeid x, lnodeid *z) {
	return elf_nodexyz(fs,line,NODE_CLOSURE,NT_FUN,x,NO_NODE,z);
}


lnodeid elf_nodenil(elf_FileState *fs, llineid line) {
	return elf_nodenullary(fs,line,NODE_NIL,NT_NIL);
}


lnodeid elf_nodecache(elf_FileState *fs, llineid line, llocalid x) {
	return elf_nodeunary(fs,line,NODE_CACHE,NT_ANY,x);
}


lnodeid elf_nodelocal(elf_FileState *fs, llineid line, llocalid x) {
	return elf_nodeunary(fs,line,NODE_LOCAL,NT_ANY,x);
}


lnodeid elf_nodeglobal(elf_FileState *fs, llineid line, lglobalid x) {
	return elf_nodeunary(fs,line,NODE_GLOBAL,NT_ANY,x);
}


lnodeid elf_nodefield(elf_FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return elf_nodebinary(fs,line,NODE_FIELD,NT_ANY,x,y);
}


lnodeid elf_nodeindex(elf_FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return elf_nodebinary(fs,line,NODE_INDEX,NT_ANY,x,y);
}


lnodeid elf_noderangedindex(elf_FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return elf_nodebinary(fs,line,NODE_RANGE_INDEX,NT_ANY,x,y);
}


lnodeid elf_nodemetafield(elf_FileState *fs, llineid line, lnodeid x, lnodeid y) {
	return elf_nodebinary(fs,line,NODE_METAFIELD,NT_ANY,x,y);
}


lnodeid elf_nodecall(elf_FileState *fs, llineid line, lnodeid x, lnodeid *z) {
	return elf_nodexyz(fs,line,NODE_CALL,NT_ANY,x,NO_NODE,z);
}


lnodeid elf_nodeloadfile(elf_FileState *fs, llineid line, lnodeid x) {
	return elf_nodeunary(fs,line,NODE_FILE,NT_ANY,x);
}


lnodeid elf_nodebuiltincall(elf_FileState *fs, llineid line, ltokentype k, lnodeid *z) {
	return elf_nodexyz(fs,line,NODE_BUILTIN,NT_ANY,k,NO_NODE,z);
}


elf_val elf_nodetolitval(elf_FileState *fs, lnodeid id);


void elf_nodelitapply(elf_FileState *fs, elf_Table *tab, lnodeid id) {
	lNode v = fs->nodes[id];
	switch (v.k) {
		case NODE_LOAD: {
			lNode x = fs->nodes[v.x];
			if (x.k == NODE_LOCAL) {
				LNOBRANCH;
			} else
			if ((x.k == NODE_FIELD) || (x.k == NODE_INDEX)) {
				elf_val key = elf_nodetolitval(fs,x.x);
				elf_val val = elf_nodetolitval(fs,v.y);
				elf_tabput(tab,key,val);
			} else LNOBRANCH;
		} break;
		default: LNOBRANCH;
	}
}


elf_val elf_nodetolitval(elf_FileState *fs, lnodeid id) {
	lNode nd = fs->nodes[id];
	switch (nd.k) {
		case NODE_INTEGER: {
			return lang_I(nd.lit.i);
		}
		case NODE_NUMBER: {
			return lang_N(nd.lit.n);
		}
		case NODE_TABLE: {
			elf_Table *tab = elf_newtab(fs->R);
			elf_arrfori(nd.z) {
				elf_nodelitapply(fs,tab,nd.z[i]);
			}
			return lang_H(tab);
		}
		default: LNOBRANCH;
	}
	return (elf_val){TAG_NIL};
}
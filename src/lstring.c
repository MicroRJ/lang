/*
** See Copyright Notice In lang.h
** lstring.c
** String lObject and String Tools
*/


lTable *langS_newclass(lRuntime *R) {
	lTable *tab = lang_pushnewtable(R);
	langH_mf(R,tab,"length",langS_length_);
	langH_mf(R,tab,"match",langS_match_);
	langH_mf(R,tab,"hash",langS_hash_);
	return tab;
}


lString *langS_new2(lRuntime *R, llongint length) {
	lString *obj = langGC_allocobj(R,OBJ_STRING,sizeof(lString)+length+1);
	obj->obj.metaclass = R->classofS;
	obj->length = length;
	obj->hash = -1;
	obj->c[length] = 0;
	return obj;
}


lString *langS_new(lRuntime *R, char const *junk) {
	int length = S_length(junk);
	lString *obj = langS_new2(R,length);
	langM_copy(obj->c,junk,length);
	obj->hash = langH_hashS((char*)junk);
	return obj;
}


lbool langS_eq(lString *x, lString *y) {
	if (x == y) return ltrue;
	/* assuming we use the same hash function */
	if (x->hash != y->hash) return lfalse;

	if (x->length != y->length) return lfalse;

	return S_eq(x->string,y->string);
}


int S_length(char const *s) {
	int n = 0;
	if (s != lnil) {
		while (*s ++ != 0) {
			n += 1;
		}
	}
	return n;
}


lbool S_eql(char const *x, char const *y, int n) {
	for (int i = 0; i < n; i += 1) {
		if (x[i] != y[i]) {
			return lfalse;
		}
	}
	return ltrue;
}


lbool S_eq(char const *x, char const *y) {
	int lx = S_length(x);
	int ly = S_length(y);
	return (lx == ly) && S_eql(x,y,lx);
}


char *S_ncopy(Alloc *allocator, int length, char const *string) {
	if (length <= 0) {
		length = S_length(string);
	}
	char *result = langM_alloc(allocator,length+1);
	langM_copy(result,string,length);
	result[length]=0;
	return result;
}


char *S_copy(Alloc *allocator, char const *string) {

	return S_ncopy(allocator,-1,string);
}


int langS_length_(lRuntime *c) {
	lang_pushlong(c,((lString*)c->f->obj)->length);
	return 1;
}


int langS_match_(lRuntime *c) {
	lString *s = (lString*) c->f->obj;
	lString *p = lang_loadS(c,0);
	lang_pushlong(c,S_match(p->string,s->string));
	return 1;
}


int langS_hash_(lRuntime *c) {
	lString *s = (lString*) c->f->obj;
	lang_pushlong(c,s->hash);
	return 1;
}


char *S_pfv(Alloc *cator, char const *format, va_list v) {
	int length = stbsp_vsnprintf(NULL,0,format,v);
	char *contents = langM_alloc(cator,length+1);
	stbsp_vsnprintf(contents,length+1,format,v);
	return contents;
}


char *S_tpfv(char const *format, va_list v) {
	return S_pfv(lTLOC,format,v);
}


char *S_tpf_(char const *format, ...) {
	va_list v;
	va_start(v,format);
	char *contents = S_tpfv(format,v);
	va_end(v);
	return contents;
}


/*
** Simple pattern matcher utility.
** Pattern, lString
*/
lbool S_matchsingle(char *p, char *s);


lbool S_match(char *p, char *s) {
	char *b = s;
	while (!S_matchsingle(p,s)) {

		while (*p != 0 && *p != '|') ++p;
		if (*p == 0) return lfalse;

		++ p, s = b;
	}
	return ltrue;
}


lbool S_matchsingle(char *p, char *s) {
	while (*p != 0 && *p != '|') {
		if (*p == '?') {
			/* matches any character except terminator. */
			if (*s == 0) return lfalse;
			++ p, ++ s;
		} else
		if (*p == '*') {
			/* unlikely the user will do this. */
			while (p[1] == '*') ++ p;

			/* got to end of string, do we still
			have a pattern after epsilon? If so
			then no match. */
			if (*s == 0) return p[1] == 0 || p[1] == '|';

			/* '*' operator causes matcher to split branches,
			we can either match the next pattern after '*' or
			delay the match by skipping this char and remaining
			in this pattern char. */
			if (S_matchsingle(p+1,s)) {
				return ltrue;
			}
			/* no match, move on to next char, remain in
			this branch and keep checking for matches. */
			++ s;
		} else
		/* otherwise, match literal fail if no match. */
		if (*p != *s) {
			return lfalse;
		} else {
			++ p, ++ s;
		}
	}
	/* did we match the whole string */
	return *s == 0;
}



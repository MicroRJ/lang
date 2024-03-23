/*
** See Copyright Notice In lang.h
** lstring.c
** String Object and String Tools
*/


String *langS_new(Runtime *fs, char const *contents) {
	static MetaFunc _m[] = {
		{"length",langS_length_},
		{"match",langS_match_},
	};

	int length = S_length(contents);

	String *string = langGC_allocobj(fs,OBJ_STRING,sizeof(String)+length);
	string->obj._m = _m;
	string->obj._n = _countof(_m);

	/* todo: allocate this along with the header */
	langM_copy(string->string,contents,length);
	string->hash = langH_hashS((char*)contents);
	return string;
}


Bool langS_eq(String *x, String *y) {
	if (x == y) return True;
	/* assuming we use the same hash function */
	if (x->hash != y->hash) return False;

	if (x->length != y->length) return False;

	return S_eq(x->string,y->string);
}


char *S_printfv(Alloc *cator, char const *format, va_list v) {
	int length = stbsp_vsnprintf(NULL,0,format,v);
	char *contents = langM_alloc(cator,length+1);
	stbsp_vsnprintf(contents,length+1,format,v);
	return contents;
}


char *S_tprintfv(char const *format, va_list v) {
	return S_printfv(elTEMP,format,v);
}


char *S_tprintf_(char const *format, ...) {
	va_list v;
	va_start(v,format);
	char *contents = S_tprintfv(format,v);
	va_end(v);
	return contents;
}


int S_length(char const *s) {
	int n = 0;
	if (s != Null) {
		while (*s ++ != 0) {
			n += 1;
		}
	}
	return n;
}


Bool S_eql(char const *x, char const *y, int n) {
	for (int i = 0; i < n; i += 1) {
		if (x[i] != y[i]) {
			return False;
		}
	}
	return True;
}


Bool S_eq(char const *x, char const *y) {
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


int langS_length_(Runtime *c) {
	langR_pushI(c,((String*)c->f->obj)->length);
	return 1;
}


int langS_match_(Runtime *c) {
	String *s = (String*) c->f->obj;
	String *p = langR_loadS(c,0);
	langR_pushI(c,S_match(p->string,s->string));
	return 1;
}


/*
** Simple pattern matcher utility.
*/
Bool S_match(char *p, char *s) {
	while (*p != 0) {
		if (*p == '?') {
			/* Matches any character except null
			terminator */
			if (*s == 0) return False;
			++ p, ++ s;
		} else
		if (*p == '*') {
			/* Unlikely the user will do this */
			while (p[1] == '*') ++ p;

			/* Got to end of string, do we still
			have a pattern after epsilon? If so
			then no match. */
			if (*s == 0) return p[1] == 0;

			/* Epsilon operator causes matcher branch
			to split, we can either match the sub-pattern
			or delay the match for later. */
			if (S_match(p+1,s)) {
				return True;
			}
			/* No match, move on to next string,
			remain in this pattern char and to
			keep matching on this branch. */
			++ s;
		} else
		/* Check literal */
		if (*p != *s) {
			return False;
		} else {
			++ p, ++ s;
		}
	}
	/* Have we consumed the whole string
	without no pattern matches */
	return *s == 0;
}
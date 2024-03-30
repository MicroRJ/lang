/*
**
** Copyright(c) 2023 - Dayan Rodriguez - Dyr
**
** -+- elang -+-
**
*/


lbool langX_islineend(char x) {
	return x == '\r' || x == '\n' || x == '\0';
}


/* finds line number and line loc from single source location */
void langX_getlocinfo(FileState *fs, char *loc, int *linenum, char **lineloc) {
	char *q = fs->contents;
	char *c = q;
	int n = 0;
	while (q < loc) {
		while (!langX_islineend(*q) && q < loc) q ++;
		if (*q == 0) break;
		if ((*q != '\n') || (c = ++ q, n ++, 1)) {
			if ((*q == '\r') && (c = ++ q, n ++, 1)) {
				if (*q == '\n') c = ++ q;
			} else q ++;
		}
	}

	if (linenum != 0) *linenum = n + 1;
	if (lineloc != 0) *lineloc = c;
}


/* diagnostics function for syntax errors */
void langX_error(FileState *fs, char *loc, char const *fmt, ...) {
	int linenum;
	char *lineloc;
	langX_getlocinfo(fs,loc,&linenum,&lineloc);

	/* skip initial blank characters for optimal gimmicky */
	while (*lineloc == '\t' || *lineloc == ' ') {
		lineloc += 1;
	}

	char u[0x40];

	int underline = loc - lineloc;
	if (underline >= sizeof(u)) {
		underline = sizeof(u)-1;
		lineloc = loc - underline;
	}

	int linelen = 0;
	for (;; ++ linelen) {
		if (lineloc[linelen] == '\0') break;
		if (lineloc[linelen] == '\r') break;
		if (lineloc[linelen] == '\n') break;
	}

	for (int i = 0; i < underline; ++ i) {
		u[i] = lineloc[i] == '\t' ? '\t' : '-';
	}
	u[underline]='^';

	if (fmt !=  0) {
		char b[0x1000];
		va_list v;
		va_start(v,fmt);
		stbsp_vsnprintf(b,sizeof(b),fmt,v);
		va_end(v);
		printf("%s [%i:%lli]: %s\n",fs->filename,linenum,1+loc-lineloc,b);
	}
	printf("| %.*s\n",linelen,lineloc);
	printf("| %.*s\n",underline+1,u);
}


lbool langX_isdigit(char x) {
	return x >= '0' && x <= '9';
}


lbool langX_islowercase(char x) {
	return x >= 'a' && x <= 'z';
}


lbool langX_isuppercase(char x) {
	return x >= 'A' && x <= 'Z';
}


lbool langX_isletter(char x) {
	return langX_isuppercase(x) || langX_islowercase(x);
}


lbool langX_isalphanum(char x) {
	return langX_isletter(x) || langX_isdigit(x) || (x) == '_';
}


ltokentype wordorkeyword(char const *name) {
	for (ltokentype i = FIRST_KEYWORD; i < LAST_KEYWORD; ++ i) {
		ltokenintel intel = langX_tokenintel[i];
		if (S_eq(intel.name,name)) {
			return i;
		}
	}
	return TK_WORD;
	/* todo: hashing! */
// #define KEYWORD_XITEM(NAME,STRING) \
// 	if (S_eq(name,STRING)) {        \
// 		return TK_##NAME;            \
// 	}
// 	KEYWORD_XLIST
// #undef KEYWORD_XITEM
	// return TK_WORD;
}


#define thischr() (file->thischar[0])
#define thenchr() (file->thischar[1])
#define movechr() (*(file->thischar++))
#define cmovchr(xx) ((thischr() == (xx)) ? movechr(), 1 : 0)


int langX_escapechr(FileState *file) {
	int tk = movechr();
	if (tk != '\\') return tk;

	tk = movechr();
	switch (tk) {
		case '\\': return '\\';
		case '\0': return '\0';
		case 't':  return '\t';
		case 'n':  return '\n';
		case 'r':  return '\r';
	}
	return tk;
}


ltoken langX_yield(FileState *file) {

	/* remove, not needed #todo */
	lglobaldecl char buffer[0x100];

	retry:

	char *line = file->thischar;
	ltoken tk = {TK_NONE,file->thischar};

	/* we could put all of the ascii codes in the switch
	statement, but that just makes it look incredibly silly  */
	switch (thischr()) {
		default: {
			if (langX_isletter(thischr()) || (thischr() == '_')) {
				int length = 0;
				do {
					buffer[length++] = movechr();
				} while (langX_isalphanum(thischr()));
				buffer[length] = 0;

				tk.type = wordorkeyword(buffer);
				if (tk.type == TK_WORD) {
					/* todo: string interner, or arena? */
					tk.s = S_ncopy(lHEAP,length,buffer);
				}
			}
		} break;
		case '0':case '1':case '2':case '3':case '4':
		case '5':case '6':case '7':case '8':case '9': {
			tk.type = TK_INTEGER;

			llong base = 10;
			if (thischr() == '0') {
				if (thenchr() == 'x') {
					movechr();
					movechr();
					base = 16;
				}
			}

			llong i = 0;
			if (base == 10) {
				do {
					i = i * 10 + (movechr() - '0');
				} while (isdigit(thischr()));
			} else {
				for (;;) {
					if (thischr() >= 'A' && thischr() <= 'Z') {
						i = i * base + 10 + (movechr() - 'A');
					} else
					if (thischr() >= 'a' && thischr() <= 'z') {
						i = i * base + 10 + (movechr() - 'a');
					} else
					if (thischr() >= '0' && thischr() <= '9') {
						i = i * base + (movechr() - '0');
					} else break;
				}
			}
			if (thischr() == '.') {
				// x{..}
				if (thenchr() != '.') {
					movechr();
					tk.type = TK_NUMBER;

					lnumber p = 1;
					lnumber n = 0;
					if (isdigit(thischr())) {
						do  {
							n = n * 10 + (movechr() - '0');
							p *= 10;
						} while (isdigit(thischr()));
					}
					tk.n = i + n / p;
					// lang_loginfo("[%lli] = num(%f)",tk.value,n);
					goto leave;
				}
			}
			tk.i = i;
			// lang_loginfo("[%lli] = int(%lli)",tk.value,integer);
		} break;
		case '\'': {
			movechr();
			tk.type = TK_LETTER;
			do {
				tk.i = movechr();
			} while(0);

			if (!cmovchr('\'')) {
				langX_error(file,tk.line,"invalid character constant, expected \"'\"");
			}
		} break;
		case '"': {
			movechr();

			int length = 0;
			while (thischr() != '"') {
				buffer[length ++] = langX_escapechr(file);
			}
			buffer[length] = 0;

			if (!cmovchr('"')) {
				langX_error(file,tk.line,"invalid string");
			}
			tk.type = TK_STRING;
			tk.s = S_ncopy(lHEAP,length,buffer);
			// tk.string = S_ncopy(lHEAP,length,buffer);
			// lang_loginfo("string %s",tk.string);
		} break;
		case '.': {
			movechr();
			tk.type = TK_DOT;
			if (cmovchr('.')) {
				tk.type = TK_DOT_DOT;
			} else
			if (isdigit(thischr())) {
				tk.type = TK_NUMBER;
				lnumber n = 0;
				lnumber p = 1;
				do {
					n = n * 10 + (movechr() - '0');
					p *= 10;
				} while (isdigit(thischr()));
				tk.n = n / p;
				// lang_loginfo("[%lli] = num(%f)",tk.value,n);
			}
		} break;
		case '\0': {
			tk.type = TK_NONE;
		} break;
		case ' ':
		case '\t': {
			movechr();
		} goto retry;
		case '\n': {
			movechr();
			file->linechar = file->thischar;
			file->linenumber += 1;
		} goto retry;
		case '\r': {
			movechr();
			cmovchr('\n');
			file->linechar = file->thischar;
			file->linenumber += 1;
		} goto retry;
		case ';': {
			movechr();
			while (!langX_islineend(thischr())) {
				movechr();
			}
			goto retry;
		} break;


		case '<': {
			movechr();
			tk.type = TK_LESS_THAN;
			if (cmovchr('=')) {
				tk.type = TK_LESS_THAN_EQUAL;
			} else
			if (cmovchr('<')) {
				tk.type = TK_LEFT_SHIFT;
			}
		} break;
		case '>': {
			movechr();
			tk.type = TK_GREATER_THAN;
			if (cmovchr('=')) {
				tk.type = TK_GREATER_THAN_EQUAL;
			} else
			if (cmovchr('>')) {
				tk.type = TK_RIGHT_SHIFT;
			}
		} break;

		#define TK_XCASE2(C0,T0,C1,T1) \
		case C0: {                     \
			movechr();                  \
			tk.type = T0;               \
			if (cmovchr(C1)) {          \
				tk.type = T1;            \
			}                           \
		} break;

		TK_XCASE2('|',TK_BIT_OR,'|',TK_LOG_OR);
		TK_XCASE2('&',TK_BIT_AND,'&',TK_LOG_AND);
		TK_XCASE2('!',TK_NEGATE,'=',TK_NOT_EQUALS);
		TK_XCASE2('=',TK_ASSIGN,'=',TK_EQUALS);
		TK_XCASE2('?',TK_QMARK,'=',TK_ASSIGN_QUESTION);

		#undef TK_XCASE2

		#define TK_XCASE1(C,T) \
		case C: {              \
			movechr();          \
			tk.type = T;        \
		} break

		TK_XCASE1('[',TK_SQUARE_LEFT);
		TK_XCASE1(']',TK_SQUARE_RIGHT);
		TK_XCASE1('(',TK_PAREN_LEFT);
		TK_XCASE1(')',TK_PAREN_RIGHT);
		TK_XCASE1('{',TK_CURLY_LEFT);
		TK_XCASE1('}',TK_CURLY_RIGHT);
		TK_XCASE1(',',TK_COMMA);
		TK_XCASE1('*',TK_MUL);
		TK_XCASE1('%',TK_MODULUS);
		TK_XCASE1(':',TK_COLON);
		TK_XCASE1('^',TK_BIT_XOR);
		TK_XCASE1('-',TK_SUB);
		TK_XCASE1('+',TK_ADD);

		#undef TK_XCASE1

		case '/': {
			movechr();
			tk.type = TK_DIV;
			if (cmovchr('*')) {
				for (;;) {
					/* handle lines */
					if (cmovchr('\n')) {
						file->linechar = file->thischar;
						file->linenumber += 1;
					} else
					if (cmovchr('\r')) {
						cmovchr('\n');
						file->linechar = file->thischar;
						file->linenumber += 1;
					} else
					if (cmovchr('*')) {
						if (cmovchr('/')) {
							/* end of comment */
							break;
						}
					} else {
						/* keep skipping! */
						movechr();
					}
				}
				goto retry;
			} else
			if (cmovchr('/')) {
				while (!langX_islineend(thischr())) {
					movechr();
				}
				goto retry;
			}
		} break;
	}

	leave: ;
	file->lasttk = file->tk;
	file->tk = file->thentk;
	file->thentk = tk;

	// langX_error(files,tk.line,"token %s",langX_tokenintel[tk.type].name);
	return file->lasttk;
}
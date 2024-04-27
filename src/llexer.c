/*
** See Copyright Notice In elf.h
** llexer.c
** Lexical Analyzer
*/



elf_bool elf_chriseol(char x) {
	return x == '\r' || x == '\n' || x == '\0';
}


/* finds line number and line loc from single source location */
void elfX_getlocinfo(char *q, char *loc, int *linenum, char **lineloc) {
	char *c = q;
	int n = 0;
	while (q < loc) {
		while (!elf_chriseol(*q) && q < loc) q ++;
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
void elfX_error2(char *filename, char *contents, char *loc, char const *fmt, ...) {
	int linenum;
	char *lineloc;
	elfX_getlocinfo(contents,loc,&linenum,&lineloc);

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
		printf("%s [%i:%lli]: %s\n",filename,linenum,(elf_int)(1+loc-lineloc),b);
	}
	printf("| %.*s\n",linelen,lineloc);
	printf("| %.*s\n",underline+1,u);
}


void elf_lineerror(elf_FileState *fs, char *loc, char const *fmt, ...) {
	int linenum;
	char *lineloc;
	elfX_getlocinfo(fs->contents,loc,&linenum,&lineloc);

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
		printf("%s [%i:%lli]: %s\n",fs->filename,linenum,(elf_int)(1+loc-lineloc),b);
	}
	printf("| %.*s\n",linelen,lineloc);
	printf("| %.*s\n",underline+1,u);
}


elf_bool elf_chrisdigit(char x) {
	return x >= '0' && x <= '9';
}


elf_bool elf_chrislowercase(char x) {
	return x >= 'a' && x <= 'z';
}


elf_bool elf_chrisuppercase(char x) {
	return x >= 'A' && x <= 'Z';
}


elf_bool elf_chrisletter(char x) {
	return elf_chrisuppercase(x) || elf_chrislowercase(x);
}


elf_bool elf_chrisalphanum(char x) {
	return elf_chrisletter(x) || elf_chrisdigit(x) || (x) == '_';
}


ltokentype wordorkeyword(char *name) {
	/* todo: */
	for (ltokentype i = FIRST_KEYWORD; i < LAST_KEYWORD; ++ i) {
		ltokenintel intel = elfX_tokenintel[i];
		if (S_eq(intel.name,name)) {
			return i;
		}
	}
	return TK_WORD;
}


#define elf_thischr() (file->thischar[0])
#define elf_thenchr() (file->thischar[1])
#define elf_movechr() (*(file->thischar++))
#define elf_movxchr(n) ((file->thischar += n))
#define elf_cmovchr(xx) ((elf_thischr() == (xx)) ? elf_movechr(), 1 : 0)


int elf_lexescchr(elf_FileState *file) {
	int tk = elf_movechr();
	if (tk != '\\') return tk;

	tk = elf_movechr();
	switch (tk) {
		case '\\': return '\\';
		case '\0': return '\0';
		case 't':  return '\t';
		case 'n':  return '\n';
		case 'r':  return '\r';
	}
	return tk;
}


/* not the fastest thing out there */
ltoken elf_lexone(elf_FileState *file) {

	/* remove, not needed #todo */
	elf_globaldecl char buffer[0x100];

	ltoken tk;

	retry:
	tk = (ltoken){TK_NONE,file->thischar};

	/* we could put all of the ascii codes in the switch
	statement, but that just makes it look incredibly silly  */
	switch (elf_thischr()) {
		default: {
			if (elf_chrisletter(elf_thischr()) || (elf_thischr() == '_')) {
				int length = 0;
				do {
					buffer[length++] = elf_movechr();
				} while (elf_chrisalphanum(elf_thischr()));
				buffer[length] = 0;

				tk.type = wordorkeyword(buffer);
				if (tk.type == TK_WORD) {
					if (S_eq(buffer,"__STATBREAK__")) {
						file->statbreak = ltrue;
						goto retry;
					}
					/* todo: string interner, or arena? */
					tk.s = S_ncopy(lHEAP,length,buffer);
				}
			}
		} break;
		case '0':case '1':case '2':case '3':case '4':
		case '5':case '6':case '7':case '8':case '9': {
			tk.type = TK_INTEGER;

			elf_int base = 10;
			if (elf_thischr() == '0') {
				if (elf_thenchr() == 'x') {
					elf_movxchr(2);
					base = 16;
				}
			}

			elf_int i = 0;
			if (base == 10) {
				do {
					i = i * 10 + (elf_movechr() - '0');
				} while (elf_chrisdigit(elf_thischr()));
			} else {
				for (;;) {
					if (elf_thischr() >= 'A' && elf_thischr() <= 'Z') {
						i = i * base + 10 + (elf_movechr() - 'A');
					} else
					if (elf_thischr() >= 'a' && elf_thischr() <= 'z') {
						i = i * base + 10 + (elf_movechr() - 'a');
					} else
					if (elf_thischr() >= '0' && elf_thischr() <= '9') {
						i = i * base + (elf_movechr() - '0');
					} else break;
				}
			}
			if (elf_thischr() == '.') {
				// x{..}
				if (elf_thenchr() != '.') {
					elf_movechr();
					tk.type = TK_NUMBER;

					elf_num p = 1;
					elf_num n = 0;
					if (elf_chrisdigit(elf_thischr())) {
						do  {
							n = n * 10 + (elf_movechr() - '0');
							p *= 10;
						} while (elf_chrisdigit(elf_thischr()));
					}
					tk.n = i + n / p;
					// elf_loginfo("[%lli] = num(%f)",tk.value,n);
					goto leave;
				}
			}
			tk.i = i;
			// elf_loginfo("[%lli] = int(%lli)",tk.value,integer);
		} break;
		case '\'': {
			elf_movechr();
			tk.type = TK_LETTER;
			do {
				tk.i = elf_movechr();
			} while(0);

			if (!elf_cmovchr('\'')) {
				elf_lineerror(file,tk.line,"invalid character constant, expected \"'\"");
			}
		} break;
		case '"': {
			elf_movechr();
			int length = 0;
			while (elf_thischr() != 0 && elf_thischr() != '"') {
				buffer[length ++] = elf_lexescchr(file);
			}
			buffer[length] = 0;
			if (!elf_cmovchr('"')) {
				elf_lineerror(file,tk.line,"invalid string");
			}
			tk.type = TK_STRING;
			tk.s = S_ncopy(lHEAP,length,buffer);
			// tk.string = S_ncopy(lHEAP,length,buffer);
			// elf_loginfo("string %s",tk.string);
		} break;
		case '.': {
			elf_movechr();
			tk.type = TK_DOT;
			if (elf_cmovchr('.')) {
				tk.type = TK_DOT_DOT;
			} else
			if (elf_chrisdigit(elf_thischr())) {
				tk.type = TK_NUMBER;
				elf_num n = 0;
				elf_num p = 1;
				do {
					n = n * 10 + (elf_movechr() - '0');
					p *= 10;
				} while (elf_chrisdigit(elf_thischr()));
				tk.n = n / p;
				// elf_loginfo("[%lli] = num(%f)",tk.value,n);
			}
		} break;
		case '#': {
			elf_movechr();
		} goto retry;
		case '\0': {
			tk.type = TK_NONE;
		} break;
		case ' ':
		case '\t': {
			elf_movechr();
		} goto retry;
		case '\n': {
			elf_movechr();
			file->linechar = file->thischar;
			file->linenumber += 1;
		} goto retry;
		case '\r': {
			elf_movechr();
			elf_cmovchr('\n');
			file->linechar = file->thischar;
			file->linenumber += 1;
		} goto retry;
		case ';': {
			elf_movechr();
			while (elf_thischr() != 0 && !elf_chriseol(elf_thischr())) {
				elf_movechr();
			}
			goto retry;
		} break;


		case '<': {
			elf_movechr();
			tk.type = TK_LESS_THAN;
			if (elf_cmovchr('=')) {
				tk.type = TK_LESS_THAN_EQUAL;
			} else
			if (elf_cmovchr('<')) {
				tk.type = TK_LEFT_SHIFT;
			}
		} break;
		case '>': {
			elf_movechr();
			tk.type = TK_GREATER_THAN;
			if (elf_cmovchr('=')) {
				tk.type = TK_GREATER_THAN_EQUAL;
			} else
			if (elf_cmovchr('>')) {
				tk.type = TK_RIGHT_SHIFT;
			}
		} break;

		#define TK_XCASE2(C0,T0,C1,T1) \
		case C0: {                     \
			elf_movechr();                  \
			tk.type = T0;               \
			if (elf_cmovchr(C1)) {          \
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
			elf_movechr();          \
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
			elf_movechr();
			tk.type = TK_DIV;
			if (elf_cmovchr('*')) {
				for (;;) {
					/* handle lines */
					if (elf_cmovchr('\n')) {
						file->linechar = file->thischar;
						file->linenumber += 1;
					} else
					if (elf_cmovchr('\r')) {
						elf_cmovchr('\n');
						file->linechar = file->thischar;
						file->linenumber += 1;
					} else
					if (elf_cmovchr('*')) {
						if (elf_cmovchr('/')) {
							/* end of comment */
							break;
						}
					} else {
						/* keep skipping! */
						elf_movechr();
					}
				}
				goto retry;
			} else
			if (elf_cmovchr('/')) {
				while (elf_thischr() != 0 && !elf_chriseol(elf_thischr())) {
					elf_movechr();
				}
				goto retry;
			}
		} break;
	}

	leave: ;

	/* for this language, we do context
	sensitive statement/expression
	termination, this is one passive way
	of doing so, we hint the parser that
	the token is succeed by a eol, and
	where the parser sees fit (contextually)
	it'll consult the flag to determine
	whether to end the expression or not. */
	while (elf_thischr() == ' '
	|| 	 elf_thischr() == '\t') elf_movechr();

	if ( elf_thischr() == '/'
	&& ( elf_thenchr() == '/' || elf_thenchr() == '*') ) {
		tk.eol = ltrue;
}
if ( elf_thischr() == ';'
|| ( elf_thischr() == '\n' || elf_thischr() == '\r') ) {
	tk.eol = ltrue;
}

file->lasttk = file->tk;
file->tk = file->thentk;
file->thentk = tk;

	// elf_lineerror(files,tk.line,"token %s",elfX_tokenintel[tk.type].name);
return file->lasttk;
}
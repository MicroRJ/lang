/*
** See Copyright Notice In elf.h
** llog.c
** Simple Logging Tools
*/


char *S_filename(char *s) {
	char *r;

	for(r = s; *s != 0; s += 1) {
		if (*s=='/' || *s=='\\') {
			r=s+1;
		}
	}
	return r;
}


char *elf_logtostr(int type) {
	switch (type) {
		case ELF_LOGDBUG: return "DEBUG";
		case ELF_LOGINFO: return "INFO";
		case ELF_LOGWARN: return "WARN";
		case ELF_LOGERROR: return "ERROR";
		case ELF_LOGFATAL: return "FATAL";
		default: return "OTHER";
	}
}


lapi void elf_log_(int type, ldebugloc source, const char *fmt, ...) {

	char b[0x1000];

	va_list xx;
	va_start(xx,fmt);

	stbsp_vsnprintf(b,sizeof(b),fmt,xx);

	va_end(xx);

	int line = source.lineNumber;
	char *file = (char*) source.fileName;
	char *func = (char*) source.func;

	printf("%s %s[%i] %s(): %s\n",elf_logtostr(type),S_filename(file),line,func,b);
}




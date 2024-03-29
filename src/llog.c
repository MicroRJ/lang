/*
** See Copyright Notice In lang.h
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


lapi void lang_log_(int type, ldebugloc source, const char *fmt, ...) {
	static const char *toString[] = {
		"FATAL", "ERROR", "WARNING", "INFO", "DEBUG"
	};
	char b[0x1000];

	va_list xx;
	va_start(xx,fmt);

	stbsp_vsnprintf(b,sizeof(b),fmt,xx);

	va_end(xx);

	int line = source.lineNumber;
	char *file = (char*) source.fileName;
	char *func = (char*) source.func;

	printf("%s %s[%i] %s(): %s\n",toString[type],S_filename(file),line,func,b);
}




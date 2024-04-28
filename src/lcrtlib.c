/*
** See Copyright Notice In elf.h
** lcrtlib.c
** CRT Library
*/


#include <time.h>

#if defined(_WIN32)
#include <conio.h>
#include <process.h>
#include <io.h>
#include <direct.h>
#endif


int crtlib_floor(elf_Runtime *R) {
	elf_putnum(R,floor(elf_getnum(R,0)));
	return 1;
}


int crtlib_sqrt(elf_Runtime *R) {
	elf_putnum(R,sqrt(elf_getnum(R,0)));
	return 1;
}


int crtlib_sin(elf_Runtime *R) {
	elf_putnum(R,sin(elf_getnum(R,0)));
	return 1;
}


int crtlib_cos(elf_Runtime *R) {
	elf_putnum(R,cos(elf_getnum(R,0)));
	return 1;
}


int crtlib_tan(elf_Runtime *R) {
	elf_putnum(R,tan(elf_getnum(R,0)));
	return 1;
}


int crtlib_atan2(elf_Runtime *R) {
	elf_putnum(R,atan2(elf_getnum(R,0),elf_getnum(R,1)));
	return 1;
}


int crtlib_abort(elf_Runtime *rt) {
	if(1) abort();
	return 0;
}


int crtlib_exit(elf_Runtime *rt) {
	if(1) exit(elf_getint(rt,0));
	return 0;
}


int crtlib__chdir(elf_Runtime *rt) {
	elf_String *name = elf_getstr(rt,0);
#if defined(PLATFORM_WEB)
	elf_putint(rt,chdir(name->c));
#else
	elf_putint(rt,_chdir(name->c));
#endif
	return 1;
}


int crtlib_fopen(elf_Runtime *c) {
	elf_String *name = elf_getstr(c,0);
	elf_String *flags = elf_getstr(c,1);
	FILE *file = lnil;
#if defined(PLATFORM_WEB)
	file = fopen(name->c,flags->c);
#else
	fopen_s(&file,name->c,flags->c);
#endif
	elf_putsys(c,(elf_Handle) file);
	return 1;
}


int crtlib_fclose(elf_Runtime *c) {
	elf_Handle file = elf_getsys(c,0);
	fclose(file);
	return 0;
}


int crtlib_fsize(elf_Runtime *c) {
	elf_Handle file = elf_getsys(c,0);
	fseek(file,0,SEEK_END);
	elf_putint(c,ftell(file));
	return 1;
}


#if defined(_MSC_VER)

int crtlib__getch(elf_Runtime *rt) {
	elf_putint(rt,_getch());
	return 1;
}


int crtlib__getpid(elf_Runtime *rt) {
	elf_putint(rt,_getpid());
	return 1;
}


int crtlib_time(elf_Runtime *rt) {
	elf_putint(rt,time(0));
	return 1;
}


int crtlib_clock(elf_Runtime *rt) {
	elf_putint(rt,clock());
	return 1;
}


int crtlib__strdate(elf_Runtime *rt) {
	char buf[128];
	_strdate_s(buf,sizeof(buf));
	elf_putnewstr(rt,buf);
	return 1;
}


int crtlib__strtime(elf_Runtime *rt) {
	char buf[128];
	_strtime_s(buf,sizeof(buf));
	elf_putnewstr(rt,buf);
	return 1;
}


int crtlib__unlink(elf_Runtime *rt) {
	elf_String *name = elf_getstr(rt,0);
	elf_putint(rt,_unlink(name->c));
	return 1;
}


int crtlib__unlock_file(elf_Runtime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	_unlock_file(file);
	return 0;
}


int crtlib__write(elf_Runtime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	elf_String *buf = elf_getstr(rt,1);
	elf_putint(rt,_write((elf_int)file,buf->c,buf->length));
	return 1;
}


int crtlib__close(elf_Runtime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	elf_putint(rt,_close((int)(elf_int)file));
	return 1;
}


int crtlib__commit(elf_Runtime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	elf_putint(rt,_commit((int)(elf_int)file));
	return 1;
}


int crtlib__chdrive(elf_Runtime *rt) {
	elf_int letter = elf_getint(rt,0);
	elf_putint(rt,_chdrive(letter));
	return 1;
}


int crtlib__chmode(elf_Runtime *rt) {
	elf_String *name = elf_getstr(rt,0);
	elf_int mode = elf_getint(rt,1);
	elf_putint(rt,_chmod(name->c,mode));
	return 1;
}


int crtlib__execl(elf_Runtime *rt) {
	elf_String *cl = elf_getstr(rt,0);
	elf_putint(rt,_execl(cl->c,0,0));
	return 1;
}


int crtlib_system(elf_Runtime *rt) {
	elf_String *cl = elf_getstr(rt,0);
	elf_putint(rt,system(cl->c));
	return 1;
}
#else

#define DEFSTUB(NAME) \
int NAME(elf_Runtime *R) {\
	elf_logerror(XSTRINGIFY(NAME)"(): not implemented for this platform");\
	return 0;\
}

DEFSTUB(crtlib__getch)
DEFSTUB(crtlib__getpid)
DEFSTUB(crtlib_time)
DEFSTUB(crtlib_clock)
DEFSTUB(crtlib__strdate)
DEFSTUB(crtlib__strtime)
DEFSTUB(crtlib__unlink)
DEFSTUB(crtlib__unlock_file)
DEFSTUB(crtlib__write)
DEFSTUB(crtlib__close)
DEFSTUB(crtlib__commit)
DEFSTUB(crtlib__chdrive)
DEFSTUB(crtlib__chmode)
DEFSTUB(crtlib__execl)
DEFSTUB(crtlib_system)
#endif

elf_api void crtlib_load(elf_Runtime *rt) {
	elf_Module *md = rt->md;

	lang_addglobal(md,elf_putnewstr(rt,"floor"),lang_C(crtlib_floor));
	lang_addglobal(md,elf_putnewstr(rt,"sqrt"),lang_C(crtlib_sqrt));
	lang_addglobal(md,elf_putnewstr(rt,"sin"),lang_C(crtlib_sin));
	lang_addglobal(md,elf_putnewstr(rt,"cos"),lang_C(crtlib_cos));
	lang_addglobal(md,elf_putnewstr(rt,"tan"),lang_C(crtlib_tan));
	lang_addglobal(md,elf_putnewstr(rt,"atan2"),lang_C(crtlib_atan2));

	lang_addglobal(md,elf_putnewstr(rt,"stderr"),lang_H(stderr));
	lang_addglobal(md,elf_putnewstr(rt,"stdout"),lang_H(stdout));
	lang_addglobal(md,elf_putnewstr(rt,"stdin"),lang_H(stdin));


	lang_addglobal(md,elf_putnewstr(rt,"abort"),lang_C(crtlib_abort));
	lang_addglobal(md,elf_putnewstr(rt,"_execl"),lang_C(crtlib__execl));
	lang_addglobal(md,elf_putnewstr(rt,"system"),lang_C(crtlib_system));
	lang_addglobal(md,elf_putnewstr(rt,"_getch"),lang_C(crtlib__getch));
	lang_addglobal(md,elf_putnewstr(rt,"time"),lang_C(crtlib_time));
	lang_addglobal(md,elf_putnewstr(rt,"_getpid"),lang_C(crtlib__getpid));
	lang_addglobal(md,elf_putnewstr(rt,"_strdate"),lang_C(crtlib__strdate));
	lang_addglobal(md,elf_putnewstr(rt,"_strtime"),lang_C(crtlib__strtime));

	lang_addglobal(md,elf_putnewstr(rt,"fopen"),lang_C(crtlib_fopen));
	lang_addglobal(md,elf_putnewstr(rt,"fclose"),lang_C(crtlib_fclose));
	lang_addglobal(md,elf_putnewstr(rt,"fsize"),lang_C(crtlib_fsize));
	lang_addglobal(md,elf_putnewstr(rt,"_unlink"),lang_C(crtlib__unlink));
	lang_addglobal(md,elf_putnewstr(rt,"_unlock_file"),lang_C(crtlib__unlock_file));
	lang_addglobal(md,elf_putnewstr(rt,"_write"),lang_C(crtlib__write));
	lang_addglobal(md,elf_putnewstr(rt,"_commit"),lang_C(crtlib__commit));
	lang_addglobal(md,elf_putnewstr(rt,"_close"),lang_C(crtlib__close));
	lang_addglobal(md,elf_putnewstr(rt,"_chdir"),lang_C(crtlib__chdir));
	lang_addglobal(md,elf_putnewstr(rt,"_chdrive"),lang_C(crtlib__chdrive));
	lang_addglobal(md,elf_putnewstr(rt,"clock"),lang_C(crtlib_clock));
}
/*
** See Copyright Notice In elf.h
** lcrtlib.c
** CRT Library
*/


#include <time.h>
#include <conio.h>
#include <process.h>
#include <io.h>
#include <direct.h>



int crtlib_floor(lRuntime *R) {
	elf_putnum(R,floor(elf_getnum(R,0)));
	return 1;
}


int crtlib_sqrt(lRuntime *R) {
	elf_putnum(R,sqrt(elf_getnum(R,0)));
	return 1;
}


int crtlib_sin(lRuntime *R) {
	elf_putnum(R,sin(elf_getnum(R,0)));
	return 1;
}


int crtlib_cos(lRuntime *R) {
	elf_putnum(R,cos(elf_getnum(R,0)));
	return 1;
}


int crtlib_tan(lRuntime *R) {
	elf_putnum(R,tan(elf_getnum(R,0)));
	return 1;
}


int crtlib_atan2(lRuntime *R) {
	elf_putnum(R,atan2(elf_getnum(R,0),elf_getnum(R,1)));
	return 1;
}


int crtlib_abort(lRuntime *rt) {
	if(1) abort();
	return 0;
}


int crtlib_exit(lRuntime *rt) {
	if(1) exit(elf_getint(rt,0));
	return 0;
}


int crtlib__getch(lRuntime *rt) {
	elf_putint(rt,_getch());
	return 1;
}


int crtlib__getpid(lRuntime *rt) {
	elf_putint(rt,_getpid());
	return 1;
}


int crtlib_time(lRuntime *rt) {
	elf_putint(rt,time(0));
	return 1;
}


int crtlib_clock(lRuntime *rt) {
	elf_putint(rt,clock());
	return 1;
}


int crtlib__strdate(lRuntime *rt) {
	char buf[128];
	_strdate_s(buf,sizeof(buf));
	elf_pushnewstr(rt,buf);
	return 1;
}


int crtlib__strtime(lRuntime *rt) {
	char buf[128];
	_strtime_s(buf,sizeof(buf));
	elf_pushnewstr(rt,buf);
	return 1;
}


int crtlib_fopen(lRuntime *c) {
	elf_String *name = elf_getstr(c,0);
	elf_String *flags = elf_getstr(c,1);
	FILE *file = lnil;
	fopen_s(&file,name->c,flags->c);
	elf_putsys(c,(elf_Handle) file);
	return 1;
}


int crtlib_fclose(lRuntime *c) {
	elf_Handle file = elf_getsys(c,0);
	fclose(file);
	return 0;
}


int crtlib_fsize(lRuntime *c) {
	elf_Handle file = elf_getsys(c,0);
	fseek(file,0,SEEK_END);
	elf_putint(c,ftell(file));
	return 1;
}


int crtlib__unlink(lRuntime *rt) {
	elf_String *name = elf_getstr(rt,0);
	elf_putint(rt,_unlink(name->c));
	return 1;
}


int crtlib__unlock_file(lRuntime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	_unlock_file(file);
	return 0;
}


int crtlib__write(lRuntime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	elf_String *buf = elf_getstr(rt,1);
	elf_putint(rt,_write((elf_int)file,buf->c,buf->length));
	return 1;
}


int crtlib__close(lRuntime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	elf_putint(rt,_close((int)(elf_int)file));
	return 1;
}


int crtlib__commit(lRuntime *rt) {
	elf_Handle file = elf_getsys(rt,0);
	elf_putint(rt,_commit((int)(elf_int)file));
	return 1;
}


int crtlib__chdir(lRuntime *rt) {
	elf_String *name = elf_getstr(rt,0);
	elf_putint(rt,_chdir(name->c));
	return 1;
}


int crtlib__chdrive(lRuntime *rt) {
	elf_int letter = elf_getint(rt,0);
	elf_putint(rt,_chdrive(letter));
	return 1;
}


int crtlib__chmode(lRuntime *rt) {
	elf_String *name = elf_getstr(rt,0);
	elf_int mode = elf_getint(rt,1);
	elf_putint(rt,_chmod(name->c,mode));
	return 1;
}


int crtlib__execl(lRuntime *rt) {
	elf_String *cl = elf_getstr(rt,0);
	elf_putint(rt,_execl(cl->c,0,0));
	return 1;
}


int crtlib_system(lRuntime *rt) {
	elf_String *cl = elf_getstr(rt,0);
	elf_putint(rt,system(cl->c));
	return 1;
}


lapi void crtlib_load(lRuntime *rt) {
	elf_Module *md = rt->md;

	lang_addglobal(md,elf_pushnewstr(rt,"floor"),lang_C(crtlib_floor));
	lang_addglobal(md,elf_pushnewstr(rt,"sqrt"),lang_C(crtlib_sqrt));
	lang_addglobal(md,elf_pushnewstr(rt,"sin"),lang_C(crtlib_sin));
	lang_addglobal(md,elf_pushnewstr(rt,"cos"),lang_C(crtlib_cos));
	lang_addglobal(md,elf_pushnewstr(rt,"tan"),lang_C(crtlib_tan));
	lang_addglobal(md,elf_pushnewstr(rt,"atan2"),lang_C(crtlib_atan2));

	lang_addglobal(md,elf_pushnewstr(rt,"stderr"),lang_H(stderr));
	lang_addglobal(md,elf_pushnewstr(rt,"stdout"),lang_H(stdout));
	lang_addglobal(md,elf_pushnewstr(rt,"stdin"),lang_H(stdin));


	lang_addglobal(md,elf_pushnewstr(rt,"abort"),lang_C(crtlib_abort));
	lang_addglobal(md,elf_pushnewstr(rt,"_execl"),lang_C(crtlib__execl));
	lang_addglobal(md,elf_pushnewstr(rt,"system"),lang_C(crtlib_system));
	lang_addglobal(md,elf_pushnewstr(rt,"_getch"),lang_C(crtlib__getch));
	lang_addglobal(md,elf_pushnewstr(rt,"time"),lang_C(crtlib_time));
	lang_addglobal(md,elf_pushnewstr(rt,"_getpid"),lang_C(crtlib__getpid));
	lang_addglobal(md,elf_pushnewstr(rt,"_strdate"),lang_C(crtlib__strdate));
	lang_addglobal(md,elf_pushnewstr(rt,"_strtime"),lang_C(crtlib__strtime));

	lang_addglobal(md,elf_pushnewstr(rt,"fopen"),lang_C(crtlib_fopen));
	lang_addglobal(md,elf_pushnewstr(rt,"fclose"),lang_C(crtlib_fclose));
	lang_addglobal(md,elf_pushnewstr(rt,"fsize"),lang_C(crtlib_fsize));
	lang_addglobal(md,elf_pushnewstr(rt,"_unlink"),lang_C(crtlib__unlink));
	lang_addglobal(md,elf_pushnewstr(rt,"_unlock_file"),lang_C(crtlib__unlock_file));
	lang_addglobal(md,elf_pushnewstr(rt,"_write"),lang_C(crtlib__write));
	lang_addglobal(md,elf_pushnewstr(rt,"_commit"),lang_C(crtlib__commit));
	lang_addglobal(md,elf_pushnewstr(rt,"_close"),lang_C(crtlib__close));
	lang_addglobal(md,elf_pushnewstr(rt,"_chdir"),lang_C(crtlib__chdir));
	lang_addglobal(md,elf_pushnewstr(rt,"_chdrive"),lang_C(crtlib__chdrive));
	lang_addglobal(md,elf_pushnewstr(rt,"clock"),lang_C(crtlib_clock));
}
/*
** See Copyright Notice In lang.h
** lcrtlib.c
** CRT Library
*/


#include <time.h>
#include <conio.h>
#include <process.h>
#include <io.h>
#include <direct.h>


int crtlib_abort(lRuntime *rt) {
	if(1) abort();
	return 0;
}


int crtlib_exit(lRuntime *rt) {
	if(1) exit(lang_loadlong(rt,0));
	return 0;
}


int crtlib__getch(lRuntime *rt) {
	lang_pushlong(rt,_getch());
	return 1;
}


int crtlib__getpid(lRuntime *rt) {
	lang_pushlong(rt,_getpid());
	return 1;
}


int crtlib_time(lRuntime *rt) {
	lang_pushlong(rt,time(0));
	return 1;
}


int crtlib_clock(lRuntime *rt) {
	lang_pushlong(rt,clock());
	return 1;
}


int crtlib__strdate(lRuntime *rt) {
	char buf[128];
	_strdate_s(buf,sizeof(buf));
	lang_pushnewS(rt,buf);
	return 1;
}


int crtlib__strtime(lRuntime *rt) {
	char buf[128];
	_strtime_s(buf,sizeof(buf));
	lang_pushnewS(rt,buf);
	return 1;
}


int crtlib__unlink(lRuntime *rt) {
	lString *name = lang_loadS(rt,0);
	lang_pushlong(rt,_unlink(name->c));
	return 1;
}


int crtlib__unlock_file(lRuntime *rt) {
	Handle file = lang_loadhandle(rt,0);
	_unlock_file(file);
	return 0;
}


int crtlib__write(lRuntime *rt) {
	Handle file = lang_loadhandle(rt,0);
	lString *buf = lang_loadS(rt,1);
	lang_pushlong(rt,_write((llongint)file,buf->c,buf->length));
	return 1;
}


int crtlib__close(lRuntime *rt) {
	Handle file = lang_loadhandle(rt,0);
	lang_pushlong(rt,_close((int)(llongint)file));
	return 1;
}


int crtlib__commit(lRuntime *rt) {
	Handle file = lang_loadhandle(rt,0);
	lang_pushlong(rt,_commit((int)(llongint)file));
	return 1;
}


int crtlib__chdir(lRuntime *rt) {
	lString *name = lang_loadS(rt,0);
	lang_pushlong(rt,_chdir(name->c));
	return 1;
}


int crtlib__chdrive(lRuntime *rt) {
	llongint letter = lang_loadlong(rt,0);
	lang_pushlong(rt,_chdrive(letter));
	return 1;
}


int crtlib__chmode(lRuntime *rt) {
	lString *name = lang_loadS(rt,0);
	llongint mode = lang_loadlong(rt,1);
	lang_pushlong(rt,_chmod(name->c,mode));
	return 1;
}


int crtlib__execl(lRuntime *rt) {
	lString *cl = lang_loadS(rt,0);
	lang_pushlong(rt,_execl(cl->c,0,0));
	return 1;
}


int crtlib_system(lRuntime *rt) {
	lString *cl = lang_loadS(rt,0);
	lang_pushlong(rt,system(cl->c));
	return 1;
}


lapi void crtlib_load(lRuntime *rt) {
	lModule *md = rt->md;

	lang_addglobal(md,lang_pushnewS(rt,"stderr"),lang_H(stderr));
	lang_addglobal(md,lang_pushnewS(rt,"stdout"),lang_H(stdout));
	lang_addglobal(md,lang_pushnewS(rt,"stdin"),lang_H(stdin));


	lang_addglobal(md,lang_pushnewS(rt,"abort"),lang_C(crtlib_abort));
	lang_addglobal(md,lang_pushnewS(rt,"_execl"),lang_C(crtlib__execl));
	lang_addglobal(md,lang_pushnewS(rt,"system"),lang_C(crtlib_system));
	lang_addglobal(md,lang_pushnewS(rt,"_getch"),lang_C(crtlib__getch));
	lang_addglobal(md,lang_pushnewS(rt,"time"),lang_C(crtlib_time));
	lang_addglobal(md,lang_pushnewS(rt,"_getpid"),lang_C(crtlib__getpid));
	lang_addglobal(md,lang_pushnewS(rt,"_strdate"),lang_C(crtlib__strdate));
	lang_addglobal(md,lang_pushnewS(rt,"_strtime"),lang_C(crtlib__strtime));
	lang_addglobal(md,lang_pushnewS(rt,"_unlink"),lang_C(crtlib__unlink));
	lang_addglobal(md,lang_pushnewS(rt,"_unlock_file"),lang_C(crtlib__unlock_file));
	lang_addglobal(md,lang_pushnewS(rt,"_write"),lang_C(crtlib__write));
	lang_addglobal(md,lang_pushnewS(rt,"_commit"),lang_C(crtlib__commit));
	lang_addglobal(md,lang_pushnewS(rt,"_close"),lang_C(crtlib__close));
	lang_addglobal(md,lang_pushnewS(rt,"_chdir"),lang_C(crtlib__chdir));
	lang_addglobal(md,lang_pushnewS(rt,"_chdrive"),lang_C(crtlib__chdrive));
	lang_addglobal(md,lang_pushnewS(rt,"clock"),lang_C(crtlib_clock));
}
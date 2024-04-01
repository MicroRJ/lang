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


int crtlib_abort(Runtime *rt) {
	if(1) abort();
	return 0;
}


int crtlib_exit(Runtime *rt) {
	if(1) exit(lang_loadlong(rt,0));
	return 0;
}


int crtlib__getch(Runtime *rt) {
	lang_pushlong(rt,_getch());
	return 1;
}


int crtlib__getpid(Runtime *rt) {
	lang_pushlong(rt,_getpid());
	return 1;
}


int crtlib_time(Runtime *rt) {
	lang_pushlong(rt,time(0));
	return 1;
}


int crtlib_clock(Runtime *rt) {
	lang_pushlong(rt,clock());
	return 1;
}


int crtlib__strdate(Runtime *rt) {
	char buf[128];
	_strdate_s(buf,sizeof(buf));
	lang_pushnewS(rt,buf);
	return 1;
}


int crtlib__strtime(Runtime *rt) {
	char buf[128];
	_strtime_s(buf,sizeof(buf));
	lang_pushnewS(rt,buf);
	return 1;
}



int crtlib__unlink(Runtime *rt) {
	lString *name = lang_loadS(rt,0);
	lang_pushlong(rt,_unlink(name->c));
	return 1;
}


int crtlib__unlock_file(Runtime *rt) {
	Handle file = lang_loadhandle(rt,0);
	_unlock_file(file);
	return 0;
}


int crtlib__write(Runtime *rt) {
	Handle file = lang_loadhandle(rt,0);
	lString *buf = lang_loadS(rt,1);
	lang_pushlong(rt,_write((llong)file,buf->c,buf->length));
	return 1;
}


int crtlib__close(Runtime *rt) {
	Handle file = lang_loadhandle(rt,0);
	lang_pushlong(rt,_close((int)(llong)file));
	return 1;
}


int crtlib__commit(Runtime *rt) {
	Handle file = lang_loadhandle(rt,0);
	lang_pushlong(rt,_commit((int)(llong)file));
	return 1;
}


int crtlib__chdir(Runtime *rt) {
	lString *name = lang_loadS(rt,0);
	lang_pushlong(rt,_chdir(name->c));
	return 1;
}


int crtlib__chdrive(Runtime *rt) {
	llong letter = lang_loadlong(rt,0);
	lang_pushlong(rt,_chdrive(letter));
	return 1;
}


int crtlib__chmode(Runtime *rt) {
	lString *name = lang_loadS(rt,0);
	llong mode = lang_loadlong(rt,1);
	lang_pushlong(rt,_chmod(name->c,mode));
	return 1;
}


int crtlib__execl(Runtime *rt) {
	lString *cl = lang_loadS(rt,0);
	lang_pushlong(rt,_execl(cl->c,0,0));
	return 1;
}


int crtlib_system(Runtime *rt) {
	lString *cl = lang_loadS(rt,0);
	lang_pushlong(rt,system(cl->c));
	return 1;
}


lapi void crtlib_load(Runtime *rt) {
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
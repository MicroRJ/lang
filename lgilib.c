/*
** See Copyright Notice In lang.h
** lgilib.c
** LGI Library Bindings For Lang.
*/

#include <lang.h>
#include <lgi/lgi.h>


lgi_API void lgi_initWindowed(int windowWidth, int windowHeight, char const *windowTitle);
lgi_API int lgi_tick();
lgi_API void lgi_drawQuad(lgi_Color color, float x, float y, float w, float h);


llibfn int lgilib_time(lRuntime *rt) {
	lnumber time = lgi.Time.totalSeconds;
	lang_pushnum(rt,time);
	return 1;
}


llibfn int lgilib_deltatime(lRuntime *rt) {
	lnumber time = lgi.Time.deltaSeconds;
	lang_pushnum(rt,time);
	return 1;
}


llibfn int lgilib_initWindowed(lRuntime *rt) {
	int windowWidth = (int) lang_loadlong(rt,0);
	int windowHeight = (int) lang_loadlong(rt,1);
	lString *name = lang_loadS(rt,2);
	lgi_initWindowed(windowWidth,windowHeight,name->c);
	return 0;
}


llibfn int lgilib_tick(lRuntime *rt) {
	int r = lgi_tick();
	lang_pushlong(rt,r);
	return 1;
}


llibfn int lgilib_drawQuad(lRuntime *rt) {
	LASSERT(lang_leftover(rt) == 8);
	float r = (float) lang_loadnum(rt,0);
	float g = (float) lang_loadnum(rt,1);
	float b = (float) lang_loadnum(rt,2);
	float a = (float) lang_loadnum(rt,3);

	float x = (float) lang_loadnum(rt,4);
	float y = (float) lang_loadnum(rt,5);
	float w = (float) lang_loadnum(rt,6);
	float h = (float) lang_loadnum(rt,7);
	lgi_drawQuad((lgi_Color){r,g,b,a},x,y,w,h);
	return 0;
}


llibfn int lgilib_clearBackground(lRuntime *rt) {
	(void) rt;
	lgi_clearBackground(lgi_BLACK);
	return 0;
}


llibfn int lgilib_testKey(lRuntime *rt) {
	(void) rt;
	int key = (int) lang_loadlong(rt,0);
	lang_pushlong(rt,lgi_testKey(key));
	return 1;
}



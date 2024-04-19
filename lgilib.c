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
	lString *name = lang_getstr(rt,2);
	lgi_initWindowed(windowWidth,windowHeight,name->c);
	return 0;
}


llibfn int lgilib_loadtexture(lRuntime *R) {
	lString *filename = lang_getstr(R,0);
	lgi_Texture *texture = lgi_loadTexture(filename->c);
	lang_pushsysobj(R,(lsysobj)texture);
	return 1;
}


llibfn int lgilib_gettexturewidth(lRuntime *R) {
	lgi_Texture *texture = (lgi_Texture *) lang_getsysobj(R,0);
	lang_pushlong(R,texture->size_x);
	return 1;
}


llibfn int lgilib_gettextureheight(lRuntime *R) {
	lgi_Texture *texture = (lgi_Texture *) lang_getsysobj(R,0);
	lang_pushlong(R,texture->size_y);
	return 1;
}


llibfn int lgilib_bindtexture(lRuntime *R) {
	// lgi_bindProgram(lgi.defaultProgram);
	lgi_Texture *tex = (lgi_Texture *) lang_getsysobj(R,0);
	lgi_bindTexture(0,tex,lgi_True);
	return 0;
}


llibfn int lgilib_beginvertices(lRuntime *R) {
	int ni = (int) lang_loadlong(R,0);
	int nv = (int) lang_loadlong(R,1);
	lgi_beginVertexArray(ni,nv);
	return 0;
}


llibfn int lgilib_closevertices(lRuntime *R) {
	(void) R;
	lgi_endVertexArray();
	return 0;
}


llibfn int lgilib_addnidx(lRuntime *R) {
	int num = R->call->x;
	lgi_ASSERT(lgi.State.index_tally + num < lgi_DEFAULT_INDEX_BUFFER_LENGTH);
	for(int i=0; i<num; i+=1) {
		lgi_Index index = (lgi_Index) ltolong(R->call->locals[i]);
		lgi.State.index_array[
		lgi.State.index_tally] = lgi.State.index_offset + index;
		lgi.State.index_tally += 1;
	}
	return 0;
}


llibfn int lgilib_attrrgba(lRuntime *R) {
	lgi.State.attr.r = (float) lang_getnum(R,0);
	lgi.State.attr.g = (float) lang_getnum(R,1);
	lgi.State.attr.b = (float) lang_getnum(R,2);
	lgi.State.attr.a = (float) lang_getnum(R,3);
	return 0;
}


llibfn int lgilib_attrxyzw(lRuntime *R) {
	lgi.State.attr.x = (float) lang_getnum(R,0);
	lgi.State.attr.y = (float) lang_getnum(R,1);
	lgi.State.attr.z = (float) lang_getnum(R,2);
	lgi.State.attr.w = (float) lang_getnum(R,3);
	return 0;
}


llibfn int lgilib_attruv(lRuntime *R) {
	lgi.State.attr.u = (float) lang_getnum(R,0);
	lgi.State.attr.v = (float) lang_getnum(R,1);
	return 0;
}


llibfn int lgilib_attrxyuv(lRuntime *R) {
	lgi.State.attr.x = (float) lang_getnum(R,0);
	lgi.State.attr.y = (float) lang_getnum(R,1);
	lgi.State.attr.u = (float) lang_getnum(R,2);
	lgi.State.attr.v = (float) lang_getnum(R,3);
	return 0;
}


llibfn int lgilib_pushvertex(lRuntime *R) {
	(void) R;
	lgi_Vertex vertex = lgi.State.attr;
	lgi.State.vertex_array[
	lgi.State.vertex_tally] = vertex;
	lgi.State.vertex_tally += 1;
	return 0;
}


llibfn int lgilib_setsampler(lRuntime *R) {
	lgi_Texture *tex = (lgi_Texture *) lang_getsysobj(R,0);
	lString *sampler = lang_getstr(R,1);
	if (S_eq(sampler->c,"linear"))  {
		tex->d3d11.sampler = lgi.LINEAR_SAMPLER;
	} else
	if (S_eq(sampler->c,"point"))  {
		tex->d3d11.sampler = lgi.POINT_SAMPLER;
	}
	return 0;
}


llibfn int lgilib_tick(lRuntime *rt) {
	int r = lgi_tick();
	lang_pushlong(rt,r);
	return 1;
}


llibfn int lgilib_drawQuad(lRuntime *rt) {
	LASSERT(lang_toplen(rt) == 8);
	float r = (float) lang_getnum(rt,0);
	float g = (float) lang_getnum(rt,1);
	float b = (float) lang_getnum(rt,2);
	float a = (float) lang_getnum(rt,3);

	float x = (float) lang_getnum(rt,4);
	float y = (float) lang_getnum(rt,5);
	float w = (float) lang_getnum(rt,6);
	float h = (float) lang_getnum(rt,7);
	lgi_drawQuad((lgi_Color){r,g,b,a},x,y,w,h);
	return 0;
}


llibfn int lgilib_drawQuadUV(lRuntime *R) {
	LASSERT(lang_toplen(R) == 9);
	float r = (float) lang_getnum(R,0);
	float g = (float) lang_getnum(R,1);
	float b = (float) lang_getnum(R,2);
	float a = (float) lang_getnum(R,3);
	float x = (float) lang_getnum(R,4);
	float y = (float) lang_getnum(R,5);
	float w = (float) lang_getnum(R,6);
	float h = (float) lang_getnum(R,7);
	lgi_Texture *tex = lang_getsysobj(R,8);
	lgi_drawQuadUV((lgi_Color){r,g,b,a},tex,x,y,w,h);
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


llibfn int lgilib_getcursorx(lRuntime *rt) {
	lang_pushlong(rt,lgi.Input.Mice.xcursor);
	return 1;
}


llibfn int lgilib_getcursorx(lRuntime *rt) {
	lang_pushlong(rt,lgi.Input.Mice.ycursor);
	return 1;
}


llibfn int lgilib_getSizeX(lRuntime *rt) {
	lang_pushlong(rt,lgi.Window.size_x);
	return 1;
}


llibfn int lgilib_getSizeY(lRuntime *rt) {
	lang_pushlong(rt,lgi.Window.size_y);
	return 1;
}



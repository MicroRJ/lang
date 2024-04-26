/*
** See Copyright Notice In lang.h
** lgilib.c
** LGI Library Bindings For Lang.
*/

#include <lang.h>
#include <lgi/lgi.h>



llibfn int lgilib_wasButtonDown(lRuntime *R) {
	elf_putint(R,lgi_wasButtonDown((int) elf_getint(R,0)));
	return 1;
}


llibfn int lgilib_isButtonDown(lRuntime *R) {
	elf_putint(R,lgi_isButtonDown((int) elf_getint(R,0)));
	return 1;
}


llibfn int lgilib_isButtonReleased(lRuntime *R) {
	elf_putint(R,lgi_isButtonReleased((int) elf_getint(R,0)));
	return 1;
}


llibfn int lgilib_isButtonPressed(lRuntime *R) {
	elf_putint(R,lgi_isButtonPressed((int) elf_getint(R,0)));
	return 1;
}


llibfn int lgilib_time(lRuntime *rt) {
	elf_num time = lgi.Time.totalSeconds;
	elf_putnum(rt,time);
	return 1;
}


llibfn int lgilib_deltatime(lRuntime *rt) {
	elf_num time = lgi.Time.deltaSeconds;
	elf_putnum(rt,time);
	return 1;
}


llibfn int lgilib_initWindowed(lRuntime *rt) {
	int windowWidth = (int) elf_getint(rt,0);
	int windowHeight = (int) elf_getint(rt,1);
	elf_str *name = elf_getstr(rt,2);
	lgi_initWindowed(windowWidth,windowHeight,name->c);
	return 0;
}


llibfn int lgilib_binddeftex(lRuntime *R) {
	(void) R;
	lgi_bindTexture(0,lgi.whiteTexture,lgi_True);
	return 0;
}


llibfn int lgilib_loadtexture(lRuntime *R) {
	elf_str *filename = elf_getstr(R,0);
	lgi_Texture *texture = lgi_loadTexture(filename->c);
	elf_putsys(R,(elf_Handle)texture);
	return 1;
}


llibfn int lgilib_gettexturewidth(lRuntime *R) {
	lgi_Texture *texture = (lgi_Texture *) elf_getsys(R,0);
	elf_putint(R,texture->size_x);
	return 1;
}


llibfn int lgilib_gettextureheight(lRuntime *R) {
	lgi_Texture *texture = (lgi_Texture *) elf_getsys(R,0);
	elf_putint(R,texture->size_y);
	return 1;
}


llibfn int lgilib_bindtexture(lRuntime *R) {
	// lgi_bindProgram(lgi.defaultProgram);
	lgi_Texture *tex = (lgi_Texture *) elf_getsys(R,0);
	lgi_bindTexture(0,tex,lgi_True);
	return 0;
}


llibfn int lgilib_beginvertices(lRuntime *R) {
	int ni = (int) elf_getint(R,0);
	int nv = (int) elf_getint(R,1);
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
		lgi_Index index = (lgi_Index) elf_ntoi(R->call->locals[i]);
		lgi.State.index_array[
		lgi.State.index_tally] = lgi.State.index_offset + index;
		lgi.State.index_tally += 1;
	}
	return 0;
}


llibfn int lgilib_attrrgba(lRuntime *R) {
	lgi.State.attr.r = (float) elf_getnum(R,0);
	lgi.State.attr.g = (float) elf_getnum(R,1);
	lgi.State.attr.b = (float) elf_getnum(R,2);
	lgi.State.attr.a = (float) elf_getnum(R,3);
	return 0;
}


llibfn int lgilib_attrxyzw(lRuntime *R) {
	lgi.State.attr.x = (float) elf_getnum(R,0);
	lgi.State.attr.y = (float) elf_getnum(R,1);
	lgi.State.attr.z = (float) elf_getnum(R,2);
	lgi.State.attr.w = (float) elf_getnum(R,3);
	return 0;
}


llibfn int lgilib_attruv(lRuntime *R) {
	lgi.State.attr.u = (float) elf_getnum(R,0);
	lgi.State.attr.v = (float) elf_getnum(R,1);
	return 0;
}


llibfn int lgilib_attrxyuv(lRuntime *R) {
	lgi.State.attr.x = (float) elf_getnum(R,0);
	lgi.State.attr.y = (float) elf_getnum(R,1);
	lgi.State.attr.u = (float) elf_getnum(R,2);
	lgi.State.attr.v = (float) elf_getnum(R,3);
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
	lgi_Texture *tex = (lgi_Texture *) elf_getsys(R,0);
	elf_str *sampler = elf_getstr(R,1);
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
	elf_putint(rt,r);
	return 1;
}


llibfn int lgilib_drawline(lRuntime *R) {
	float r = (float) elf_getnum(R,0);
	float g = (float) elf_getnum(R,1);
	float b = (float) elf_getnum(R,2);
	float a = (float) elf_getnum(R,3);
	float thickness = (float) elf_getnum(R,4);
	float x0 = (float) elf_getnum(R,5);
	float y0 = (float) elf_getnum(R,6);
	float x1 = (float) elf_getnum(R,7);
	float y1 = (float) elf_getnum(R,8);

	lgi_drawLine((lgi_Color){r,g,b,a},thickness,x0,y0,x1,y1);
	return 0;
}


llibfn int lgilib_drawQuad(lRuntime *rt) {
	elf_assert(elf_stklen(rt) == 8);
	float r = (float) elf_getnum(rt,0);
	float g = (float) elf_getnum(rt,1);
	float b = (float) elf_getnum(rt,2);
	float a = (float) elf_getnum(rt,3);

	float x = (float) elf_getnum(rt,4);
	float y = (float) elf_getnum(rt,5);
	float w = (float) elf_getnum(rt,6);
	float h = (float) elf_getnum(rt,7);
	lgi_drawQuad((lgi_Color){r,g,b,a},x,y,w,h);
	return 0;
}


llibfn int lgilib_drawQuadUV(lRuntime *R) {
	elf_assert(elf_stklen(R) == 9);
	float r = (float) elf_getnum(R,0);
	float g = (float) elf_getnum(R,1);
	float b = (float) elf_getnum(R,2);
	float a = (float) elf_getnum(R,3);
	float x = (float) elf_getnum(R,4);
	float y = (float) elf_getnum(R,5);
	float w = (float) elf_getnum(R,6);
	float h = (float) elf_getnum(R,7);
	lgi_Texture *tex = elf_getsys(R,8);
	lgi_drawQuadUV((lgi_Color){r,g,b,a},tex,x,y,w,h);
	return 0;
}


llibfn int lgilib_clearBackground(lRuntime *rt) {
	(void) rt;
	lgi_clearBackground(lgi_BLACK);
	return 0;
}


llibfn int lgilib_testKey(lRuntime *R) {
	int key = (int) elf_getint(R,0);
	elf_putint(R,lgi_testKey(key));
	return 1;
}

llibfn int lgilib_iskeydown(lRuntime *R) {
	int key = (int) elf_getint(R,0);
	elf_putint(R,GetKeyState(key)>>15);
	return 1;
}

llibfn int lgilib_isshiftdown(lRuntime *R) {
	elf_putint(R,lgi.Input.Keyboard.is_shft);
	return 1;
}

llibfn int lgilib_getcursorx(lRuntime *rt) {
	elf_putint(rt,lgi.Input.Mice.xcursor);
	return 1;
}


llibfn int lgilib_getcursory(lRuntime *rt) {
	elf_putint(rt,lgi.Input.Mice.ycursor);
	return 1;
}


llibfn int lgilib_getSizeX(lRuntime *rt) {
	elf_putint(rt,lgi.Window.size_x);
	return 1;
}


llibfn int lgilib_getSizeY(lRuntime *rt) {
	elf_putint(rt,lgi.Window.size_y);
	return 1;
}



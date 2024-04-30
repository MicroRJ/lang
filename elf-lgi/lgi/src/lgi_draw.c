/*
**
**                      -+- rx -+-
**
**                 Mundane Graphics API.
**
**  *--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
**  Copyright(C) Dayan Rodriguez, 2022, All Rights Reserved
**  *--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
**
**        NOT PRODUCTION READY/NOT PRODUCTION QUALITY
**
**
**               HONI SOIT QUI MAL Y PENSE
**
**
**                 github.com/MicroRJ/rx
**
*/

lgi_API void lgi_beginMode3D() {
	lgi_bindProgram(lgi.defaultProgram);
	lgi.State.world_matrix = lgi_Matrix__identity();
	lgi.State. view_matrix = lgi_Matrix__identity();
	lgi.State. view_matrix = lgi_Matrix__projection((double)(lgi.Window.size_y)/(double)(lgi.Window.size_x),90,0.01,1000);
}

lgi_API void lgi_beginMode2D() {
	lgi_bindProgram(lgi.defaultProgram);

	lgi.State.world_matrix = lgi_Matrix__identity();
	lgi.State. view_matrix = lgi_Matrix__identity();
	lgi.State.view_matrix.m[0][0]=  2. / lgi.Window.size_x;
	lgi.State.view_matrix.m[1][1]=  2. / lgi.Window.size_y;
	lgi.State.view_matrix.m[2][2]= .5;
	lgi.State.view_matrix.m[3][0]=- 1.;
	lgi.State.view_matrix.m[3][1]=- 1.;
}

lgi_API void lgi_bindProgram(lgi_Program program) {
	lgi_bindShaders(program.vertexShader,program.pixelShader,lgi_True);
	if (lgi.State.d3d11.blendState != program.d3d11.blendState) {
		lgi.State.d3d11.blendState = program.d3d11.blendState;
		ID3D11DeviceContext_OMSetBlendState(lgi.d3d11.ctx,program.d3d11.blendState,0x00,0xFFFFFFFu);
	}
}

lgi_API void lgi_bindShaders(lgi_Shader *vertexShader, lgi_Shader *pixelShader, int flush) {
	lgi_Shader *liveVertexShader = lgi.State.liveVertexShader;
	lgi_Shader *livePixelShader = lgi.State.livePixelShader;
	if ((liveVertexShader != vertexShader) || (livePixelShader != pixelShader)) {
		lgi_flushImmediatly();
	}
	lgi.State.liveVertexShader = vertexShader;
	lgi.State.livePixelShader = pixelShader;

	if (liveVertexShader != vertexShader) {
		ID3D11DeviceContext_VSSetShader(lgi.d3d11.ctx,vertexShader->d3d11.vertexShader,0x00,0);
		ID3D11DeviceContext_VSSetConstantBuffers(lgi.d3d11.ctx,0,1,&vertexShader->inputBlock->d3d11.buffer);
		ID3D11DeviceContext_IASetInputLayout(lgi.d3d11.ctx,vertexShader->d3d11.layout);
	}

	if (livePixelShader != pixelShader) {
		ID3D11DeviceContext_PSSetShader(lgi.d3d11.ctx,pixelShader->d3d11.pixelShader,0x00,0);
		ID3D11DeviceContext_PSSetConstantBuffers(lgi.d3d11.ctx,0,1,&pixelShader->inputBlock->d3d11.buffer);
	}
}

//
// TODO: Allow Binding Textures To Vertex Shaders?
//

lgi_API void lgi_bindTexture(int slot, lgi_Texture *texture, int flush) {
	if (lgi.State.liveTextures[slot] != texture) {
		lgi_flushImmediatly();

		lgi.State.liveTextures[slot] = texture;
		// TODO:
		if (slot == 0) {
			lgi.State.constSlots.xysource.x = texture->size_x;
			lgi.State.constSlots.xysource.y = texture->size_y;
		}

		// NOTE: Here we assume the sampler is not different, which is less
		// likely given that most textures will use common samplers!
		if (lgi.State.d3d11.samplerStates[slot] != texture->d3d11.sampler) {
			lgi.State.d3d11.samplerStates[slot] = texture->d3d11.sampler;
			ID3D11DeviceContext_PSSetSamplers(lgi.d3d11.ctx,slot,1,&texture->d3d11.sampler);
		}

		ID3D11DeviceContext_PSSetShaderResources(lgi.d3d11.ctx,slot,1,&texture->d3d11.shader_target);
	}
}

void lgi_flushImmediatly() {

	if (lgi.State.index_tally != 0) {
		/* return resources */
		lgi_returnBufferContents(lgi.State.vertexBuffer);
		lgi_returnBufferContents(lgi.State.indexBuffer);
		lgi.State.vertex_array=lgi_Null;
		lgi.State.index_array=lgi_Null;

		lgi_Matrix matrix = lgi_Matrix__multiply(lgi.State.world_matrix,lgi.State.view_matrix);
		lgi.State.constSlots.matrix = matrix;
		lgi.State.constSlots.xyscreen.x = (float)(lgi.Window.size_x);
		lgi.State.constSlots.xyscreen.y = (float)(lgi.Window.size_y);
		lgi.State.constSlots.xycursor.x = (float)(lgi.Input.Mice.xcursor) / lgi.Window.size_x;
		lgi.State.constSlots.xycursor.y = (float)(lgi.Input.Mice.ycursor) / lgi.Window.size_y;
		lgi.State.constSlots.total_seconds = lgi.Time.total_seconds;
		lgi.State.constSlots.delta_seconds = lgi.Time.delta_seconds;
		lgi_updateBufferContents(lgi.defaultConstBlock,&lgi.State.constSlots,sizeof(lgi.State.constSlots));

		ID3D11DeviceContext_DrawIndexed(lgi.d3d11.ctx,lgi.State.index_tally,0,0);
	}

	lgi.State.index_tally  = 0;
	lgi.State.vertex_tally = 0;
}


/* YOU MUST CALL THIS FUNCTION, CAN'T YOU SEE
IT IS FUNDAMENTAL */
lgi_API inline void
lgi_endVertexArray()
{

}

lgi_API void lgi_beginVertexArray(int index_count, int vertex_count) {
	lgi_Bool shouldFlush = lgi_False;
	if (lgi.State.index_tally + index_count > lgi_DEFAULT_INDEX_BUFFER_LENGTH) {
		shouldFlush = lgi_True;
	}

	if (lgi.State.vertex_tally + vertex_count > lgi_DEFAULT_VERTEX_BUFFER_LENGTH) {
		shouldFlush = lgi_True;
	}
	if (shouldFlush) {
		lgi_flushImmediatly();
	}

	if (lgi.State.vertex_array == NULL) {
		lgi.State.vertex_array = lgi_borrowBufferContents(lgi.State.vertexBuffer,lgi_Null);
		lgi.State.index_array = lgi_borrowBufferContents(lgi.State.indexBuffer,lgi_Null);
		lgi.State.vertex_tally = 0;
		lgi.State.index_tally = 0;

		lgi.State.index_offset = 0;
	}

	lgi_ASSERT(lgi.State.vertex_array != 0);
	lgi_ASSERT(lgi.State. index_array != 0);

	/* #pending is this something that we want to do? */
	lgi.State.attr.xyzw.x = .0;
	lgi.State.attr.xyzw.y = .0;
	lgi.State.attr.xyzw.z = .5;
	lgi.State.attr.xyzw.w =  1;
	lgi.State.attr.rgba.r = .0;
	lgi.State.attr.rgba.g = .0;
	lgi.State.attr.rgba.b = .0;
	lgi.State.attr.rgba.a = .0;
	lgi.State.attr.  uv.x = .0;
	lgi.State.attr.  uv.y = .0;

	lgi.State.index_offset = lgi.State.vertex_tally;
}


lgi_API inline lgi_Vertex
rxvtx_xy(float x, float y)
{
	lgi.State.attr.xyzw.x =  x;
	lgi.State.attr.xyzw.y =  y;
	return lgi.State.attr;
}

lgi_API inline lgi_Vertex
rxvtx_xyuv(float x, float y, float u, float v)
{
	lgi.State.attr.xyzw.x =  x;
	lgi.State.attr.xyzw.y =  y;
	lgi.State.attr.xyzw.z = .5;
	lgi.State.attr.xyzw.w =  1;
	lgi.State.attr.  uv.x =  u;
	lgi.State.attr.  uv.y =  v;
	return lgi.State.attr;
}

lgi_API inline lgi_Vertex
rxvtx_xyuv_col(float x, float y, float u, float v, lgi_Color rgba)
{
	lgi.State.attr.xyzw.x =  x;
	lgi.State.attr.xyzw.y =  y;
	lgi.State.attr.xyzw.z = .5;
	lgi.State.attr.xyzw.w =  1;
	lgi.State.attr.  uv.x =  u;
	lgi.State.attr.  uv.y =  v;
	lgi.State.attr.rgba   = rgba;
	return lgi.State.attr;
}


lgi_API void lgi_addIndicesV(int length, ...) {
	lgi_ASSERT(lgi.State.index_tally + length < lgi_DEFAULT_INDEX_BUFFER_LENGTH);

	va_list vli;
	va_start(vli,length);

	for(int i=0; i<length; i+=1) {
		lgi.State.index_array[
		lgi.State.index_tally] = lgi.State.index_offset + va_arg(vli,lgi_Index);
		lgi.State.index_tally += 1;
	}

	va_end(vli);
}

lgi_API void lgi_addVerticesV(int length, ...) {
	lgi_ASSERT(lgi.State.vertex_tally + length < lgi_DEFAULT_VERTEX_BUFFER_LENGTH);

	va_list vli;
	va_start(vli,length);

	for (int i=0; i<length; i+=1) {
		lgi.State.vertex_array[
		lgi.State.vertex_tally] = va_arg(vli,lgi_Vertex);
		lgi.State.vertex_tally += 1;
	}

	va_end(vli);
}


lgi_API void lgi_drawCircleSDF(vec2 center, vec2 radius, lgi_Color color, float roundness, float softness) {

	int x0,y0,x1,y1;
	x0 = (int) (center.x - (1. + radius.x + softness));
	y0 = (int) (center.y - (1. + radius.y + softness));
	x1 = (int) (center.x + (1. + radius.x + softness));
	y1 = (int) (center.y + (1. + radius.y + softness));

	lgi_bindProgram(lgi.sdfCircleProgram);
	lgi_beginVertexArray(6,4);
	{
		/* Set Shared Attributes */
		lgi.State.attr.rect.xyxy   = Vec4_xyzw(center.x,center.y,radius.x,radius.y);
		lgi.State.attr.rect.rgba   = color;
		lgi.State.attr.rect.flag.x = roundness;
		lgi.State.attr.rect.flag.w = softness;

		lgi_addIndicesV(6, 0,1,2, 0,2,3);
		lgi_addVerticesV(4
		,	rxvtx_xy(x0,y0),	rxvtx_xy(x0,y1)
		,	rxvtx_xy(x1,y1),	rxvtx_xy(x1,y0));

	}
	lgi_endVertexArray();
}

lgi_API void lgi_drawBoxSDF(vec2 center, vec2 radius, lgi_Color color, float roundness, float softness);

lgi_API void lgi_drawBox(vec2 xy, vec2 sz, lgi_Color color, float roundness, float softness) {
	vec2 radius = {sz.x * .5, sz.y * .5};
	vec2 center = { xy.x + radius.x, xy.y + radius.y };
	lgi_drawBoxSDF(center,radius,color,roundness,softness);
}

lgi_API void lgi_drawBoxSDF(vec2 center, vec2 radius, lgi_Color color, float roundness, float softness) {

	int x0,y0,x1,y1;
	x0 = (int) (center.x - (1. + radius.x + softness));
	y0 = (int) (center.y - (1. + radius.y + softness));
	x1 = (int) (center.x + (1. + radius.x + softness));
	y1 = (int) (center.y + (1. + radius.y + softness));

	lgi_bindProgram(lgi.sdfBoxProgram);

	lgi_beginVertexArray(6,4);
	{
		/* Set Shared Attributes */
		lgi.State.attr.rect.xyxy   = Vec4_xyzw(center.x,center.y,radius.x,radius.y);
		lgi.State.attr.rect.rgba   = color;
		lgi.State.attr.rect.flag.x = roundness;
		lgi.State.attr.rect.flag.w = softness;

		lgi_addIndicesV(6, 0,1,2, 0,2,3);
		lgi_addVerticesV(4
		,	rxvtx_xy(x0,y0),	rxvtx_xy(x0,y1)
		,	rxvtx_xy(x1,y1),	rxvtx_xy(x1,y0));
	}
	lgi_endVertexArray();
}

lgi_API void lgi_drawQuad(lgi_Color color, float x, float y, float w, float h) {

	vec2 xy0 = (vec2){x+0,y+0};
	vec2 xy1 = (vec2){x+w,y+h};
	vec2 uv0 = (vec2){0,0};
	vec2 uv1 = (vec2){1,1};
	lgi_bindProgram(lgi.defaultProgram);
	lgi_bindTexture(0,lgi.whiteTexture,lgi_True);
	lgi_beginVertexArray(6,4);
	{
		/* Set Shared Attributes */
		lgi.State.attr.rgba = color;
		lgi_addIndicesV(6, 0,1,2, 0,2,3);

		lgi_addVerticesV(4
		,	rxvtx_xyuv(xy0.x,xy0.y, uv0.x,uv1.y)
		,	rxvtx_xyuv(xy0.x,xy1.y, uv0.x,uv0.y)
		,	rxvtx_xyuv(xy1.x,xy1.y, uv1.x,uv0.y)
		,	rxvtx_xyuv(xy1.x,xy0.y, uv1.x,uv1.y));
	}
	lgi_endVertexArray();
}

lgi_API void lgi_drawQuadUV(lgi_Color color, lgi_Texture *texture, float x, float y, float w, float h) {
	vec2 xy0 = (vec2){x+0,y+0};
	vec2 xy1 = (vec2){x+w,y+h};

	vec2 uv0 = (vec2){0,0};
	vec2 uv1 = (vec2){1,1};

	lgi_bindProgram(lgi.defaultProgram);
	lgi_bindTexture(0,texture,TRUE);
	lgi_beginVertexArray(6,4);
	{
		/* Set Shared Attributes */
		lgi.State.attr.rgba = color;

		lgi_addIndicesV(6, 0,1,2, 0,2,3);
		lgi_addVerticesV(4
		, rxvtx_xyuv(xy0.x,xy0.y, uv0.x,uv1.y)
		, rxvtx_xyuv(xy0.x,xy1.y, uv0.x,uv0.y)
		, rxvtx_xyuv(xy1.x,xy1.y, uv1.x,uv0.y)
		, rxvtx_xyuv(xy1.x,xy0.y, uv1.x,uv1.y));
	}
	lgi_endVertexArray();
}


lgi_API void lgi_drawLine(lgi_Color color, float thickness, float x0, float y0, float x1, float y1) {
	float xdist=x1-x0;
	float ydist=y1-y0;
	float length=sqrtf(xdist*xdist + ydist*ydist);
	float xnormal=.5f * thickness * -ydist/length;
	float ynormal=.5f * thickness * +xdist/length;

	lgi_bindProgram(lgi.defaultProgram);
	lgi_bindTexture(0,lgi.whiteTexture,lgi_True);
	lgi_beginVertexArray(6,4);
	{
		/* Set Shared Attributes */
		lgi.State.attr.rgba = color;
		lgi_addIndicesV(6, 0,1,2, 0,2,3);
		lgi_addVerticesV(4
		,	rxvtx_xyuv(x0-xnormal,y0-ynormal,0,1)
		,	rxvtx_xyuv(x0+xnormal,y0+ynormal,0,0)
		,	rxvtx_xyuv(x1+xnormal,y1+ynormal,1,0)
		,	rxvtx_xyuv(x1-xnormal,y1-ynormal,1,1));
	}
	lgi_endVertexArray();
}


lgi_API void lgi_drawOutline(float x, float y, float zx, float zy, float thickness, lgi_Color color) {
	float x0 = x - thickness;
	float y0 = y - thickness;
	float x1 = x + zx;
	float y1 = y + zy;

	lgi_drawQuad(color,x0,y1,zx+thickness*2,thickness);
	lgi_drawQuad(color,x0,y0,zx+thickness*2,thickness);

	lgi_drawQuad(color,x0,y,thickness,zy);
	lgi_drawQuad(color,x1,y,thickness,zy);
}

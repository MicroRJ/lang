/*
**
** Copyright(c) 2023 - Dayan Rodriguez - Dyr
**
** -+- elang -+-
**
*/

// NOTE: maybe have something like this so that I don't have to specify
// lib_lgi all the time...
// module :: "lgi"
// addon library :: "lib_lgi"

__win32_Sleep :: addon "lib_lgi" (millis: int) -> int

// TODO: enums!
// lgi_Format :: enum {
// 	R8_UNORM,
// 	R8G8B8A8_UNORM,
// }

lgi_Format :: int

lgi_Texture :: struct {
	size_x := 0
	size_y := 0
	format: lgi_Format
}

lgi_Vertex :: struct {
	xyzw: float4
	rgba: float4
	uv:   float4
}

lgi_initWindowed :: addon "lib_lgi" (w: int, h: int, name := "lgi::window") -> int
lgi_tick :: addon "lib_lgi" () -> int
lgi_pollInput :: addon "lib_lgi" () -> int
lgi_time :: addon "lib_lgi" () -> num

lgi_getCursorX :: addon "lib_lgi"  () -> int
lgi_getCursorY :: addon "lib_lgi"  () -> int
lgi_getSizeX :: addon "lib_lgi"  () -> int
lgi_getSizeY :: addon "lib_lgi"  () -> int
lgi_testKey :: addon "lib_lgi" (key: int) -> int
lgi_testCtrlKey :: addon "lib_lgi" () -> int
lgi_testButton :: addon "lib_lgi" (x: int) -> int

// TODO: enums!
lgi_loadTexture :: addon "lib_lgi" (file: str) -> lgi_Texture *
lgi_bindTexture :: addon "lib_lgi" (texture: lgi_Texture *, slot: int = 0) -> int

lgi_clearBackground :: addon "lib_lgi" (r: float, g: float, b: float, a: float) -> int
lgi_clearBackgroundColor :: (c: float4) -> int {
	ret lgi_clearBackground(c.x,c.y,c.z,c.w)
}
lgi_beginMode2D :: addon "lib_lgi" () -> int

lgi_drawTextLine :: addon "lib_lgi" (x: float, y: float, text: str) -> int
lgi_drawRoundBox :: addon "lib_lgi" (x: float, y: float, w: float, h: float, radius: float, r: float, g: float, b: float, a: float) -> int
lgi_drawCircleSDF :: addon "lib_lgi" (x: float, y: float, radius: float, r: float, g: float, b: float, a: float, roundness: float, edge_softness: float) -> int

// TODO: global initializers and constants
lgi_Globals :: struct {
	COLOR_WHITE   ::= new float4 { 1.,    1., 1., 1. }
	COLOR_RED     ::= new float4 { 1.,    0., 0., 1. }
	COLOR_GREEN   ::= new float4 { 0.,    1., 0., 1. }
	COLOR_BLUE    ::= new float4 { 0.,    0., 1., 1. }
	COLOR_BLACK   ::= new float4 { 0.,    0., 0., 1. }
	COLOR_GRAY    ::= new float4 { .5,    .5, .5, 1. }
	COLOR_CYAN    ::= new float4 {  0,    1., 1., 1. }
	COLOR_YELLOW  ::= new float4 { 1.,    1.,  0, 1. }
	COLOR_MAGENTA ::= new float4 { 1.,     0, 1., 1. }
	COLOR_ORANGE  ::= new float4 { 1., 0.647,  0, 1. }
}
lgi: lgi_Globals *

lgi_initAll :: (w := 0, h := 0, name := "unnamed") -> int {
	// TODO: this should be allocated in global memory!
	lgi = new lgi_Globals {}
	ret lgi_initWindowed(w,h,name)
}

draw_circle :: (x: num, y: num, radius: num, rgba := lgi.COLOR_WHITE, edge_softness := 1.) -> int {
	ret lgi_drawCircleSDF(x,y,radius,rgba.x,rgba.y,rgba.z,rgba.w,1.,edge_softness)
}

draw_text :: (x: num, y: num, text: str) -> int {
	ret lgi_drawTextLine(x,y,text)
}

draw_box :: (x: num, y: num, w: num, h: num, r: num = 1., rgba: float4) -> int {
	ret lgi_drawRoundBox(x,y,w,h,r,rgba.x,rgba.y,rgba.z,rgba.w)
}

// NOTE: this is part of the lower level API, thought tempting it does not mean that you should
// necessarly prefer to use it since it will be slower!

lgi_beginVertices :: addon "lib_lgi" (numIndices: int, numVertices: int) -> int
lgi_addVertex :: addon "lib_lgi" (x: num, y: num, z := .0, w := .0, u := .0, v := .0, r := .0, g := .0, b := .0, a := .0) -> int
lgi_addIndex :: addon "lib_lgi" (i: int) -> int
lgi_endVertices :: addon "lib_lgi" () -> int

draw_immediate_quad_uv :: (texture: lgi_Texture *, x0: num, y0: num, w: num, h: num) -> int {
	x1 := x0+w
	y1 := y0+h
	// __log(__fmt("x0 = %, y0 = %, x1 = %, y1 = %",x0,y0,x1,y1))

	u0 := 0.
	v0 := 0.
	u1 := 1.
	v1 := 1.

	lgi_bindTexture(texture,0)
	lgi_beginVertices(6,4)
	lgi_addVertex(x0,y0, .5, 1., u0,v1, 1.,1.,1.,1.)
	lgi_addVertex(x0,y1, .5, 1., u0,v0, 1.,1.,1.,1.)
	lgi_addVertex(x1,y1, .5, 1., u1,v0, 1.,1.,1.,1.)
	lgi_addVertex(x1,y0, .5, 1., u1,v1, 1.,1.,1.,1.)
	lgi_addIndex(0)
	lgi_addIndex(1)
	lgi_addIndex(2)
	lgi_addIndex(0)
	lgi_addIndex(2)
	lgi_addIndex(3)
	lgi_endVertices()

	ret 1
}

import "core/core.el"
import "core/lgi.el"


main :: () -> int {
	lgi_initAll(512,512,"texture sample")

	texture := lgi_loadTexture("media/logo.jfif")

	no_error := 1
	for no_error ? {
		draw_box(0,0,200.,200.,2.,lgi.COLOR_WHITE)
		lgi_drawTextLine(0,0,"Welcome to the thing")
		draw_immediate_quad_uv(texture,0,0,256,256)
		no_error = lgi_tick()
	}
	ret 0
}
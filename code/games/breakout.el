import "code/core/core.el"
import "code/core/lgi.el"

Game :: struct {
	ball_speed ::= new float2 { 10.,10. }
	ball_direction ::= local float2{0,1}
	ball_position ::= new float2 { 256. + 64., 12. }
	bricks := new int[8*12]

	paddle_position ::= new float2 {}
	paddle_dimensions ::= new float2 {200.,12.}
}

G: Game *

sampleall :: (ball: float2) -> float {
	result := 100000.
	{
		box_xy ::= G.paddle_position
		box_rr ::= G.paddle_dimensions * local float2{.5,.5}
		box_ct ::= box_xy+box_rr
		dst ::= ball-box_ct
		result = min(result,boxsdf(dst,box_rr)-10.)
		if result < 12. ? {
			ret result
		}
	}

	size_x ::= lgi_getSizeX()
	size_y ::= lgi_getSizeY()
	for i in 0..8 ? {
		for j in 0..12 ? {
			if G.bricks[i * 12 + j] == 0 ? {
				x ::= j * 80 + j * 4
				y ::= size_y - (((i + 1) * 20) + (i + 1) * 4)

				box_xy ::= local float2{ x, y }
				box_rr ::= local float2{ 80. * .5, 20. * .5}

				result = min(result,boxsdf((ball-(box_xy+box_rr)),box_rr) - 10.)

				if result < 12. ? {
					G.bricks[i * 12 + j] = 1
					ret result
				}
			}
		}
	}

	walls_xy ::= local float2{ 0., 0. }
	walls_rr ::= local float2{ 1024. * .5, 1024. * .5}
	result = min(result,-boxsdf(ball-(walls_xy+walls_rr),walls_rr))
	ret result
}

bounceoff :: (ball: float2) -> float2 {
	ddx ::= local float2 { .001, .0 }
	ddy ::= local float2 { .0, .001 }
	result ::= local float2 {
		sampleall(ball + ddx) - sampleall(ball - ddx),
		sampleall(ball + ddy) - sampleall(ball - ddy)
	}
	ret normalize(result)
}

run_game :: () -> int {
	lgi_initAll(1024,1024, "breakout")

	G = new Game {}

	the_game ::= G


	no_error := 1
	game_over := 0
	timer := 0.

	color_bands ::= new float4 *[8]
	color_bands[0] = lgi.COLOR_RED
	color_bands[1] = lgi.COLOR_RED
	color_bands[2] = lgi.COLOR_ORANGE
	color_bands[3] = lgi.COLOR_ORANGE
	color_bands[4] = lgi.COLOR_GREEN
	color_bands[5] = lgi.COLOR_GREEN
	color_bands[6] = lgi.COLOR_YELLOW
	color_bands[7] = lgi.COLOR_YELLOW

	for no_error ? {
		delta := lgi_time()
		timer = timer + delta
		size_x ::= lgi_getSizeX()
		size_y ::= lgi_getSizeY()

		cursor_x ::= iton(lgi_getCursorX())
		cursor_y ::= iton(lgi_getCursorY())

		G.paddle_position.x = cursor_x - G.paddle_dimensions.x * .5

		distance ::= sampleall(G.ball_position)

		if distance < 12. ? {
			// draw_circle(G.ball_position.x,G.ball_position.y,12.,lgi.COLOR_RED)
			penetration ::= 12. - distance
			normal ::= bounceoff(G.ball_position)

			// TODO:
			// this is the leak
			G.ball_position = float2_add(G.ball_position,float2_mul(normal,local float2{penetration,penetration}))
			G.ball_direction = reflect(normal,normalize(G.ball_direction))
		}

		velocity ::= G.ball_direction * G.ball_speed
		G.ball_position = float2_add(G.ball_position,velocity)
		// __log(__fmt("ball: %, %",position.x,position.y))

		/* Begin Rendering
		*/
		lgi_clearBackgroundColor(lgi.COLOR_BLACK)

		// drawing
		for i in 0..8 ? {
			for j in 0..12 ? {
				x ::= j * 80 + j * 4
				y ::= size_y - (((i + 1) * 20) + (i + 1) * 4)
				if G.bricks[i * 12 + j] == 0 ? {
					draw_box(x,y,80,20,5.,color_bands[i])
				}
			}
		}

		draw_box(G.paddle_position.x,0,G.paddle_dimensions.x,G.paddle_dimensions.y,5.,lgi.COLOR_CYAN)
		draw_circle(G.ball_position.x,G.ball_position.y,12.,lgi.COLOR_GREEN)
		no_error = lgi_tick()
	}

	ret 0
}

main :: () -> int {
	testelang()
	run_game()
	ret 0
}


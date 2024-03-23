// NOTE(RJ): this example is old not working anymore!
addon lengthOf :: (arr: num[1]*) -> int
addon __strfmt :: (format: str, number: num) -> str

addon "rxlib" __win32_Sleep :: (millisecondsToSleep: int) -> int

addon "rxlib" lgi_drawTextLine :: (x: num, y: num, text: str) -> int

addon "rxlib" lgi_initWindowed :: (windowName: str) -> int
addon "rxlib" lgi_tick :: () -> int
addon "rxlib" lgi_time :: () -> num

addon "rxlib" lgi_getCursorX ::  () -> int
addon "rxlib" lgi_getCursorY ::  () -> int
addon "rxlib" lgi_getSizeX ::  () -> int
addon "rxlib" lgi_getSizeY ::  () -> int

addon "rxlib" lgi_drawRoundBox :: (x: num, y: num, w: num, h: num, r: num) -> int
addon "rxlib" lgi_testKey :: (key: int) -> int
addon "rxlib" lgi_testCtrlKey :: () -> int
addon "rxlib" lgi_testButton :: (x: int) -> int

rect_t :: struct {
	x0: num
	y0: num
	x1: num
	y1: num
}

vec2 :: struct {
	x: num
	y: num
}

vec2_sub :: (a: vec2 *, b: vec2 *) -> vec2 * {
}

vec2_len :: (vec: vec2 *) -> num {
	ret 0.
}

xorshift32 :: (q: int) -> int {
	q = q ^ q << 13
	q = q ^ q >> 17
	q = q ^ q << 5
	ret q
}

cursor_vec2 :: () -> vec2 * {
	x := lgi_getCursorX()
	y := lgi_getCursorY()
	ret new vec2 { x, y }
}


max :: (x: int, y: int) -> int {

	if y < x ? {
		ret x
	}

	ret y
}

get_window_client_rect :: () -> rect_t * {
	size_x := lgi_getSizeX()
	size_y := lgi_getSizeY()
	ret new rect_t { .0, .0, size_x, size_y }
}

rect_padd :: (rect: rect_t *, xpadd: num, ypadd: num) -> rect_t * {
	xpadd = xpadd * .5
	ypadd = ypadd * .5
	ret new rect_t { rect.x0 + xpadd, rect.y0 + ypadd, rect.x1 - xpadd, rect.y1 - ypadd }
}

cut_top :: (rect: rect_t *, size: num) -> rect_t * {
	result := new rect_t { rect.x0, rect.y1-size, rect.x1, rect.y1 }
	rect.y1 = rect.y1 - size
	ret result
}


rect_in_xy :: (rect: rect_t *, vec: vec2 *) -> int {
	if rect.x0 < vec.x ? {
		if rect.y0 < vec.y ? {
			if vec.x < rect.x1 ? {
				if vec.y <  rect.y1 ? {
					ret 1
				}
			}
		}
	}
	ret 0
}


wrap :: (val: int, min_: int, max_: int) -> int {
	off := max_ - min_
	if val < 0 ? {
		val = 0 - val
	}
	wrp := val % off
	ret min_ + wrp
}

main :: () -> int {
	lgi_initWindowed("MiniClicker")

	true := 1
	false := 0

	show_positions := false

	targets := new rect_t *[10]
	unpopped := 0
	highest_score := 0
	score := 0
	started := 0
	timer := 0.
	travelled := 0.

	old_click_xy := cursor_vec2()

	seed := 5735


	no_error := 1


	for no_error ? {
		size_x := lgi_getSizeX()
		size_y := lgi_getSizeY()

		if lgi_testKey('A') ? {
			show_positions = show_positions != true
		}

		delta := lgi_time()

		if unpopped < 1 ? {
			unpopped = lengthOf(targets)

			j := 0
			for j < unpopped ? {
				seed = xorshift32(seed)
				r := wrap(seed,32,64)
				seed = xorshift32(seed)
				x0 := wrap(seed,256,size_x - 256)
				seed = xorshift32(seed)
				y0 := wrap(seed,256,size_y - 256)
				x1 := x0 + r
				y1 := y0 + r
				targets[j] = new rect_t { x0,y0,x1,y1 }

				j = j + 1
			}
		}

		modifier := 0 - 1

		is_click := lgi_testButton(0)
		click := cursor_vec2()

		if is_click ? {

			if started != true ? {

				started = true
				timer = 0
				score = 0

				old_click_xy = cursor_vec2()
			}
		}

		i := 0
		for i < lengthOf(targets) ? {
			rect := targets[i]
			x := rect.x0
			y := rect.y0
			w := rect.x1-rect.x0
			h := rect.y1-rect.y0
			if 0. < w ? {
				if started ? {
					if is_click ? {

						in_rect := rect_in_xy(rect,click)
						if in_rect ? {
							// POP
							rect.x0 = 0
							rect.y0 = 0
							rect.x1 = 0
							rect.y1 = 0

							travelled = travelled + vec2_len(vec2_sub(click,old_click_xy))

							old_click_xy = click

							unpopped = unpopped + 0 - 1
							modifier = 1
						}
					}
				}

				no_error = lgi_drawRoundBox(x,y,w,h,2)
			}

			if show_positions ? {
				no_error = lgi_drawTextLine(128*0,i*16,__strfmt("x := %",x))
				no_error = lgi_drawTextLine(128*1,i*16,__strfmt("y := %",y))
				no_error = lgi_drawTextLine(128*2,i*16,__strfmt("w := %",w))
				no_error = lgi_drawTextLine(128*3,i*16,__strfmt("h := %",h))
			}

			i = i + 1
		}

		if is_click ? {
			score = score + modifier
		}

		// no_error = lgi_drawTextLine(0,size_y-32,__strfmt("ms := %",time))

		if started ? {
			window_rect := rect_padd(get_window_client_rect(),8.,8.)
			no_error = lgi_drawTextLine(0,cut_top(window_rect,32).y0,__strfmt("timer: %",timer))
			no_error = lgi_drawTextLine(0,cut_top(window_rect,32).y0,__strfmt("highest score: %",highest_score))
			no_error = lgi_drawTextLine(0,cut_top(window_rect,32).y0,__strfmt("score: %",score))
		}

		if started ? {
			if 30. < timer ? {
				highest_score = max(score,highest_score)
				started = false
				unpopped = 0
				no_error = __win32_Sleep(1000 * 3)
			}
			timer = timer + delta
		}

		no_error = lgi_tick()
	}

	ret 0
}

